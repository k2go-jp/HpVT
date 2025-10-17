#ifndef SCS_5_FEATURE_RTT_H_
#define SCS_5_FEATURE_RTT_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/2/lock.h"

/* ---------------------------------------------------------------------------------------------- */

#define SCS_RTTMEASURER_MINENTRIES	(8)
#define SCS_RTTMEASURER_MAXENTRIES	(128)

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSRTTMeasurer {
	scs_mutex mutex;
	scs_timespec * values;
	size_t capacity;
	struct {
		size_t minimum;
		size_t maximum;
		size_t current;
		size_t previous;
	} index;
	struct {
		scs_timespec minimum;
		scs_timespec maximum;
	} value;
} SCSRTTMeasurer;

extern void SCSRTTMeasurerInitialize(SCSRTTMeasurer * self);
extern void SCSRTTMeasurerFinalize(SCSRTTMeasurer * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSRTTMeasurerStandBy(SCSRTTMeasurer * self, size_t capacity);

extern void SCSRTTMeasurerUpdate(SCSRTTMeasurer * self, scs_timespec timestamp);

/* ---------------------------------------------------------------------------------------------- */

extern scs_timespec SCSRTTMeasurerGetLatest(SCSRTTMeasurer * self);
extern scs_timespec SCSRTTMeasurerGetMin(SCSRTTMeasurer * self);
extern scs_timespec SCSRTTMeasurerGetMax(SCSRTTMeasurer * self);

/* ============================================================================================== */

#endif /* SCS_5_FEATURE_RTT_H_ */
