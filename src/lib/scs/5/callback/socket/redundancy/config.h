#ifndef SCS_5_CALLBACK_SOCKET_REDUNDANCY_CONFIG_H_
#define SCS_5_CALLBACK_SOCKET_REDUNDANCY_CONFIG_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/5/callback/socket/redundancy/types.h"

/* ---------------------------------------------------------------------------------------------- */

extern void SCSRedundancyCallbackConfigInitialize(SCSRedundancyCallbackConfig * self);
extern void SCSRedundancyCallbackConfigFinalize(SCSRedundancyCallbackConfig * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSRedundancyCallbackConfigAdapt(SCSRedundancyCallbackConfig * self);

extern bool SCSRedundancyCallbackConfigValidate(SCSRedundancyCallbackConfig * self);

#define SCSRedundancyCallbackConfigCall(xxx_self, xxx_info) \
		(xxx_self)->func(xxx_self, xxx_info)

/* ---------------------------------------------------------------------------------------------- */

#define SCSRedundancyCallbackConfigCopy(xxx_self, xxx_target) \
		memcpy(xxx_target, xxx_self, sizeof(SCSRedundancyCallbackConfig))

/* ---------------------------------------------------------------------------------------------- */

#define SCSRedundancyCallbackConfigSet(xxx_self, xxx_times, xxx_func) \
		(xxx_self)->times = xxx_times;\
		(xxx_self)->func = xxx_func
#define SCSRedundancyCallbackConfigGetFunction(xxx_self) \
		((xxx_self)->func)

#define SCSRedundancyCallbackConfigSetTimes(xxx_self, xxx_value) \
		(xxx_self)->times = xxx_value
#define SCSRedundancyCallbackConfigGetTimes(xxx_self) \
		((xxx_self)->times)

/* ---------------------------------------------------------------------------------------------- */

#define SCSRedundancyCallbackConfigIsSet(xxx_self) \
		(0 < (xxx_self)->times)

/* ---------------------------------------------------------------------------------------------- */

#define SCSRedundancyCallbackConfigCanCall(xxx_self) \
		((xxx_self)->func != NULL ? true : false)

/* ---------------------------------------------------------------------------------------------- */

#define SCSRedundancyCallbackConfigCompareTimes(xxx_self, xxx_value, xxx_action) \
		((xxx_self)->times xxx_action xxx_value)

/* ---------------------------------------------------------------------------------------------- */

extern void SCSRedundancyCallbackConfigDump( 													//
		SCSRedundancyCallbackConfig * __restrict self, 											//
		__const char * __restrict prefix);

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SOCKET_REDUNDANCY_CONFIG_H_ */
