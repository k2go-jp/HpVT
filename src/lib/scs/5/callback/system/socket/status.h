#ifndef SCS_5_CALLBACK_SYSTEM_SOCKET_STATUS_H_
#define SCS_5_CALLBACK_SYSTEM_SOCKET_STATUS_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/5/callback/system/socket/types.h"

/* ---------------------------------------------------------------------------------------------- */

extern void SCSSocketCallbackStatusInitialize(SCSSocketCallbackStatus * self);
extern void SCSSocketCallbackStatusFinalize(SCSSocketCallbackStatus * self);

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SYSTEM_SOCKET_STATUS_H_ */
