#define SCS_SOURCECODE_FILEID	"5CBKSYSSYSSTA"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/callback/system/system/status.h"
#include "scs/5/callback/system/system/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

void SCSSystemCallbackStatusInitialize(SCSSystemCallbackStatus * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	//TODO

	memset(self, 0, sizeof(SCSSystemCallbackStatus));

}
void SCSSystemCallbackStatusFinalize(SCSSystemCallbackStatus * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	//TODO

	memset(self, 0, sizeof(SCSSystemCallbackStatus));

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
