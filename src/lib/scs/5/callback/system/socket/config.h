#ifndef SCS_5_CALLBACK_SYSTEM_SOCKET_CONFIG_H_
#define SCS_5_CALLBACK_SYSTEM_SOCKET_CONFIG_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/5/callback/system/socket/types.h"

/* ---------------------------------------------------------------------------------------------- */

extern void SCSSocketCallbackConfigInitialize(SCSSocketCallbackConfig * self);
extern void SCSSocketCallbackConfigFinalize(SCSSocketCallbackConfig * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSSocketCallbackConfigValidate(SCSSocketCallbackConfig * self);

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SYSTEM_SOCKET_CONFIG_H_ */
