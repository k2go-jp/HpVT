#define SCS_SOURCECODE_FILEID	"5CBKSKTREDSAT"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/defines.h"
#include "scs/5/callback/socket/redundancy/status.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

inline void SCSRedundancyCallbackStatusInitialize(SCSRedundancyCallbackStatus * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSRedundancyCallbackStatus));

	//self->interval = 0;
	//self->times = 0;

}
inline void SCSRedundancyCallbackStatusFinalize(SCSRedundancyCallbackStatus * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	//self->interval = 0;
	//self->times = 0;

	memset(self, 0, sizeof(SCSRedundancyCallbackStatus));

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
