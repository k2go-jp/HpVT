#ifndef SCS_5_CALLBACK_SOCKET_FEEDBACK_CONFIG_H_
#define SCS_5_CALLBACK_SOCKET_FEEDBACK_CONFIG_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/5/callback/socket/feedback/types.h"

/* ---------------------------------------------------------------------------------------------- */

extern void SCSFeedbackCallbackConfigInitialize(SCSFeedbackCallbackConfig * self);
extern void SCSFeedbackCallbackConfigFinalize(SCSFeedbackCallbackConfig * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSFeedbackCallbackConfigValidate(SCSFeedbackCallbackConfig * self);

#define SCSFeedbackCallbackConfigCall(xxx_self, xxx_ptr, xxx_size) \
		(xxx_self)->func(xxx_self, xxx_ptr, xxx_size)

/* ---------------------------------------------------------------------------------------------- */

#define SCSFeedbackCallbackConfigCanCall(xxx_self) \
		((xxx_self)->func != NULL ? true : false)

/* ---------------------------------------------------------------------------------------------- */

#define SCSFeedbackCallbackConfigCopy(xxx_self, xxx_target) \
		memcpy(xxx_target, xxx_self, sizeof(SCSFeedbackCallbackConfig))

/* ---------------------------------------------------------------------------------------------- */

extern void SCSFeedbackCallbackConfigDump(														//
		SCSFeedbackCallbackConfig * __restrict self,											//
		__const char * __restrict prefix);

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_SOCKET_FEEDBACK_CONFIG_H_ */
