#define SCS_SOURCECODE_FILEID	"5CBKSYSSCKCFG"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/callback/system/socket/config.h"
#include "scs/5/callback/system/socket/types.h"
#include "scs/5/socket/types.h"
#include "scs/5/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

void SCSSocketCallbackConfigInitialize(SCSSocketCallbackConfig * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSSocketCallbackConfig));

	//self->func = NULL;
	//self->argument.ptr = NULL;
	//self->argument.size = 0;
	//self->interval = 0;
	self->id = SCS_SKTID_INVVAL;

}
void SCSSocketCallbackConfigFinalize(SCSSocketCallbackConfig * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSSocketCallbackConfig));

	//self->func = NULL;
	//self->argument.ptr = NULL;
	//self->argument.size = 0;
	//self->interval = 0;
	self->id = SCS_SKTID_INVVAL;

}

/* ---------------------------------------------------------------------------------------------- */

bool SCSSocketCallbackConfigValidate(SCSSocketCallbackConfig * self) {

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

	if (!SCSSocketIdValidate(self->id)) {
		SCS_LOG(WARN, SYSTEM, 99997, "<<%zu>>", self->id);
		return false;
	}

	return true;
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
