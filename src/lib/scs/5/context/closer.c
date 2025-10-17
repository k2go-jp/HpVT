#define SCS_SOURCECODE_FILEID	"5CTXCLR"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/2/logger.h"
#include "scs/5/context/context.h"
#include "scs/5/packet/queue.h"
#include "scs/5/packet/packet.h"
#include "scs/5/packet/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_INTERVAL_MS
#error
#endif

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_INTERVAL_MS	1000
#define _SCS_INTERVAL_MS	1000

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSCloserQueue {
	SCSSocketQueue * close;
	SCSSocketQueue * retry;
	SCSSocketQueue * timewait;
} SCSCloserQueue;

/* ---------------------------------------------------------------------------------------------- */

static scs_mutex _scs_global_mutex = SCSMutexInitializer;
static SCSSocketQueue * _scs_global_queue = NULL;

/* ---------------------------------------------------------------------------------------------- */

static void _SCSCloserStandBy(SCSContext * __restrict context, SCSCloserQueue * __restrict queue) {

	queue->close = context->close.queue;
	queue->retry = SCSSocketQueueCreate(SCS_CLOSEWAITQUEUE_SIZE);
	queue->timewait = SCSSocketQueueCreate(SCS_CLOSEWAITQUEUE_SIZE);

	SCSMutexLock(_scs_global_mutex);

	if (SCSSocketQueueHold(queue->close) == true) {
		_scs_global_queue = queue->close;
	}
	else {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
	}

	SCSMutexUnlock(_scs_global_mutex);

}
static bool _SCSCloserIsAvailable(SCSContext * __restrict context,
		SCSCloserQueue * __restrict queue) {
	bool tmp_retval = true;

	if (queue->close == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		tmp_retval = false;
	}

	if (queue->retry == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		tmp_retval = false;
	}

	if (queue->timewait == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		tmp_retval = false;
	}

	return tmp_retval;
}
static void _SCSCloserCleanUp(SCSContext * __restrict context, SCSCloserQueue * __restrict queue) {

	SCSMutexLock(_scs_global_mutex);

	if (_scs_global_queue != NULL) {
		_scs_global_queue = NULL;
	}

	SCSMutexUnlock(_scs_global_mutex);

	if (queue->close != NULL) {
		SCSSocketQueueCleanUp(queue->close);
		//SCSSocketQueueDestroy(queue->close);
		SCSSocketQueueFree(queue->close);
	}

	if (queue->retry != NULL) {
		SCSSocketQueueCleanUp(queue->retry);
		SCSSocketQueueDestroy(queue->retry);
		//SCSSocketQueueFree(queue->retry);
	}

	if (queue->timewait != NULL) {
		SCSSocketQueueCleanUp(queue->timewait);
		SCSSocketQueueDestroy(queue->timewait);
		//SCSSocketQueueFree(queue->timewait);
	}

}

/* ---------------------------------------------------------------------------------------------- */

static bool _SCSSocketDestory(SCSContext * __restrict context, SCSSocket * __restrict socket) {
	SCSSocketId tmp_id;
	int tmp_sd;

	if ((tmp_id = SCSSocketGetId(socket)) == SCS_SKTID_INVVAL) {
		SCS_LOG(ERROR, SOCKET, 99999, "");
		return true;
	}

	if ((tmp_sd = SCSSocketGetSocketDescriptor(socket)) == SCS_SKTDESC_INVVAL) {
		SCS_LOG(ERROR, SOCKET, 99999, "");
		return true;
	}

	if (SCSContextRemoveReceivableSocketDescriptor(context, tmp_sd) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "");
	}

	if (SCSSocketTableRemove(&context->sockets, tmp_id) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "");
	}

	SCSSocketDestroy(socket);

	SCS_LOG(NOTICE, SOCKET, 00000, "Disposed a socket. <<%d>>", tmp_id);

	return true;
}

static inline void ___SCSCloserWork3(SCSContext * __restrict context,
		SCSCloserQueue * __restrict queue) {
	static scs_timespec tmp_next;
	scs_timespec tmp_current;
	size_t tmp_num;
	size_t n;

	SCSTimespecSetCurrentTime(tmp_current, CLOCK_MONOTONIC);

	if (SCSTimespecCompare(tmp_next, tmp_current, <)) {
		SCSTimespecCopy(tmp_next, tmp_current);
		SCSTimespecIncreaseSec(tmp_next, 1);
	}
	else {
		return;
	}

	if ((tmp_num = SCSSocketQueueCount(queue->timewait)) < 1) {
		return;
	}

	for (n = 0; n < tmp_num; n++) {
		SCSSocket * tmp_socket;

		if ((tmp_socket = SCSSocketQueueDequeue(queue->timewait)) == NULL) {
			SCS_LOG(NOTICE, SYSTEM, 99999, "<<%zu/%zu>>", n, tmp_num);
			break;
		}

		if (SCSSocketIsClosed(tmp_socket)) {
			if (_SCSSocketDestory(context, tmp_socket) == false) {
				SCS_LOG(WARN, SOCKET, 99999, "<<%zu/%zu>>", n, tmp_num);
				//TODO
			}
		}
		else
		{
			if (SCSSocketQueueEnqueue(queue->timewait, tmp_socket) == false) {
							SCS_LOG(WARN, SOCKET, 99999, "<<%zu/%zu>>", n, tmp_num);
			}
		}

		SCSSocketFree(tmp_socket);
	}

}
static inline void ___SCSCloserWork2(SCSContext * __restrict context,
		SCSCloserQueue * __restrict queue) {
	static scs_timespec tmp_next;
	scs_timespec tmp_current;
	size_t tmp_num;
	size_t n;

	SCSTimespecSetCurrentTime(tmp_current, CLOCK_MONOTONIC);

	if (SCSTimespecCompare(tmp_next, tmp_current, <)) {
		SCSTimespecCopy(tmp_next, tmp_current);
		SCSTimespecIncreaseSec(tmp_next, 1);
	}
	else {
		return;
	}

	if ((tmp_num = SCSSocketQueueCount(queue->retry)) < 1) {
		return;
	}

	for (n = 0; n < tmp_num; n++) {
		SCSSocket * tmp_socket;

		if ((tmp_socket = SCSSocketQueueDequeue(queue->retry)) == NULL) {
			SCS_LOG(NOTICE, SYSTEM, 99999, "<<%zu/%zu>>", n, tmp_num);
			break;
		}

		if (SCSSocketClose(tmp_socket) == true) {
			if (SCSSocketQueueEnqueue(queue->timewait, tmp_socket) == false) {
				SCS_LOG(WARN, SOCKET, 99999, "<<%zu/%zu>>", n, tmp_num);
			}
		}
		else {
			if (SCSSocketQueueEnqueue(queue->retry, tmp_socket) == false) {
				SCS_LOG(WARN, SOCKET, 99999, "<<%zu/%zu>>", n, tmp_num);
				_SCSSocketDestory(context, tmp_socket);
			}
		}

		SCSSocketFree(tmp_socket);
	}

}
static inline void ___SCSCloserWork1(SCSContext * __restrict context,
		SCSCloserQueue * __restrict queue) {
	size_t tmp_num;
	size_t n;

	if ((tmp_num = SCSSocketQueueCount(queue->close)) < 1) {
		return;
	}

	for (n = 0; n < tmp_num; n++) {
		SCSSocket * tmp_socket;

		if ((tmp_socket = SCSSocketQueueDequeue(queue->close)) == NULL) {
			SCS_LOG(NOTICE, SYSTEM, 99999, "<<%zu/%zu>>", n, tmp_num);
			break;
		}

		if (SCSSocketClose(tmp_socket) == true) {
			if (SCSSocketQueueEnqueue(queue->timewait, tmp_socket) == false) {
				SCS_LOG(WARN, SOCKET, 99999, "<<%zu/%zu>>", n, tmp_num);
			}
		}
		else {
			if (SCSSocketQueueEnqueue(queue->retry, tmp_socket) == false) {
				SCS_LOG(WARN, SOCKET, 99999, "<<%zu/%zu>>", n, tmp_num);
			}
		}

		SCSSocketFree(tmp_socket);
	}

}
static inline void __SCSCloserWork(SCSContext * __restrict context,
		SCSCloserQueue * __restrict queue) {

	if (_SCSCloserIsAvailable(context, queue) == false) {
		return;
	}

	while (SCSWorkerIsStarted(&context->worker.closer)) {

		___SCSCloserWork1(context, queue);
		___SCSCloserWork2(context, queue);
		___SCSCloserWork3(context, queue);

		SCSSocketQueueWaitToEnqueue(queue->close, _SCS_INTERVAL_MS);
	}
}
void * _SCSCloserWork(void * argument) {
	SCSContext * tmp_context;
	SCSCloserQueue tmp_queue;

	SCS_LOG(NOTICE, SYSTEM, 10001, "<<%s>>", __func__);

	if ((tmp_context = (SCSContext *) argument) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		return NULL;
	}

	_SCSCloserStandBy(tmp_context, &tmp_queue);

	SCSWorkerStarted(&tmp_context->worker.closer);

	__SCSCloserWork(tmp_context, &tmp_queue);

	SCSWorkerStopped(&tmp_context->worker.closer);

	_SCSCloserCleanUp(tmp_context, &tmp_queue);

	SCS_LOG(NOTICE, SYSTEM, 10002, "<<%s>>", __func__);

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

bool SCSCloserWakeUp(SCSSocket * __restrict socket, scs_time timeout_ms) {
	bool tmp_retval;
	SCSSocketQueue * tmp_queue;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	if (SCSSocketIsClosing(socket)) {
		return true;
	}

	SCSMutexLock(_scs_global_mutex);

	if ((tmp_queue = _scs_global_queue) != NULL) {
		if (SCSSocketQueueHold(tmp_queue) == false) {
			SCS_LOG(ALERT, SYSTEM, 99999, "");
			tmp_queue = NULL;
		}
	}

	SCSMutexUnlock(_scs_global_mutex);

	if (tmp_queue == NULL) {
		return false;
	}

	if ((tmp_retval = SCSSocketQueueEnqueue(tmp_queue, socket)) == false) {
		SCSSocketQueueWaitToDequeue(tmp_queue, timeout_ms);
		tmp_retval = SCSSocketQueueEnqueue(tmp_queue, socket);
	}

	SCSSocketQueueFree(tmp_queue);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_INTERVAL_MS

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
