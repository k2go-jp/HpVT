#define SCS_SOURCECODE_FILEID	"3WKR"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/2/debug.h"
#include "scs/2/logger.h"
#include "scs/2/thread.h"
#include "scs/3/worker.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSWorkerStateValidate(SCSWorkerState value) {

	switch (value) {
		case SCS_WORKERSTATE_STARTED:
		case SCS_WORKERSTATE_STARTING:
		case SCS_WORKERSTATE_STOPPED:
		case SCS_WORKERSTATE_STOPPING:
			return true;
		default:
			return false;
	}

}

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
		scs_errno xxx_errno;\
		char xxx_message[(SCS_EMSGSIZE_MAX + 1)] = { 0 };\
		SCSGetLastError(xxx_errno);\
		SCSGetErrorMessage(xxx_errno, xxx_message, SCS_EMSGSIZE_MAX);\
		SCS_LOG(FATAL, SYSTEM, 00000,  "%s. <<%d>>", xxx_message, xxx_errno);\
		abort();\
	}
#define _SCS_UNLOCK(xxx_self) \
	if (SCSMutexUnlock((xxx_self)->mutex) != 0) {\
		scs_errno xxx_errno;\
		char xxx_message[(SCS_EMSGSIZE_MAX + 1)] = { 0 };\
		SCSGetLastError(xxx_errno);\
		SCSGetErrorMessage(xxx_errno, xxx_message, SCS_EMSGSIZE_MAX);\
		SCS_LOG(FATAL, SYSTEM, 00000,  "%s. <<%d>>", xxx_message, xxx_errno);\
		abort();\
	}

/* ---------------------------------------------------------------------------------------------- */

void SCSWorkerInitialize(SCSWorker * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSWorker));

	SCSMutexInitialize(self->mutex);

	//self->id = 0;
	self->state = SCS_WORKERSTATE_STOPPED;

	//self->cpu = 0;

}
void SCSWorkerFinalize(SCSWorker * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	SCSMutexFinalize(self->mutex);

	//self->id = 0;
	self->state = SCS_WORKERSTATE_STOPPED;

	//self->cpu = 0;

	memset(self, 0, sizeof(SCSWorker));

}

bool SCSWorkerStart(					//
		SCSWorker * __restrict self, 	//
		void * (*func)(void *), 		//
		void * __restrict argument) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (func == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (argument == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);

	if (self->state == SCS_WORKERSTATE_STOPPED) {
		if ((tmp_result = SCSThreadCreate(self, func, argument)) == true) {
			self->state = SCS_WORKERSTATE_STARTING;
		}
		else {
			SCS_LOG(ALERT, SYSTEM, 99999, "");
		}
	}
	else {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%d>>", self->state);
	}

	_SCS_UNLOCK(self);

	return true;
}
void SCSWorkerStop(SCSWorker * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	_SCS_LOCK(self);

	if (self->state == SCS_WORKERSTATE_STARTED) {
		self->state = SCS_WORKERSTATE_STOPPING;
	}
	else {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%d>>", self->state);
	}

	_SCS_UNLOCK(self);

}

void SCSWorkerStarted(SCSWorker * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	_SCS_LOCK(self);

	if (self->state == SCS_WORKERSTATE_STARTING) {
		self->state = SCS_WORKERSTATE_STARTED;
	}
	else {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%d>>", self->state);
	}

	_SCS_UNLOCK(self);

}
void SCSWorkerStopped(SCSWorker * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	_SCS_LOCK(self);

	if (self->state == SCS_WORKERSTATE_STOPPING) {
		self->state = SCS_WORKERSTATE_STOPPED;
	}
	else {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%d>>", self->state);
	}

	_SCS_UNLOCK(self);

}

bool SCSWorkerCheckState(SCSWorker * self, SCSWorkerState state) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (SCSWorkerStateValidate(state) == false) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);

	tmp_result = (self->state == state ? true : false);

	_SCS_UNLOCK(self);

	return tmp_result;
}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
