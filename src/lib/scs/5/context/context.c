#define SCS_SOURCECODE_FILEID	"5CTXCTX"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef WIN32
#else /* WIN32 */
#include <sys/epoll.h>
#endif /* WIN32 */

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/1/unistd.h"
#include "scs/2/logger.h"
#include "scs/3/counter.h"
#include "scs/3/worker.h"
#include "scs/4/gc.h"
#include "scs/5/context/context.h"
#include "scs/5/context/internal.h"
#include "scs/5/packet/queue.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_LOCK
#error
#endif
#ifdef _SCS_UNLOCK
#error
#endif

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_LOCK(xxx_self) \
		if (SCSMutexLock((xxx_self)->mutex) != 0) {\
			SCS_LOG(ALERT, SYSTEM, 99999, "");\
			abort();\
		}
#define _SCS_UNLOCK(xxx_self) \
		if (SCSMutexUnlock((xxx_self)->mutex) != 0) {\
			SCS_LOG(ALERT, SYSTEM, 99999, "");\
			abort();\
		}

/* ---------------------------------------------------------------------------------------------- */

static SCSObjectCounter _counter = SCSObjectCounterInitializer;

/* ---------------------------------------------------------------------------------------------- */

void SCSContextInitialize(SCSContext * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	memset(self, 0, sizeof(SCSContext));

	SCSMutexInitialize(self->mutex);

	SCSAtomicReferenceInitialize(self->reference);

	SCSSettingsInitialize(&self->settings);
	SCSSocketTableInitialize(&self->sockets);
	SCSNetworkTrafficCounterInitialize(&self->traiffc);
	SCSCallbackListInitialize(&self->callbacks);

	//self->send.queue = NULL;

	self->receive.sd = SCS_SKTDESC_INVVAL;

	SCSWorkerInitialize(&self->worker.caller);
	SCSWorkerInitialize(&self->worker.closer);
	SCSWorkerInitialize(&self->worker.manager);
	SCSWorkerInitialize(&self->worker.receiver);
	SCSWorkerInitialize(&self->worker.sender);

}
void SCSContextFinalize(SCSContext * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	SCSMutexFinalize(self->mutex);

	SCSAtomicReferenceFinalize(self->reference);

	SCSSettingsFinalize(&self->settings);
	SCSSocketTableFinalize(&self->sockets);
	SCSNetworkTrafficCounterFinalize(&self->traiffc);
	SCSCallbackListFinalize(&self->callbacks);

	if (self->send.queue != NULL) {
		SCSSocketQueueDestroy(self->send.queue);
		self->send.queue = NULL;
	}

	if (self->receive.sd != SCS_SKTDESC_INVVAL) {
		close(self->receive.sd);
		self->receive.sd = SCS_SKTDESC_INVVAL;
	}

	if (self->close.queue != NULL) {
		SCSSocketQueueDestroy(self->close.queue);
		self->close.queue = NULL;
	}

	SCSWorkerFinalize(&self->worker.caller);
	SCSWorkerFinalize(&self->worker.closer);
	SCSWorkerFinalize(&self->worker.manager);
	SCSWorkerFinalize(&self->worker.receiver);
	SCSWorkerFinalize(&self->worker.sender);

	memset(self, 0, sizeof(SCSContext));
}

/* ---------------------------------------------------------------------------------------------- */

static bool _SCSContextCanDestroy(SCSContext * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	if (SCSAtomicReferenceCanDestroy(self->reference) == false) {
		return false;
	}

	if (SCSWorkerIsStopped(&self->worker.caller) == false) {
		return false;
	}

	if (SCSWorkerIsStopped(&self->worker.closer) == false) {
		return false;
	}

	if (SCSWorkerIsStopped(&self->worker.manager) == false) {
		return false;
	}

	if (SCSWorkerIsStopped(&self->worker.sender) == false) {
		return false;
	}

	if (SCSWorkerIsStopped(&self->worker.receiver) == false) {
		return false;
	}

	return true;
}

SCSContext * SCSContextCreate(void) {
	SCSContext * tmp_self;

	if ((tmp_self = (SCSContext *) malloc(sizeof(SCSContext))) == NULL) {
		SCS_LOG(ALERT, MEMORY, 00002, "<<%zu>>", sizeof(SCSContext));
		return NULL;
	}

	SCSContextInitialize(tmp_self);

	SCSObjectCounterIncreaseCreate(_counter);

	return tmp_self;
}
static bool _SCSContextDestroy(void * self) {

	if (self == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99998, "<<%zu>>", sizeof(SCSContext));
		return true;
	}

	if (_SCSContextCanDestroy((SCSContext *) self) == false) {
		return false;
	}

	SCSContextFinalize(self);
	free(self);

	SCSObjectCounterIncreaseDestroyed(_counter);

	return true;
}
void SCSContextDestroy(SCSContext * self) {
	SCSGarbage * tmp_garbage;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	bool tmp_destroyed;

	_SCS_LOCK(self);

	tmp_destroyed = SCSAtomicReferenceIsDestroy(self->reference);

	if (tmp_destroyed == false) {
		SCSAtomicReferenceForbid(self->reference);
	}

	_SCS_UNLOCK(self);

	if (tmp_destroyed == true) {
		// Already destroyed.
		return;
	}

	SCSObjectCounterIncreaseDestroy(_counter);

	SCSSocketTableCleanUp(&self->sockets);

	SCSWorkerStop(&self->worker.manager);
	SCSWorkerStop(&self->worker.sender);
	SCSWorkerStop(&self->worker.receiver);
	SCSWorkerStop(&self->worker.caller);

	if ((tmp_garbage = SCSGarbageCreate(self, _SCSContextDestroy)) == NULL) {
		SCS_LOG(ERROR, MEMORY, 00001, "<<%zu>>", sizeof(SCSContext));
		return;
	}

	SCSGarbageCollectorPush(&_scs_global_gc, tmp_garbage);

}

inline bool SCSContextHold(SCSContext * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	return SCSAtomicReferenceIncrease(self->reference);
}
inline void SCSContextFree(SCSContext * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSAtomicReferenceDecrease(self->reference);

}

/* ---------------------------------------------------------------------------------------------- */

static inline bool _SCSContextStandBy(SCSContext * self) {
	bool tmp_retval = true;

	if (SCSCallbackListStandBy(&self->callbacks, self->settings.callback.entries) == false) {
		return false;
	}

	if ((self->send.queue = SCSSocketQueueCreate(self->settings.queue.size.send)) == NULL) {
		return false;
	}

	if ((self->close.queue = SCSSocketQueueCreate(self->settings.queue.size.close)) == NULL) {
		return false;
	}

	if (SCSWorkerStart(&self->worker.caller, _SCSCallerWork, self) == false) {
		tmp_retval = false;
	}

	if (SCSWorkerStart(&self->worker.closer, _SCSCloserWork, self) == false) {
		tmp_retval = false;
	}

	if (SCSWorkerStart(&self->worker.manager, _SCSManagerWork, self) == false) {
		tmp_retval = false;
	}

	if (SCSWorkerStart(&self->worker.receiver, _SCSReceiverWork, self) == false) {
		tmp_retval = false;
	}

	if (SCSWorkerStart(&self->worker.sender, _SCSSenderWork, self) == false) {
		tmp_retval = false;
	}

	return tmp_retval;
}
bool SCSContextStandBy(SCSContext * __restrict self, SCSSettings * __restrict settings) {
	bool tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	_SCS_LOCK(self);

	SCSSettingsCopy(&self->settings, settings);

	if (_SCSContextStandBy(self) == true) {
		tmp_result = true;
	}
	else {
		tmp_result = false;
	}

	_SCS_UNLOCK(self);

	return tmp_result;
}

void SCSContextWaitReady(SCSContext * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	_SCS_LOCK(self);

	while (true) {
		sleep(1);

		if (SCSWorkerIsStarted(&self->worker.caller) == false) {
			continue;
		}

		if (SCSWorkerIsStarted(&self->worker.closer) == false) {
			continue;
		}

		if (SCSWorkerIsStarted(&self->worker.manager) == false) {
			continue;
		}

		if (SCSWorkerIsStarted(&self->worker.receiver) == false) {
			continue;
		}

		if (SCSWorkerIsStarted(&self->worker.sender) == false) {
			continue;
		}

		break;
	}

	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

scs_socket_desc SCSContextGetPollingSocketDescriptor(SCSContext * self) {
	int tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	_SCS_LOCK(self);

	if (self->receive.sd != SCS_SKTDESC_INVVAL) {
		tmp_retval = self->receive.sd;
	}
	else {
		if ((tmp_retval = epoll_create1(0)) != SCS_SKTDESC_INVVAL) {
			self->receive.sd = tmp_retval;
		}
		else {
			scs_errno tmp_errno;
			char tmp_message[(SCS_EMSGSIZE_MAX + 1)];

			SCSGetLastError(tmp_errno);
			SCSGetErrorMessage(tmp_errno, tmp_message, SCS_EMSGSIZE_MAX);
			SCS_LOG(ERROR, SYSTEM, 99999, "%s, <<%d>>", tmp_message, tmp_errno);
		}
	}

	_SCS_UNLOCK(self);

	return tmp_retval;
}

static bool _SCSContextAddReceivableSocketDescriptor(SCSContext * self, int sd) {
	int tmp_sd;
	struct epoll_event tmp_event;

	if ((tmp_sd = self->receive.sd) == SCS_SKTDESC_INVVAL) {
		SCS_LOG(ERROR, SYSTEM, 00000, "No ready.");
		return false;
	}

	memset(&tmp_event, 0, sizeof(tmp_event));
	tmp_event.data.fd = sd;
	tmp_event.events = EPOLLIN;

	if (epoll_ctl(tmp_sd, EPOLL_CTL_ADD, sd, &tmp_event) != 0) {
		if (errno != EEXIST) {
			scs_errno tmp_errno;
			char tmp_message[(SCS_EMSGSIZE_MAX + 1)];

			SCSGetLastError(tmp_errno);
			SCSGetErrorMessage(tmp_errno, tmp_message, SCS_EMSGSIZE_MAX);
			SCS_LOG(ERROR, SYSTEM, 00000, "%s. <<%d>>", tmp_message, tmp_errno);

			return false;
		}
	}

	return true;
}
bool SCSContextAddReceivableSocketDescriptor(SCSContext * self, int sd) {
	bool tmp_retval;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (SCSSocketDescValidate(sd) == false) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);

	tmp_retval = _SCSContextAddReceivableSocketDescriptor(self, sd);

	_SCS_UNLOCK(self);

	return tmp_retval;
}

static bool _SCSContextRemoveReceivableSocketDescriptor(SCSContext * self, int sd) {
	int tmp_sd;
	struct epoll_event tmp_event;

	if ((tmp_sd = self->receive.sd) == SCS_SKTDESC_INVVAL) {
		SCS_LOG(ERROR, SYSTEM, 00000, "No ready.");
		return false;
	}

	memset(&tmp_event, 0, sizeof(tmp_event));
	tmp_event.data.fd = sd;
	tmp_event.events = EPOLLIN;

	if (epoll_ctl(tmp_sd, EPOLL_CTL_DEL, sd, &tmp_event) != 0) {
		if (errno != ENOENT) {
			scs_errno tmp_errno;
			char tmp_message[(SCS_EMSGSIZE_MAX + 1)];

			SCSGetLastError(tmp_errno);
			SCSGetErrorMessage(tmp_errno, tmp_message, SCS_EMSGSIZE_MAX);
			SCS_LOG(ERROR, SYSTEM, 00000, "%s. <<%d>>", tmp_message, tmp_errno);

			return false;
		}
	}

	return true;
}
bool SCSContextRemoveReceivableSocketDescriptor(SCSContext * self, int sd) {
	bool tmp_retval;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (SCSSocketDescValidate(sd) == false) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);

	tmp_retval = _SCSContextRemoveReceivableSocketDescriptor(self, sd);

	_SCS_UNLOCK(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSContextMonitor(void) {

	SCSObjectCounterMonitor("Context");

}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
