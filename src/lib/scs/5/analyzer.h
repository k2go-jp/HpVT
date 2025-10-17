#ifndef SCS_5_ANALYZER_H_
#define SCS_5_ANALYZER_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/lock.h"
#include "scs/1/stdbool.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSLossInspector {
	scs_mutex mutex;

	uint64_t packets;
	uint64_t bytes;
} SCSTrafficAnalyzer;

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPathMTUInspector {
	scs_mutex mutex;

	int state;

	struct {
		int measure;
		int sepecify;			//size set by user request.
		int failure;			//ms size send function allowed to sent.
	} mtu;

	struct {
		scs_time length;
		scs_time to;
	} wait;
} SCSPathMTUInspector;

/* ============================================================================================== */

#endif /* SCS_5_ANALYZER_H_ */
