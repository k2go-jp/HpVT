#define SCS_SOURCECODE_FILEID	"5FTRPLR"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/inttypes.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/1/time.h"
#include "scs/1/unistd.h"
#include "scs/2/logger.h"
#include "scs/5/feature/plr.h"
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

inline void SCSPLREntryInitialize(SCSPLREntry * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	memset(self, 0, sizeof(SCSPLREntry));

	SCSTimespecInitialize(self->timestamp);
	//self->seqno = 0;
	//self->packets.sent = 0;
	//self->packets.received = 0;
	//self->packets.lost = 0;
	SCSTimespecInitialize(self->packets.interval);

}
inline void SCSPLREntryFinalize(SCSPLREntry * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSTimespecFinalize(self->timestamp);
	//self->seqno = 0;
	//self->packets.sent = 0;
	//self->packets.received = 0;
	//self->packets.lost = 0;
	SCSTimespecFinalize(self->packets.interval);

	memset(self, 0, sizeof(SCSPLREntry));

}

/* ---------------------------------------------------------------------------------------------- */

void SCSPLRMeasurerInitialize(SCSPLRMeasurer * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSPLRMeasurer));

	SCSMutexInitialize(self->mutex);
	//self->values = NULL;
	//self->capacity = NULL;
	//self->index.minimum = 0;
	//self->index.maximum = 0;
	//self->index.current = 0;
	//self->index.prev = 0;

	//self->lost.minimum = 0;
	//self->lost.maximum = 0;
	//self->lost.current = 0;
	//self->lost.average = 0;

}
void SCSPLRMeasurerFinalize(SCSPLRMeasurer * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	if (0 < self->capacity) {
		int i;
		for (i = 0; i < self->capacity; i++) {
			SCSPLREntryFinalize(&self->entries[i]);
		}
	}

	SCSMutexInitialize(self->mutex);
	if (self->entries != NULL) {
		free(self->entries);
		self->entries = NULL;
	}
	//self->capacity = NULL;
	//self->index.minimum = 0;
	//self->index.maximum = 0;
	//self->index.current = 0;
	//self->index.prev = 0;

	//self->lost.minimum = 0;
	//self->lost.maximum = 0;
	//self->lost.current = 0;
	//self->lost.average = 0;

	memset(self, 0, sizeof(SCSPLRMeasurer));

}

/* ---------------------------------------------------------------------------------------------- */

static inline bool _SCSPLRStandBy(SCSPLRMeasurer * self, size_t capacity) {
	SCSPLREntry * tmp_ptr;
	int i;

	if ((tmp_ptr = (SCSPLREntry *) calloc(sizeof(SCSPLREntry), capacity)) == NULL) {
		SCS_LOG(ALERT, MEMORY, 99999, "<<%zu>>", (sizeof(scs_timespec) * capacity));
		return false;
	}

	for (i = 0; i < capacity; i++) {
		SCSPLREntryInitialize(&tmp_ptr[i]);
	}

	self->entries = tmp_ptr;
	self->capacity = capacity;
	self->index.minimum = 0;
	self->index.maximum = (capacity - 1);
	self->index.current = 0;
	self->index.prev = 0;
	self->rate.minimum = INT_MAX;
	self->rate.maximum = 0;
	self->rate.latest = 0;

	return true;
}
bool SCSPLRMeasurerStandBy(SCSPLRMeasurer * self, size_t capacity) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (capacity < SCS_PLRMEASURER_MINENTRIES || SCS_PLRMEASURER_MAXENTRIES < capacity) {
		SCS_LOG(WARN, SYSTEM, 99997, "<<%d,%d,%d>>", //
				SCS_PLRMEASURER_MINENTRIES, capacity, SCS_PLRMEASURER_MAXENTRIES);
		return false;
	}

	_SCS_LOCK(self);
	tmp_result = _SCSPLRStandBy(self, capacity);
	_SCS_UNLOCK(self);

	return tmp_result;
}

static inline void _SCSPLRUpdate(SCSPLRMeasurer * self, SCSPacketSeqno seqno, uint64_t sent,
		uint64_t received) {
	SCSPLREntry * tmp_current;
	SCSPLREntry * tmp_prev;

	if (self->capacity < 1) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		return;
	}

	tmp_current = &self->entries[self->index.current];
	tmp_prev = &self->entries[self->index.prev];

	if (sent < received) {
		SCS_LOG(WARN, PACKET, 99999, "<<%zu,%zu>>", sent, received);
		return;
	}

	if (sent < tmp_prev->packets.sent) {
		SCS_LOG(WARN, PACKET, 99999, "<<%zu,%zu>>", sent, tmp_current->packets.sent);
		return;
	}

	SCSPLREntryInitialize(tmp_current);
	SCSTimespecSetCurrentTime(tmp_current->timestamp, CLOCK_MONOTONIC);
	tmp_current->seqno = seqno;
	tmp_current->packets.sent = sent;
	tmp_current->packets.received = received;
	tmp_current->packets.lost = sent - received;
	SCSTimespecSub(tmp_current->timestamp, tmp_prev->timestamp, tmp_current->packets.interval);

	//SCS_LOG(NOTICE, DEBUG, 00000, "%"PRIu64",%"PRIu64",%"PRIu64",%"PRIu64",%zu.%09ld", //
	//		tmp_prev->seqno,//
	//		tmp_prev->packets.sent,//
	//		tmp_prev->packets.received,//
	//		tmp_prev->packets.lost,//
	//		tmp_prev->packets.interval.tv_sec,//
	//		tmp_prev->packets.interval.tv_nsec);
	//SCS_LOG(NOTICE, DEBUG, 00000, "%"PRIu64",%"PRIu64",%"PRIu64",%"PRIu64",%zu.%09ld", //
	//		tmp_current->seqno,//
	//		tmp_current->packets.sent,//
	//		tmp_current->packets.received,//
	//		tmp_current->packets.lost,//
	//		tmp_current->packets.interval.tv_sec,//
	//		tmp_current->packets.interval.tv_nsec);

	self->index.prev = self->index.current;
	if (self->index.maximum <= ++self->index.current) {
		self->index.current = self->index.minimum;
	}

	if (tmp_prev->packets.lost < tmp_current->packets.lost) {
		uint64_t tmp_lost;
		uint64_t tmp_sent;
		uint32_t tmp_rate;

		tmp_lost = (tmp_current->packets.lost - tmp_prev->packets.lost);
		tmp_sent = (tmp_current->packets.sent - tmp_prev->packets.sent);
		tmp_rate = (int32_t) (((double) tmp_lost / (double) tmp_sent) * (100.0 * 1000.0));

		if (tmp_rate < self->rate.minimum) {
			self->rate.minimum = tmp_rate;
		}

		if (self->rate.maximum < tmp_rate) {
			self->rate.maximum = tmp_rate;
		}

		self->rate.latest = tmp_rate;

		/*
		 SCS_LOG(NOTICE, DEBUG, 00000, //
		 "%"PRIu64",%"PRIu64",%"PRIu32",%"PRIu32",%"PRIu32"(%"PRIu32")", //
		 tmp_lost, //
		 tmp_sent, //
		 self->rate.minimum, //
		 self->rate.maximum, //
		 self->rate.latest, //
		 tmp_rate);
		 */
	}
	else {
		self->rate.latest = 0;

		/*
		 SCS_LOG(NOTICE, DEBUG, 00000, "%"PRIu32",%"PRIu32",%"PRIu32, //
		 self->rate.minimum, //
		 self->rate.maximum, //
		 self->rate.latest);
		 */
	}

}
void SCSPLRMeasurerUpdate(SCSPLRMeasurer * self, SCSPacketSeqno seqno, uint64_t sent,
		uint64_t received) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	_SCS_LOCK(self);
	_SCSPLRUpdate(self, seqno, sent, received);
	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

uint32_t SCSPLRMeasurerGetLatest(SCSPLRMeasurer * self) {
	int tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return -1;
	}

	_SCS_LOCK(self);

	if (0 < self->capacity) {
		tmp_result = self->rate.latest;
	}

	_SCS_UNLOCK(self);

	return tmp_result;
}
uint32_t SCSPLRMeasurerGetMin(SCSPLRMeasurer * self) {
	int tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return -1;
	}

	_SCS_LOCK(self);

	if (0 < self->capacity) {
		tmp_result = self->rate.minimum;
	}

	_SCS_UNLOCK(self);

	return tmp_result;
}
uint32_t SCSPLRMeasurerGetMax(SCSPLRMeasurer * self) {
	int tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return -1;
	}

	_SCS_LOCK(self);

	if (0 < self->capacity) {
		tmp_result = self->rate.maximum;
	}

	_SCS_UNLOCK(self);

	return tmp_result;
}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
