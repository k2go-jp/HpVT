#define SCS_SOURCECODE_FILEID	"5SKTQUE"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/1/time.h"
#include "scs/1/unistd.h"
#include "scs/2/logger.h"
#include "scs/3/counter.h"
#include "scs/4/gc.h"
#include "scs/5/defines.h"
#include "scs/5/socket/queue.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_LOCK
#error
#endif
#ifdef _SCS_UNLOCK
#error
#endif

#ifdef _SCS_WAIT
#error
#endif
#ifdef _SCS_SIGNAL
#error
#endif

#ifdef _SCS_WAIT_ENQUEUE
#error
#endif
#ifdef _SCS_SIGNAL_ENQUEUE
#error
#endif

#ifdef _SCS_WAIT_ENQUEUE
#error
#endif
#ifdef _SCS_SIGNAL_ENQUEUE
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

#define _SCS_WAIT(xxx_cond, xxx_mutex, xxx_timeout, xxx_retval) \
		if ((xxx_retval = SCSCondTimedwait(xxx_cond, xxx_mutex, xxx_timeout)) != 0) {\
			switch (xxx_retval) { \
				case ETIMEDOUT: \
				case EINTR: \
					break;\
				default: {\
					SCS_LOG(ALERT, SYSTEM, 99999,  "<<%d>>", xxx_retval);\
					abort();\
				}\
			}\
		}
#define _SCS_SIGNAL(xxx_cond) \
		if (SCSCondSignal(xxx_cond) != 0) {\
			SCS_LOG(ALERT, SYSTEM, 99999, "");\
			abort();\
		}

#define _SCS_WAIT_ENQUEUE(xxx_self, xxx_timeout, xxx_retval) \
		_SCS_WAIT((xxx_self)->cond.enqueue, (xxx_self)->mutex, xxx_timeout, xxx_retval)
#define _SCS_SIGNAL_ENQUEUE(xxx_self) \
		_SCS_SIGNAL((xxx_self)->cond.enqueue)

#define _SCS_WAIT_DEQUEUE(xxx_self, xxx_timeout, xxx_retval) \
		_SCS_WAIT((xxx_self)->cond.dequeue, (xxx_self)->mutex, xxx_timeout, xxx_retval)
#define _SCS_SIGNAL_DEQUEUE(xxx_self) \
		_SCS_SIGNAL((xxx_self)->cond.dequeue)

/* ---------------------------------------------------------------------------------------------- */

static SCSObjectCounter _counter = SCSObjectCounterInitializer;

/* ---------------------------------------------------------------------------------------------- */

static bool _SCSSocketQueueStandBy(SCSSocketQueue * self, size_t capacity) {
	SCSSocket ** tmp_ptr;

	if (capacity < SCS_PKTQUEUE_MINSIZE || SCS_PKTQUEUE_MAXSIZE < capacity) {
		SCS_LOG(ERROR, SYSTEM, 99997, "<<%zu, %zu, %zu>>", //
				SCS_PKTQUEUE_MINSIZE, capacity, SCS_PKTQUEUE_MAXSIZE);
		return false;
	}

	if (self->capacity != 0) {
		return false;
	}

	if ((tmp_ptr = (SCSSocket **) calloc(capacity, sizeof(SCSSocket *))) == NULL) {
		SCS_LOG(ERROR, SYSTEM, 00002, "<<%zu>>", (capacity * sizeof(SCSSocket *)));
		return false;
	}

	self->index.head = 0;
	self->index.tail = 0;
	self->index.last = capacity - 1;
	self->entries = tmp_ptr;
	self->capacity = capacity;
	self->count = 0;

	return true;
}

static inline void _SCSSocketQueueCleanUp(SCSSocketQueue * self) {
	size_t i;

	for (i = 0; i < self->capacity; i++) {
		SCSSocket * tmp_socket;

		if ((tmp_socket = self->entries[i]) != NULL) {
			self->entries[i] = NULL;
		}
		else {
			continue;
		}

		//SCSSocketDestroy(tmp_socket);
		SCSSocketFree(tmp_socket);
	}

}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSSocketQueueInitialize(SCSSocketQueue * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	memset(self, 0, sizeof(SCSSocketQueue));

	SCSAtomicReferenceInitialize(self->reference);

	SCSMutexInitialize(self->mutex);
	SCSCondInitialize(self->cond.enqueue);
	SCSCondInitialize(self->cond.dequeue);

	//self->entries
	//self->capacity
	//self->count

	//self->index.head
	//self->index.tail
	//self->index.last

	SCSTimespecInitialize(self->timestamp.next);

}
inline void SCSSocketQueueFinalize(SCSSocketQueue * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	if (self->entries != NULL) {
		_SCSSocketQueueCleanUp(self);
		free(self->entries);
		self->entries = NULL;
	}

	SCSAtomicReferenceFinalize(self->reference);

	SCSMutexFinalize(self->mutex);
	SCSCondFinalize(self->cond.enqueue);
	SCSCondFinalize(self->cond.dequeue);

	//self->capacity
	//self->count

	//self->index.head
	//self->index.tail
	//self->index.last

	SCSTimespecFinalize(self->timestamp.next);

	memset(self, 0, sizeof(SCSSocketQueue));

}

/* ---------------------------------------------------------------------------------------------- */

static bool SCSSocketQueueCanDestroy(SCSSocketQueue * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	if (SCSAtomicReferenceCanDestroy(self->reference) == false) {
		return false;
	}

	return true;
}

SCSSocketQueue * SCSSocketQueueCreate(size_t capacity) {
	SCSSocketQueue * tmp_self;

	if ((tmp_self = (SCSSocketQueue *) malloc(sizeof(SCSSocketQueue))) == NULL) {
		SCS_LOG(ALERT, MEMORY, 00002, "<<%zu>>", sizeof(SCSSocketQueue));
		return NULL;
	}

	SCSSocketQueueInitialize(tmp_self);

	if (_SCSSocketQueueStandBy(tmp_self, capacity) == false) {
		SCSSocketQueueFinalize(tmp_self);
		free(tmp_self);
		return NULL;
	}

	SCSObjectCounterIncreaseCreate(_counter);

	return tmp_self;
}
static bool _SCSSocketQueueDestroy(void * self) {
	SCSSocketQueue * tmp_self;

	if ((tmp_self = (SCSSocketQueue *) self) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99998, "");
		return true;
	}

	if (SCSSocketQueueCanDestroy(tmp_self) == false) {
		return false;
	}

	SCSObjectCounterIncreaseDestroyed(_counter);

	SCSSocketQueueFinalize(tmp_self);
	free(tmp_self);

	return true;
}
void SCSSocketQueueDestroy(SCSSocketQueue * self) {
	SCSGarbage * tmp_garbage;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

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

	if ((tmp_garbage = SCSGarbageCreate(self, _SCSSocketQueueDestroy)) == NULL) {
		SCS_LOG(ALERT, MEMORY, 00002, "<<%zu>>", sizeof(SCSSocketQueue));
		return;
	}

	SCSGarbageCollectorPush(&_scs_global_gc, tmp_garbage);

}

inline bool SCSSocketQueueHold(SCSSocketQueue * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	return SCSAtomicReferenceIncrease((self)->reference);
}
inline void SCSSocketQueueFree(SCSSocketQueue * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	SCSAtomicReferenceDecrease((self)->reference);

}

/* ---------------------------------------------------------------------------------------------- */

static inline bool _SCSSocketQueueEnqueue(SCSSocketQueue * __restrict self,
		SCSSocket * __restrict socket) {
	size_t tmp_next;

	if (self->capacity < self->count) {
		SCS_LOG(WARN, SYSTEM, 79994, "<<%zu/%zu>>", self->count, self->capacity);
		return false;
	}

	if (self->index.tail == self->index.last) {
		tmp_next = 0;
	}
	else {
		tmp_next = self->index.tail + 1;
	}

	if (self->index.head == tmp_next) {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%zu/%zu,%zu/%zu>>", //
				self->count, self->capacity, self->index.head, self->index.tail);
		return false;
	}

	if (SCSSocketHold(socket) == false) {
		return false;
	}

	self->entries[self->index.tail] = socket;
	self->index.tail = tmp_next;
	self->count++;

	if ((self->capacity >> 1) < self->count) {
		scs_timespec tmp_timestamp;

		SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);

		if (SCSTimespecCompare(self->timestamp.next, tmp_timestamp, <)) {
			SCS_LOG(WARN, SYSTEM, 99999, "<<%zu/%zu>>", self->count, self->capacity);
			SCSTimespecIncreaseSec(tmp_timestamp, 3);
			SCSTimespecCopy(self->timestamp.next, tmp_timestamp);
		}
	}

	_SCS_SIGNAL_ENQUEUE(self);

	return true;
}
inline bool SCSSocketQueueEnqueue(SCSSocketQueue * __restrict self, SCSSocket * __restrict socket) {
	bool tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return 0;
//	}

//	if (packet == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	_SCS_LOCK(self);

	tmp_result = _SCSSocketQueueEnqueue(self, socket);

	_SCS_UNLOCK(self);

	return tmp_result;
}
static inline SCSSocket * _SCSSocketQueueDequeue(SCSSocketQueue * self) {
	SCSSocket * tmp_socket;

	if (self->count < 1) {
		return NULL;
	}

	if ((tmp_socket = self->entries[self->index.head]) != NULL) {
		self->entries[self->index.head] = NULL;
	}
	else {
		return NULL;
	}

	if (self->index.head < self->index.last) {
		self->index.head++;
	}
	else {
		self->index.head = 0;
	}

	self->count--;

	//SCSSocketFree(tmp_socket);

	_SCS_SIGNAL_DEQUEUE(self);

	return tmp_socket;
}
inline SCSSocket * SCSSocketQueueDequeue(SCSSocketQueue * self) {
	SCSSocket * tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return 0;
//	}

	_SCS_LOCK(self);

	tmp_result = _SCSSocketQueueDequeue(self);

	_SCS_UNLOCK(self);

	return tmp_result;
}

inline size_t SCSSocketQueueCount(SCSSocketQueue * self) {
	size_t tmp_reuslt;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return 0;
//	}

	_SCS_LOCK(self);

	tmp_reuslt = self->count;

	_SCS_UNLOCK(self);

	return tmp_reuslt;
}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSSocketQueueWaitToEnqueue(SCSSocketQueue * self, scs_time timeout_ms) {
	int tmp_result;
	scs_timespec tmp_timeout;

	_SCS_LOCK(self);

	if (self->count == 0) {
		SCSTimespecSetCurrentTime(tmp_timeout, CLOCK_REALTIME);
		SCSTimespecIncreaseMsec(tmp_timeout, timeout_ms);
		_SCS_WAIT_ENQUEUE(self, tmp_timeout, tmp_result);
	}

	_SCS_UNLOCK(self);

}
inline void SCSSocketQueueWaitToDequeue(SCSSocketQueue * self, scs_time timeout_ms) {
	int tmp_result;
	scs_timespec tmp_timeout;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return 0;
//	}

	_SCS_LOCK(self);

	if (self->count == self->capacity) {
		SCSTimespecSetCurrentTime(tmp_timeout, CLOCK_REALTIME);
		SCSTimespecIncreaseMsec(tmp_timeout, timeout_ms);
		_SCS_WAIT_DEQUEUE(self, tmp_timeout, tmp_result);
	}

	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSSocketQueueCleanUp(SCSSocketQueue * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return 0;
//	}

	_SCS_LOCK(self);

	_SCSSocketQueueCleanUp(self);

	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

#undef _SCS_WAIT
#undef _SCS_SIGNAL

#undef _SCS_WAIT_ENQUEUE
#undef _SCS_SIGNAL_ENQUEUE

#undef _SCS_WAIT_DEQUEUE
#undef _SCS_SIGNAL_DEQUEUE

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
