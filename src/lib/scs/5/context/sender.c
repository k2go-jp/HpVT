#define SCS_SOURCECODE_FILEID	"5CTXSDR"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/unistd.h"
#include "scs/2/logger.h"
#include "scs/5/context/context.h"
#include "scs/5/packet/queue.h"
#include "scs/5/packet/packet.h"
#include "scs/5/packet/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

static scs_mutex _scs_global_mutex = SCSMutexInitializer;
static SCSSocketQueue * _scs_global_queue = NULL;

/* ---------------------------------------------------------------------------------------------- */

static void _SCSSenderStandBy(SCSContext * __restrict context, SCSSocketQueue ** __restrict queue) {
	SCSSocketQueue * tmp_queue;

	SCSMutexLock(_scs_global_mutex);

	if (SCSSocketQueueHold(context->send.queue) == true) {
		tmp_queue = context->send.queue;
		_scs_global_queue = tmp_queue;
	}
	else {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		tmp_queue = NULL;
	}

	SCSMutexUnlock(_scs_global_mutex);

	*queue = tmp_queue;

}

static void _SCSSenderCleanUp(SCSContext * __restrict context, SCSSocketQueue * __restrict queue) {

	SCSMutexLock(_scs_global_mutex);

	if (_scs_global_queue != NULL) {
		_scs_global_queue = NULL;
	}

	SCSMutexUnlock(_scs_global_mutex);

	SCSSocketQueueFree(queue);

}

/* ---------------------------------------------------------------------------------------------- */

static inline void __SCSSenderWork(SCSContext * context, SCSSocketQueue * queue) {
	SCSNetworkTrafficCounter * tmp_statistics;

	tmp_statistics = &context->traiffc;

	while (SCSWorkerIsStarted(&context->worker.sender)) {
		size_t tmp_num;
		size_t n;
		size_t tmp_fail_cnt;
		
		if ((tmp_num = SCSSocketQueueCount(queue)) < 1) {
			// regular case
		}
		
		tmp_fail_cnt = 0;

		for (n = 0; n < tmp_num; n++) {
			SCSSocket * tmp_socket;

			if ((tmp_socket = SCSSocketQueueDequeue(queue)) == NULL) {
				SCS_LOG(NOTICE, SYSTEM, 99999, "<<%zu/%zu>>", n, tmp_num);
				break;
			}

			if (SCSSocketSend(tmp_socket, tmp_statistics) == false) {
				SCSSocketQueueEnqueue(queue, tmp_socket);
				tmp_fail_cnt++;
			}

			SCSSocketFree(tmp_socket);
		}

		if (tmp_fail_cnt != tmp_num || tmp_num == 0) {
			SCSSocketQueueWaitToEnqueue(queue, 1000);
		}
		else {
			MSLEEP(10);
		}
	}

}
void * _SCSSenderWork(void * argument) {
	SCSContext * tmp_context;
	SCSSocketQueue * tmp_queue;

	SCS_LOG(NOTICE, SYSTEM, 10001, "<<%s>>", __func__);

	if ((tmp_context = (SCSContext *) argument) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		return NULL;
	}

	_SCSSenderStandBy(tmp_context, &tmp_queue);

	SCSWorkerStarted(&tmp_context->worker.sender);

	__SCSSenderWork(tmp_context, tmp_queue);

	SCSWorkerStopped(&tmp_context->worker.sender);

	_SCSSenderCleanUp(tmp_context, tmp_queue);

	SCS_LOG(NOTICE, SYSTEM, 10002, "<<%s>>", __func__);

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

bool SCSSenderWakeUp(SCSSocket * __restrict socket, scs_time timeout_ms) {
	bool tmp_retval;
	SCSSocketQueue * tmp_queue;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

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

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
