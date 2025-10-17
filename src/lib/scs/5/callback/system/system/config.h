#ifndef SCS_5_CALLBACK_SYSTEM_SYSTEM_CONFIG_H_
#define SCS_5_CALLBACK_SYSTEM_SYSTEM_CONFIG_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/5/callback/system/system/types.h"

/* ---------------------------------------------------------------------------------------------- */

extern void SCSSystemCallbackConfigInitialize(SCSSystemCallbackConfig * self);
extern void SCSSystemCallbackConfigFinalize(SCSSystemCallbackConfig * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSSystemCallbackConfigValidate(SCSSystemCallbackConfig * self);

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SYSTEM_SYSTEM_CONFIG_H_ */
