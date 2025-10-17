#ifndef SCS_5_CALLBACK_SOCKET_REDUNDANCY_FUNCTIONS_H_
#define SCS_5_CALLBACK_SOCKET_REDUNDANCY_FUNCTIONS_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/5/callback/socket/redundancy/types.h"

/* ---------------------------------------------------------------------------------------------- */

extern SCSRedundancyCallbackResult SCSConnectionCallbackFunction( 								//
		SCSRedundancyCallbackConfig * __restrict config, 										//
		__const SCSRedundancyCallbackStatus * __restrict status);

/* ---------------------------------------------------------------------------------------------- */

extern SCSRedundancyCallbackResult SCSCloseCallbackFunction(									//
		SCSRedundancyCallbackConfig * __restrict config,										//
		__const SCSRedundancyCallbackStatus * __restrict status);

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SOCKET_REDUNDANCY_FUNCTIONS_H_ */
