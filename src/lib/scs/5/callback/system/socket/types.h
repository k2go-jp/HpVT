#ifndef SCS_5_CALLBACK_SYSTEM_SOCKET_TYPES_H_
#define SCS_5_CALLBACK_SYSTEM_SOCKET_TYPES_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/2/atomic.h"
#include "scs/2/lock.h"
#include "scs/5/callback/system/types.h"
#include "scs/5/feedback.h"
#include "scs/5/socket/types.h"
#include "scs/5/types.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSSocketCallbackConfig SCSSocketCallbackConfig;
typedef struct SCSSocketCallbackStatus SCSSocketCallbackStatus;

/* ---------------------------------------------------------------------------------------------- */

typedef void (*SCSSocketCallbackFunction)( 														//
		SCSSocketCallbackConfig * __restrict config, 											//
		__const SCSSocketCallbackStatus * __restrict status);

/* ---------------------------------------------------------------------------------------------- */

struct SCSSocketCallbackConfig {
	SCSSocketCallbackFunction func;
	struct {
		void * ptr;
		size_t size;
	} argument;
	scs_time interval;
	SCSSocketId id;
};

/* ---------------------------------------------------------------------------------------------- */

struct SCSSocketCallbackStatus {
	SCSCallbackId id;
	struct {
		SCSErrorInfo error;
		SCSNetworkTrafficInfo traffic;
	} system;
	struct {
		SCSNetworkTrafficInfo traffic;
		struct {
			SCSFeedbackInfo basic;
			struct {
				uint8_t * ptr;
				size_t size;
			} extension;
		} feedback;
		uint32_t plr; // % * 1000
		scs_timespec rtt;
		int mtu;
		int pmtu;
		SCSSocketBufferStatus buffer;
	} socket;
};

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SYSTEM_SOCKET_TYPES_H_ */
