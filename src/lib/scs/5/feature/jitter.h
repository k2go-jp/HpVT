#ifndef SCS_5_FEATURE_JITTER_H_
#define SCS_5_FEATURE_JITTER_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/2/lock.h"
#include "scs/5/packet/types.h"

/* ---------------------------------------------------------------------------------------------- */

#define SCS_JITTERMESUERER_MINENTRIES	(1)
#define SCS_JITTERMESUERER_MAXENTRIES	UINT32_MAX

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSJitterStatus {
	SCSPacketSeqno seqno;
	struct {
		SCSPacketTimestamp sent;
		scs_timespec received;
	} timestamp;
} SCSJitterStatus;

extern void SCSJitterStatusInitialize(SCSJitterStatus * self);
extern void SCSJitterStatusFinalize(SCSJitterStatus * self);

#define SCSJitterStatusCopy(xxx_dst, xxx_src) \
		memcpy(xxx_dst, xxx_src, sizeof(SCSJitterStatus))

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSJitterMeasurer {
	scs_mutex mutex;
	SCSJitterStatus * values;
	size_t capacity;
	struct {
		size_t minimum;
		size_t maximum;
		size_t current;
		size_t previous;
	} index;
} SCSJitterMeasurer;

extern void SCSJitterMeasurerInitialize(SCSJitterMeasurer * self);
extern void SCSJitterMeasurerFinalize(SCSJitterMeasurer * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSJitterMeasurerStandBy(SCSJitterMeasurer * self, size_t capacity);

extern void SCSJitterMeasurerUpdate(															//
		SCSJitterMeasurer * self, 																//
		SCSPacketSeqno seqno,																	//
		SCSPacketTimestamp timestamp);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSJitterMeasurerGetLast(															//
		SCSJitterMeasurer * __restrict self,													//
		SCSJitterStatus * __restrict result);

/* ============================================================================================== */

#endif /* SCS_5_FEATURE_JITTER_H_ */
