#define SCS_SOURCECODE_FILEID	"5CBKLST"

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
#include "scs/4/gc.h"
#include "scs/5/callback/system/action.h"
#include "scs/5/callback/system/list.h"
#include "scs/5/callback/system/types.h"
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

static bool _SCSCallbackListEntryCanCall(SCSCallbackListEntry * self, scs_timespec timestamp) {

	if (self->config.ptr == NULL) {
		SCS_LOG(ERROR, SYSTEM, 99998, "");
		return false;
	}

	if (!SCSCallbackConfigValidate(self->type, self->config.ptr, self->config.size)) {
		// Regular case
		return false;
	}

	if (SCSTimespecCompare(timestamp, self->timestamp.next, <)) {
		// Regular case
		return false;
	}

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSCallbackListEntryInitialize(SCSCallbackListEntry * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSCallbackListEntry));

	SCSAtomicReferenceInitialize(self->reference);
	SCSMutexInitialize(self->mutex);
	self->id = SCS_CALLBACKID_INVVAL;
	self->type = SCS_CALLBACKTYPE_NONE;
	//self->config.ptr = NULL;
	//self->config.size = 0;
	SCSTimespecInitialize(self->timestamp.last);
	SCSTimespecInitialize(self->timestamp.next);

}
void SCSCallbackListEntryFinalize(SCSCallbackListEntry * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	SCSCallbackConfigFinalize(self->type, self->config.ptr, self->config.size);

	SCSAtomicReferenceFinalize(self->reference);
	SCSMutexFinalize(self->mutex);
	self->id = SCS_CALLBACKID_INVVAL;
	self->type = SCS_CALLBACKTYPE_NONE;
	if (self->config.ptr != NULL) {
		free(self->config.ptr);
		self->config.ptr = NULL;
	}
	//self->config.size = 0;
	SCSTimespecInitialize(self->timestamp.last);
	SCSTimespecInitialize(self->timestamp.next);

	memset(self, 0, sizeof(SCSCallbackListEntry));

}

/* ---------------------------------------------------------------------------------------------- */

static bool _SCSCallbackListEntryCanDestroy(SCSCallbackListEntry * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	if (SCSAtomicReferenceCanDestroy(self->reference) == false) {
		return false;
	}

	return true;
}

SCSCallbackListEntry * SCSCallbackListEntryCreate(void) {
	SCSCallbackListEntry * tmp_self;

	if ((tmp_self = (SCSCallbackListEntry *) malloc(sizeof(SCSCallbackListEntry))) == NULL) {
		SCS_LOG(ALERT, MEMORY, 00002, "<<%zu>>", sizeof(SCSCallbackListEntry));
		return NULL;
	}

	SCSCallbackListEntryInitialize(tmp_self);

	SCSObjectCounterIncreaseCreate(_counter);

	return tmp_self;
}
static bool _SCSCallbackListEntryDestroy(void * self) {

	if (self == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99998, "<<%zu>>", sizeof(SCSCallbackListEntry));
		return true;
	}

	if (_SCSCallbackListEntryCanDestroy((SCSCallbackListEntry *) self) == false) {
		return false;
	}

	SCSCallbackListEntryFinalize((SCSCallbackListEntry *) self);
	free(self);

	SCSObjectCounterIncreaseDestroyed(_counter);

	return true;
}
void SCSCallbackListEntryDestroy(SCSCallbackListEntry * self) {
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

	if ((tmp_garbage = SCSGarbageCreate(self, _SCSCallbackListEntryDestroy)) == NULL) {
		SCS_LOG(ERROR, MEMORY, 00001, "<<%zu>>", sizeof(SCSCallbackListEntry));
		return;
	}

	SCSGarbageCollectorPush(&_scs_global_gc, tmp_garbage);

}

bool SCSCallbackListEntryHold(SCSCallbackListEntry * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	return SCSAtomicReferenceIncrease(self->reference);
}
void SCSCallbackListEntryFree(SCSCallbackListEntry * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	SCSAtomicReferenceDecrease(self->reference);

}

/* ---------------------------------------------------------------------------------------------- */

bool SCSCallbackListEntryCall(SCSCallbackListEntry * self, scs_timespec timestamp) {
	bool tmp_retval;
	scs_time tmp_interval;

	tmp_retval = true;
	tmp_interval = 0;

	_SCS_LOCK(self);

	if (_SCSCallbackListEntryCanCall(self, timestamp)) {
		if (SCSCallCallbackFunction(self->type, //
				self->config.ptr, self->config.size, self->id, &tmp_interval) == true) {
			SCSTimespecIncreaseSec(timestamp, tmp_interval);
			SCSTimespecCopy(self->timestamp.next, timestamp);
		}
		else {
			SCS_LOG(WARN, SYSTEM, 79851, "<<%zu>>", self->id);
			tmp_retval = false;
		}
	}

	_SCS_UNLOCK(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSCallbackConfigMonitor(void) {

	SCSObjectCounterMonitor("Callback");

}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

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

void SCSCallbackListInitialize(SCSCallbackList * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSCallbackList));

	SCSMutexInitialize(self->mutex);
	//self->head = NULL;
	//self->capacity = 0;
	//self->count = 0;

}
void SCSCallbackListFinalize(SCSCallbackList * self) {
	SCSCallbackId i;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	for (i = 0; i < self->capacity; i++) {
		SCSCallbackListEntry * tmp_entry;

		if ((tmp_entry = self->entries[i]) != NULL) {
			SCSCallbackListEntryDestroy(tmp_entry);
			SCSCallbackListEntryFree(tmp_entry);
		}
	}

	if (self->entries != NULL) {
		free(self->entries);
		self->entries = NULL;
	}
	//self->capacity = 0;
	//self->count = 0;

	memset(self, 0, sizeof(SCSCallbackList));

}

/* ---------------------------------------------------------------------------------------------- */

static inline bool _SCSCallbackListStandBy(SCSCallbackList * self, size_t capacity) {
	SCSCallbackListEntry ** tmp_ptr;
	SCSCallbackId i;

	if (0 < self->capacity) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		return false;
	}

	tmp_ptr = (SCSCallbackListEntry **) calloc(sizeof(SCSCallbackListEntry *), capacity);
	if (tmp_ptr == NULL) {
		SCS_LOG(ERROR, MEMORY, 00002, "<<%zu>>", (sizeof(SCSCallbackList) * capacity));
		return false;
	}

	for (i = 0; i < capacity; i++) {
		tmp_ptr[i] = NULL;
	}

	self->entries = tmp_ptr;
	self->capacity = capacity;
	self->count = 0;

	return true;
}
bool SCSCallbackListStandBy(SCSCallbackList * self, size_t capacity) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (capacity < SCS_CALLBACKLIST_MINENTRIES || SCS_CALLBACKLIST_MAXENTRIES < capacity) {
		SCS_LOG(WARN, SYSTEM, 99997, "<<%zu,%zu,%zu>>", //
				SCS_CALLBACKLIST_MINENTRIES, capacity, SCS_CALLBACKLIST_MAXENTRIES);
		return false;
	}

	_SCS_LOCK(self);

	tmp_result = _SCSCallbackListStandBy(self, capacity);

	_SCS_UNLOCK(self);

	return tmp_result;
}

static inline bool _SCSCallbackListAdd(SCSCallbackList * __restrict self, SCSCallbackType type,
		void * __restrict ptr, size_t size, SCSCallbackId * __restrict out) {
	void * tmp_ptr;
	size_t i;

	if (self->capacity <= self->count) {
		SCS_LOG(WARN, SYSTEM, 79994, "<<%zu/%zu>>", self->count, self->capacity);
		return false;
	}

	tmp_ptr = NULL;

	for (i = 0; i < self->capacity; i++) {
		if (self->entries[i] == NULL) {
			SCSCallbackId tmp_id;
			SCSCallbackListEntry * tmp_entry;

			tmp_id = i + 1;

			if ((tmp_ptr = malloc(size)) != NULL) {
				memcpy(tmp_ptr, ptr, size);
			}
			else {
				SCS_LOG(ALERT, MEMORY, 00002, "<<%zu>>", size);
				break;
			}

			if ((tmp_entry = SCSCallbackListEntryCreate()) != NULL) {
				SCSCallbackListEntryHold(tmp_entry);
			}
			else {
				SCS_LOG(WARN, SYSTEM, 99999, "");
				break;
			}

			tmp_entry->id = tmp_id;
			tmp_entry->type = type;
			tmp_entry->config.ptr = tmp_ptr;
			tmp_entry->config.size = size;
			SCSTimespecSetZero(tmp_entry->timestamp.last);
			SCSTimespecSetZero(tmp_entry->timestamp.next);

			self->entries[i] = tmp_entry;
			self->count++;

			if (out != NULL) {
				*out = tmp_id;
			}

			return true;
		}
	}

	if (tmp_ptr != NULL) {
		free(tmp_ptr);
	}

	return false;
}
bool SCSCallbackListAdd(SCSCallbackList * __restrict self, SCSCallbackType type,
		void * __restrict ptr, size_t size, SCSCallbackId * out) {
	SCSCallbackId tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (!SCSCallbackTypeValidate(type)) {
		SCS_LOG(WARN, SYSTEM, 99998, "<<%d>>", type);
		return false;
	}

	if (!SCSCallbackConfigValidate(type, ptr, size)) {
		SCS_LOG(WARN, SYSTEM, 99998, "<<%d>>", type);
		return false;
	}

	if (ptr == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (size < 1) {
		SCS_LOG(WARN, SYSTEM, 99997, "<<%zu>>", size);
		return false;
	}

	//if (out == NULL) {
	//	SCS_LOG(WARN, SYSTEM, 99998, "");
	//	return false;
	//}

	_SCS_LOCK(self);

	tmp_result = _SCSCallbackListAdd(self, type, ptr, size, out);

	_SCS_UNLOCK(self);

	return tmp_result;
}
static inline bool _SCSCallbackListRemove(SCSCallbackList * self, SCSCallbackId id) {
	SCSCallbackListEntry * tmp_entry;
	size_t tmp_index;

	//if (id < 1) {
	//	SCS_LOG(WARN, SYSTEM, 99997, "<<%d>>", id);
	//	return false;
	//}

	if (self->capacity <= id) {
		SCS_LOG(WARN, SYSTEM, 99997, "<<%d>>", id);
		return false;
	}

	tmp_index = id - 1;

	if ((tmp_entry = self->entries[tmp_index]) != NULL) {
		self->entries[tmp_index] = NULL;
		self->count--;
	}
	else {
		SCS_LOG(WARN, SYSTEM, 99997, "<<%d>>", id);
		return false;
	}

	SCSCallbackListEntryDestroy(tmp_entry);
	SCSCallbackListEntryFree(tmp_entry);

	return true;
}
bool SCSCallbackListRemove(SCSCallbackList * self, SCSCallbackId id) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (!SCSCallbackIdValidate(id)) {
		SCS_LOG(WARN, SYSTEM, 99997, "<<%d>>", id);
		return false;
	}

	_SCS_LOCK(self);

	tmp_result = _SCSCallbackListRemove(self, id);

	_SCS_UNLOCK(self);

	return tmp_result;
}

static inline void _SCSCallbackListCall(SCSCallbackList * self) {
	scs_timespec tmp_timestamp;
	size_t tmp_num;
	size_t tmp_counter;
	size_t i;

	SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);
	tmp_num = self->count;
	tmp_counter = 0;

	for (i = 0; i < self->capacity; i++) {
		if (self->entries[i] != NULL) {
			if (SCSCallbackListEntryCall(self->entries[i], tmp_timestamp) == false) {
				SCSCallbackListEntry * tmp_entry;

				if ((tmp_entry = self->entries[i]) != NULL) {
					self->entries[i] = NULL;
					self->count--;

					SCSCallbackListEntryDestroy(tmp_entry);
					SCSCallbackListEntryFree(tmp_entry);

					SCS_LOG(WARN, SYSTEM, 99997, "<<%d>>", i);
				}
			}

			if (tmp_num <= ++tmp_counter) {
				break;
			}
		}
	}

}
void SCSCallbackListCall(SCSCallbackList * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	_SCS_LOCK(self);

	_SCSCallbackListCall(self);

	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
