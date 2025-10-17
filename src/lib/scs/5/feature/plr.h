#ifndef SCS_5_FEATURE_PLR_H_
#define SCS_5_FEATURE_PLR_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/2/lock.h"
#include "scs/5/packet/types.h"

/* ---------------------------------------------------------------------------------------------- */

#define SCS_PLRMEASURER_MINENTRIES	(3)
#define SCS_PLRMEASURER_MAXENTRIES	(128)

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPLREntry {
	scs_timespec timestamp;
	SCSPacketSeqno seqno;
	struct {
		uint64_t sent;
		uint64_t received;
		uint64_t lost;
		scs_timespec interval;
	} packets;
} SCSPLREntry;

extern void SCSPLREntryInitialize(SCSPLREntry * self);
extern void SCSPLREntryFinalize(SCSPLREntry * self);

#define SCSPLREntryGetLostPackets(xxx_self) \
		((xxx_self)->packets.diff.lost)

#define SCSPLREntryCopy(xxx_dst, xxx_src) \
		memcpy(xxx_dst, xxx_src, sizeof(SCSPLREntry))

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPLRMeasurer {
	scs_mutex mutex;
	SCSPLREntry * entries;
	size_t capacity;
	struct {
		size_t minimum;
		size_t maximum;
		size_t current;
		size_t prev;
	} index;
	struct {
		uint32_t minimum;
		uint32_t maximum;
		uint32_t latest;
	} rate;
} SCSPLRMeasurer;

extern void SCSPLRMeasurerInitialize(SCSPLRMeasurer * self);
extern void SCSPLRMeasurerFinalize(SCSPLRMeasurer * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSPLRMeasurerStandBy(SCSPLRMeasurer * self, size_t capacity);

extern void SCSPLRMeasurerUpdate(																//
		SCSPLRMeasurer * self, 																	//
		SCSPacketSeqno seqno, 																	//
		uint64_t sent, 																			//
		uint64_t received);

/* ---------------------------------------------------------------------------------------------- */

extern uint32_t SCSPLRMeasurerGetLatest(SCSPLRMeasurer * self);
extern uint32_t SCSPLRMeasurerGetMin(SCSPLRMeasurer * self);
extern uint32_t SCSPLRMeasurerGetMax(SCSPLRMeasurer * self);

/* ============================================================================================== */

#endif /* SCS_5_FEATURE_PLR_H_ */
