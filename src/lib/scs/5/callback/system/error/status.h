#ifndef SCS_5_CALLBACK_SYSTEM_ERROR_STATUS_H_
#define SCS_5_CALLBACK_SYSTEM_ERROR_STATUS_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/5/callback/system/error/types.h"

/* ---------------------------------------------------------------------------------------------- */

extern void SCSErrorCallbackStatusInitialize(SCSErrorCallbackStatus * self);
extern void SCSErrorCallbackStatusFinalize(SCSErrorCallbackStatus * self);

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SYSTEM_ERROR_STATUS_H_ */
