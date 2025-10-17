#ifndef SCS_5_CALLBACK_SYSTEM_SYSTEM_STATUS_H_
#define SCS_5_CALLBACK_SYSTEM_SYSTEM_STATUS_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/5/callback/system/system/types.h"

/* ---------------------------------------------------------------------------------------------- */

extern void SCSSystemCallbackStatusInitialize(SCSSystemCallbackStatus * self);
extern void SCSSystemCallbackStatusFinalize(SCSSystemCallbackStatus * self);

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SYSTEM_SYSTEM_STATUS_H_ */
