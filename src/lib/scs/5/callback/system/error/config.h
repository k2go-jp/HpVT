#ifndef SCS_5_CALLBACK_SYSTEM_ERROR_CONFIG_H_
#define SCS_5_CALLBACK_SYSTEM_ERROR_CONFIG_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/5/callback/system/error/types.h"

/* ---------------------------------------------------------------------------------------------- */

extern void SCSErrorCallbackConfigInitialize(SCSErrorCallbackConfig * self);
extern void SCSErrorCallbackConfigFinalize(SCSErrorCallbackConfig * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSErrorCallbackConfigValidate(SCSErrorCallbackConfig * self);

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SYSTEM_ERROR_CONFIG_H_ */
