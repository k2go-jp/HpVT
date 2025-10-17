#define SCS_SOURCECODE_FILEID	"5CBKSYSERRSTA"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/callback/system/error/status.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

void SCSErrorCallbackStatusInitialize(SCSErrorCallbackStatus * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	//TODO

	memset(self, 0, sizeof(SCSErrorCallbackStatus));

}
void SCSErrorCallbackStatusFinalize(SCSErrorCallbackStatus * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	//TODO

	memset(self, 0, sizeof(SCSErrorCallbackStatus));

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
