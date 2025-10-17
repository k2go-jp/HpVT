#ifndef SCS_5_CALLBACK_SYSTEM_ERROR_TYPES_H_
#define SCS_5_CALLBACK_SYSTEM_ERROR_TYPES_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/2/logger.h"
#include "scs/5/callback/system/types.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSErrorCallbackConfig SCSErrorCallbackConfig;
typedef struct SCSErrorCallbackStatus SCSErrorCallbackStatus;

/* ---------------------------------------------------------------------------------------------- */

typedef void (*SCSErrorCallbackFunction)(														//
		SCSErrorCallbackConfig * __restrict config, 											//
		__const SCSErrorCallbackStatus * __restrict status);

/* ---------------------------------------------------------------------------------------------- */

struct SCSErrorCallbackConfig {
	scs_atomic_reference reference;
	SCSErrorCallbackFunction func;
	struct {
		void * ptr;
		size_t size;
	} argument;
	scs_time interval;
};

/* ---------------------------------------------------------------------------------------------- */

struct SCSErrorCallbackStatus {
	SCSCallbackId id;
	SCSErrorInfo error;
};

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SYSTEM_ERROR_TYPES_H_ */
