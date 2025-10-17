#ifndef SCS_5_CALLBACK_SOCKET_FEEDBACK_TYPES_H_
#define SCS_5_CALLBACK_SOCKET_FEEDBACK_TYPES_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/2/atomic.h"
#include "scs/2/lock.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSFeedbackCallbackConfig SCSFeedbackCallbackConfig;

/* ---------------------------------------------------------------------------------------------- */

typedef size_t (*SCSFeedbackCallbackFunction)(													//
		SCSFeedbackCallbackConfig * __restrict config, 											//
		uint8_t * __restrict ptr,  																//
		size_t size);

/* ---------------------------------------------------------------------------------------------- */

struct SCSFeedbackCallbackConfig {
	SCSFeedbackCallbackFunction func;
	struct {
		void * ptr;
		size_t size;
	} argument;
};

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SOCKET_FEEDBACK_TYPES_H_ */
