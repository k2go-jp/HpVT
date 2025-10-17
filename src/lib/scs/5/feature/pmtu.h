#ifndef SCS_5_FEATURE_PMTU_H_
#define SCS_5_FEATURE_PMTU_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/2/lock.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPMTUMeasurer {
	scs_mutex mutex;
	int value;
	scs_timespec lifetime;
	scs_timespec timeout;
	struct {
		size_t maximum;
		size_t current;
		size_t diff;
		struct {
			int maximum;
			int counter;
		} cycle;
	} predited;
} SCSPMTUMeasurer;

extern void SCSPMTUMeasurerInitialize(SCSPMTUMeasurer * self);
extern void SCSPMTUMeasurerFinalize(SCSPMTUMeasurer * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSPMTUMeasurerUpdate(SCSPMTUMeasurer * self, int value);
extern size_t SCSPMTUMeasurerPredict(SCSPMTUMeasurer * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSPMTUMeasurerSetLifetime(SCSPMTUMeasurer * self, scs_timespec value);

extern int SCSPMTUMeasurerGet(SCSPMTUMeasurer * self);

/* ============================================================================================== */

#endif /* SCS_5_FEATURE_PMTU_H_ */
