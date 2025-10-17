#ifndef SCS_5_CALLBACK_SOCKET_REDUNDANCY_RESULT_H_
#define SCS_5_CALLBACK_SOCKET_REDUNDANCY_RESULT_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/5/callback/socket/redundancy/types.h"

/* ---------------------------------------------------------------------------------------------- */

extern void SCSRedundancyCallbackResultInitialize(SCSRedundancyCallbackResult * self);
extern void SCSRedundancyCallbackResultFinalize(SCSRedundancyCallbackResult * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSRedundancyCallbackResultValidate(SCSRedundancyCallbackResult * self);

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SOCKET_REDUNDANCY_RESULT_H_ */
