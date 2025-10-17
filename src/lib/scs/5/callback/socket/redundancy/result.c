#define SCS_SOURCECODE_FILEID	"5CBKSKTREDRLT"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/defines.h"
#include "scs/5/callback/socket/redundancy/result.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

inline void SCSRedundancyCallbackResultInitialize(SCSRedundancyCallbackResult * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSRedundancyCallbackResult));

	//SCSTimespecInitialize(self->interval);
	//self->cancel = false;

}
inline void SCSRedundancyCallbackResultFinalize(SCSRedundancyCallbackResult * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	//SCSTimespecInitialize(self->interval);
	//self->cancel = false;

	memset(self, 0, sizeof(SCSRedundancyCallbackResult));

}

/* ---------------------------------------------------------------------------------------------- */

bool SCSRedundancyCallbackResultValidate(SCSRedundancyCallbackResult * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (SCS_REDUNDANCY_MAXINTERVAL < SCSTimespecGetSec(self->interval)) {
		SCSTimespecSet(self->interval, SCS_REDUNDANCY_MAXINTERVAL, 0);
	}

	return true;
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
