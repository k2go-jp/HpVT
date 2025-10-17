#ifndef SCS_5_CALLBACK_SOCKET_REDUNDANCY_STATUS_H_
#define SCS_5_CALLBACK_SOCKET_REDUNDANCY_STATUS_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/5/callback/socket/redundancy/types.h"

/* ---------------------------------------------------------------------------------------------- */

extern void SCSRedundancyCallbackStatusInitialize(SCSRedundancyCallbackStatus * self);
extern void SCSRedundancyCallbackStatusFinalize(SCSRedundancyCallbackStatus * self);

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SOCKET_REDUNDANCY_STATUS_H_ */
