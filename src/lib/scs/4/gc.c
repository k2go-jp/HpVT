#define SCS_SOURCECODE_FILEID	"4GCR"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/1/unistd.h"
#include "scs/2/logger.h"
#include "scs/3/counter.h"
#include "scs/3/worker.h"
#include "scs/4/gc.h"
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

static inline void _SCSGarbageUpdate(SCSGarbage * self) {
	struct timespec tmp_timestamp;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);

	self->debug.times++;
	if (self->debug.timestamp.first == 0) {
		self->debug.timestamp.first = tmp_timestamp.tv_sec;
	}
	self->debug.timestamp.last = tmp_timestamp.tv_sec;
}

static inline bool _SCSGarbageCollectorPush(	//
		SCSGarbageCollector * __restrict self, //
		SCSGarbage * __restrict garbage) {
	bool tmp_result;

	garbage->next = NULL;

	if (self->garbages.count < SIZE_MAX) {
		if (self->garbages.count == 0) {
			self->garbages.head = garbage;
			self->garbages.tail = garbage;
		}
		else {
			self->garbages.tail->next = garbage;
			self->garbages.tail = garbage;
		}

		self->garbages.count++;

		tmp_result = true;
	}
	else {
		SCS_LOG(ALERT, SYSTEM, 99999, "<<%zu>>", self->garbages.count);

		tmp_result = false;
	}

	return tmp_result;
}
static inline SCSGarbage * _SCSGarbageCollectorPop(SCSGarbageCollector * self) {
	SCSGarbage * tmp_garbage;

	if (self->garbages.count < 1) {
		return NULL;
	}

	if ((tmp_garbage = self->garbages.head) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		return NULL;
	}

	--self->garbages.count;

	if (self->garbages.count == 0) {
		self->garbages.head = NULL;
		self->garbages.tail = NULL;
	}
	else {
		self->garbages.head = tmp_garbage->next;
	}

	return tmp_garbage;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSGarbageInitialize(SCSGarbage * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSGarbage));

	//self->ptr = NULL;
	//self->func = NULL;

	//self->debug.times = 0;
	//self->debug.timestamp.first = 0;
	//self->debug.timestamp.last = 0;

	//self->next = NULL;

}
void SCSGarbageFinalize(SCSGarbage * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	//self->ptr = NULL;
	//self->func = NULL;

	//self->debug.times = 0;
	//self->debug.timestamp.first = 0;
	//self->debug.timestamp.last = 0;

	//self->next = NULL;

	memset(self, 0, sizeof(SCSGarbage));
}

SCSGarbage * SCSGarbageCreate(void * ptr, SCSDiscardFunction func) {
	SCSGarbage * tmp_garbage;

	if (ptr == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return NULL;
	}

	if (func == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return NULL;
	}

	if ((tmp_garbage = (SCSGarbage *) malloc(sizeof(SCSGarbage))) == NULL) {
		SCS_LOG(ALERT, MEMORY, 00002, "");
	}

	SCSGarbageInitialize(tmp_garbage);
	tmp_garbage->ptr = ptr;
	tmp_garbage->func = func;

	SCSObjectCounterIncreaseCreate(_counter);

	return tmp_garbage;
}
void SCSGarbageDestroy(SCSGarbage * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	SCSObjectCounterIncreaseDestroy(_counter);
	SCSObjectCounterIncreaseDestroyed(_counter);

	SCSGarbageFinalize(self);
	free(self);

}

/* ---------------------------------------------------------------------------------------------- */

static inline void __SCSGarbageCollerctorWork(SCSGarbageCollector * self) {
	scs_time tmp_interval;

	while (SCSWorkerIsStarted(&self->worker)) {
		SCSGarbage * tmp_garbage;
		size_t tmp_count;
		size_t i;

		tmp_interval = 1000;

		_SCS_LOCK(self);
		tmp_count = self->garbages.count;
		_SCS_UNLOCK(self);

		for (i = 0; i < tmp_count; i++) {
			_SCS_LOCK(self);
			tmp_garbage = _SCSGarbageCollectorPop(self);
			_SCS_UNLOCK(self);

			if (tmp_garbage == NULL) {
				break;
			}

			if (tmp_garbage->func(tmp_garbage->ptr) == true) {
				SCSGarbageDestroy(tmp_garbage);
			}
			else {
				_SCS_LOCK(self);
				_SCSGarbageCollectorPush(self, tmp_garbage);
				_SCS_UNLOCK(self);
			}
		}

		MSLEEP(tmp_interval);
	}
}
static void * _SCSGarbageCollerctorWork(void * arg) {
	SCSGarbageCollector * tmp_self;

	SCS_LOG(NOTICE, SYSTEM, 10001, "<<%s>>", __func__);

	if ((tmp_self = (SCSGarbageCollector *) arg) == NULL) {
		SCS_LOG(FATAL, SYSTEM, 99999, "");
		return NULL;
	}

	SCSWorkerStarted(&tmp_self->worker);
	__SCSGarbageCollerctorWork(tmp_self);
	SCSWorkerStopped(&tmp_self->worker);

	SCS_LOG(NOTICE, SYSTEM, 10002, "<<%s>>", __func__);

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSGarbageCollectorInitialize(SCSGarbageCollector * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSGarbageCollector));

	SCSMutexInitialize(self->mutex);

	//self->garbages.head = NULL;
	//self->garbages.tail = NULL;
	//self->garbages.count = 0;
	//self->interval = 0;

	//self->summary.count.entry = 0;
	//self->summary.count.disposed = 0;

	//self->statistics.count.entry = 0;
	//self->statistics.count.disposed = 0;

	SCSWorkerInitialize(&self->worker);
}
void SCSGarbageCollectorFinalize(SCSGarbageCollector * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	SCSMutexFinalize(self->mutex);

	//self->garbages.head = NULL;
	//self->garbages.tail = NULL;
	//self->garbages.count = 0;
	//self->interval = 0;

	//self->summary.count.entry = 0;
	//self->summary.count.disposed = 0;

	//self->statistics.count.entry = 0;
	//self->statistics.count.disposed = 0;

	SCSWorkerFinalize(&self->worker);

	memset(self, 0, sizeof(SCSGarbageCollector));
}

bool SCSGarbageCollectorStart(SCSGarbageCollector * self) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);
	tmp_result = SCSWorkerStart(&self->worker, _SCSGarbageCollerctorWork, self);
	_SCS_UNLOCK(self);

	return tmp_result;
}
void SCSGarbageCollectorStop(SCSGarbageCollector * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	_SCS_LOCK(self);
	SCSWorkerStop(&self->worker);
	_SCS_UNLOCK(self);
}

bool SCSGarbageCollectorPush(	//
		SCSGarbageCollector * __restrict self, //
		SCSGarbage * __restrict garbage) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (garbage != NULL) {
		_SCSGarbageUpdate(garbage);
	}
	else {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);

	tmp_result = _SCSGarbageCollectorPush(self, garbage);

	_SCS_UNLOCK(self);

	return tmp_result;
}
SCSGarbage * SCSGarbageCollectorPop(SCSGarbageCollector * self) {
	SCSGarbage * tmp_garbage;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return NULL;
	}

	_SCS_LOCK(self);
	tmp_garbage = _SCSGarbageCollectorPop(self);
	_SCS_UNLOCK(self);

	return tmp_garbage;
}

size_t SCSGarbageCollectorCount(SCSGarbageCollector * self) {
	size_t tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return 0;
	}

	_SCS_LOCK(self);
	tmp_result = self->garbages.count;
	_SCS_UNLOCK(self);

	return tmp_result;
}

scs_time SCSGarbageCollectorUpdate(SCSGarbageCollector * self) {
	scs_time tmp_interval;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return -1;
	}

	_SCS_LOCK(self);

	//TODO
	tmp_interval = 1000;

	_SCS_UNLOCK(self);

	return tmp_interval;
}

inline void SCSGarbageCollectorMonitor(void) {
	SCSObjectCounterMonitor("GC");
}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
