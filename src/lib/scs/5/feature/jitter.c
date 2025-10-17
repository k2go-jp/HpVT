#define SCS_SOURCECODE_FILEID	"5FTRJTR"

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
#include "scs/5/feature/jitter.h"
#include "scs/5/packet/types.h"
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
			SCS_LOG(ALERT, SOCKET, 99999, "");\
			abort();\
		}
#define _SCS_UNLOCK(xxx_self) \
		if (SCSMutexUnlock((xxx_self)->mutex) != 0) {\
			SCS_LOG(ALERT, SOCKET, 99999, "");\
			abort();\
		}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSJitterStatusInitialize(SCSJitterStatus * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	memset(self, 0, sizeof(SCSJitterStatus));

	//self->seqno = 0;
	self->timestamp.sent = SCS_PKTTS_INVVAL;
	SCSTimespecInitialize(self->timestamp.received);

}
inline void SCSJitterStatusFinalize(SCSJitterStatus * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

//self->seqno = 0;
	self->timestamp.sent = SCS_PKTTS_INVVAL;
	SCSTimespecFinalize(self->timestamp.received);

	memset(self, 0, sizeof(SCSJitterStatus));

}

/* ---------------------------------------------------------------------------------------------- */

void SCSJitterMeasurerInitialize(SCSJitterMeasurer * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSJitterMeasurer));

	SCSMutexInitialize(self->mutex);
	//self->values = NULL;
	//self->capacity = NULL;
	//self->index.minimum = 0;
	//self->index.maximum = 0;
	//self->index.current = 0;

}
void SCSJitterMeasurerFinalize(SCSJitterMeasurer * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	if (0 < self->capacity) {
		int i;
		for (i = 0; i < self->capacity; i++) {
			SCSJitterStatusFinalize(&self->values[i]);
		}
	}

	SCSMutexInitialize(self->mutex);
	if (self->values != NULL) {
		free(self->values);
		self->values = NULL;
	}
	//self->capacity = NULL;
	//self->index.minimum = 0;
	//self->index.maximum = 0;
	//self->index.current = 0;

	memset(self, 0, sizeof(SCSJitterMeasurer));

}

/* ---------------------------------------------------------------------------------------------- */

static inline bool _SCSJitterStandBy(SCSJitterMeasurer * self, size_t capacity) {
	SCSJitterStatus * tmp_ptr;
	int i;

	if ((tmp_ptr = (SCSJitterStatus *) calloc(sizeof(SCSJitterStatus), capacity)) == NULL) {
		SCS_LOG(ALERT, MEMORY, 99999, "<<%zu>>", (sizeof(scs_timespec) * capacity));
		return false;
	}

	for (i = 0; i < capacity; i++) {
		SCSJitterStatusInitialize(&tmp_ptr[i]);
	}

	self->values = tmp_ptr;
	self->capacity = capacity;
	self->index.minimum = 0;
	self->index.maximum = (capacity - 1);
	self->index.current = 0;
	self->index.previous = 0;

	return true;
}
bool SCSJitterMeasurerStandBy(SCSJitterMeasurer * self, size_t capacity) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (capacity < SCS_JITTERMESUERER_MINENTRIES || SCS_JITTERMESUERER_MAXENTRIES < capacity) {
		SCS_LOG(WARN, SYSTEM, 99997, "<<%d,%d,%d>>", //
				SCS_JITTERMESUERER_MINENTRIES, capacity, SCS_JITTERMESUERER_MAXENTRIES);
		return false;
	}

	_SCS_LOCK(self);
	tmp_result = _SCSJitterStandBy(self, capacity);
	_SCS_UNLOCK(self);

	return tmp_result;
}

void SCSJitterMeasurerUpdate(SCSJitterMeasurer * self, SCSPacketSeqno seqno,
		SCSPacketTimestamp timestamp) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	_SCS_LOCK(self);

	if (0 < self->capacity) {
		SCSJitterStatus * tmp_entry;

		tmp_entry = &self->values[self->index.current];

		SCSJitterStatusInitialize(tmp_entry);
		tmp_entry->seqno = seqno;
		tmp_entry->timestamp.sent = timestamp;
		SCSTimespecSetCurrentTime(tmp_entry->timestamp.received, CLOCK_MONOTONIC);

		self->index.previous = self->index.current;

		if (self->index.maximum <= ++self->index.current) {
			self->index.current = self->index.minimum;
		}
	}

	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

void SCSJitterMeasurerGetLast(SCSJitterMeasurer * __restrict self,
		SCSJitterStatus * __restrict result) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	if (result == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	_SCS_LOCK(self);

	if (0 < self->capacity) {
		SCSJitterStatusCopy(result, &self->values[self->index.previous]);
	}

	_SCS_UNLOCK(self);

	return;
}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
