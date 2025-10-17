#define SCS_SOURCECODE_FILEID	"5CBKSYSSYSCFG"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/callback/system/system/config.h"
#include "scs/5/callback/system/system/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

void SCSSystemCallbackConfigInitialize(SCSSystemCallbackConfig * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSSystemCallbackConfig));

	//self->func = NULL;
	//self->argument.ptr = NULL;
	//self->argument.size = 0;
	//self->interval = 0;

}
void SCSSystemCallbackConfigFinalize(SCSSystemCallbackConfig * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	//self->func = NULL;
	//self->argument.ptr = NULL;
	//self->argument.size = 0;
	//self->interval = 0;

	memset(self, 0, sizeof(SCSSystemCallbackConfig));

}

/* ---------------------------------------------------------------------------------------------- */

bool SCSSystemCallbackConfigValidate(SCSSystemCallbackConfig * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (self->func == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (self->interval < 1) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	return true;
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
