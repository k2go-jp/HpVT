#ifndef SCS_5_CALLBACK_SYSTEM_SYSTEM_TYPES_H_
#define SCS_5_CALLBACK_SYSTEM_SYSTEM_TYPES_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/2/atomic.h"
#include "scs/2/lock.h"
#include "scs/5/callback/system/types.h"
#include "scs/5/types.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSSystemCallbackConfig SCSSystemCallbackConfig;
typedef struct SCSSystemCallbackStatus SCSSystemCallbackStatus;

/* ---------------------------------------------------------------------------------------------- */

typedef void (*SCSSystemCallbackFunction)(														//
		SCSSystemCallbackConfig * __restrict config, 											//
		__const SCSSystemCallbackStatus * __restrict status);

/* ---------------------------------------------------------------------------------------------- */

struct SCSSystemCallbackConfig {
	SCSSystemCallbackFunction func;
	struct {
		void * ptr;
		size_t size;
	} argument;
	scs_time interval;
};

/* ---------------------------------------------------------------------------------------------- */

struct SCSSystemCallbackStatus {
	SCSCallbackId id;
	SCSErrorInfo error;
	SCSNetworkTrafficInfo traffic;
};

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SYSTEM_SYSTEM_TYPES_H_ */
