#ifndef SCS_5_CALLBACK_SOCKET_REDUNDANCY_TYPES_H_
#define SCS_5_CALLBACK_SOCKET_REDUNDANCY_TYPES_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSRedundancyCallbackResult SCSRedundancyCallbackResult;

typedef struct SCSRedundancyCallbackStatus SCSRedundancyCallbackStatus;

typedef struct SCSRedundancyCallbackConfig SCSRedundancyCallbackConfig;

/* ---------------------------------------------------------------------------------------------- */

typedef SCSRedundancyCallbackResult (*SCSRedundancyCallbackFunction)(							//
		SCSRedundancyCallbackConfig * __restrict config, 										//
		__const SCSRedundancyCallbackStatus * __restrict status);

/* ---------------------------------------------------------------------------------------------- */

struct SCSRedundancyCallbackConfig {
	SCSRedundancyCallbackFunction func;
	struct {
		void * ptr;
		size_t size;
	} argument;
	int times;
};

/* ---------------------------------------------------------------------------------------------- */

struct SCSRedundancyCallbackStatus {
	scs_timespec interval;
	int times;
};

/* ---------------------------------------------------------------------------------------------- */

struct SCSRedundancyCallbackResult {
	scs_timespec interval;
	bool cancel;
};

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SOCKET_REDUNDANCY_TYPES_H_ */
