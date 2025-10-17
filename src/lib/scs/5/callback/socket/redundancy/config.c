#define SCS_SOURCECODE_FILEID	"5CBKSKTREDCFG"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/defines.h"
#include "scs/5/callback/socket/redundancy/config.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

inline void SCSRedundancyCallbackConfigInitialize(SCSRedundancyCallbackConfig * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSRedundancyCallbackConfig));

	//self->func = NULL;
	//self->argument.prt = NULL;
	//self->argument.size = 0;
	self->times = -1;

}
inline void SCSRedundancyCallbackConfigFinalize(SCSRedundancyCallbackConfig * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	//self->func = NULL;
	//self->argument.ptr = NULL;
	//self->argument.size = 0;
	//self->times = 0;

	memset(self, 0, sizeof(SCSRedundancyCallbackConfig));

}

/* ---------------------------------------------------------------------------------------------- */

void SCSRedundancyCallbackConfigAdapt(SCSRedundancyCallbackConfig * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	//self->func;

	if (self->argument.ptr == NULL) {
		self->argument.size = 0;
	}
	else {
		if (self->argument.size == 0) {
			self->argument.ptr = NULL;
		}
	}

	if (self->times < 0) {
		self->times = 0;
	}

	if (SCS_REDUNDANCY_MAXTIMES < self->times) {
		SCS_LOG(WARN, SYSTEM, 79995, "<<%d,%d>>", SCS_REDUNDANCY_MAXTIMES, self->times);
		self->times = SCS_REDUNDANCY_MAXTIMES;
	}

}

bool SCSRedundancyCallbackConfigValidate(SCSRedundancyCallbackConfig * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	//self->func;
	//self->argument.ptr;
	//self->argument.size;

	if (self->times < 0) {
		return false;
	}

	if (SCS_REFERENCE_MAXVAL < self->times) {
		return false;
	}

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSRedundancyCallbackConfigDump(SCSRedundancyCallbackConfig * __restrict self,
		__const char * __restrict prefix) {
	const char tmp_caption[] = "redundancy.";
	char tmp_prefix[UINT8_MAX];
	char tmp_buffer[UINT8_MAX];

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	if (prefix == NULL) {
		prefix = "";
	}

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s", prefix, tmp_caption);

	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "callback", "%s",
			(self->func ? "Set" : "(None)"));

	if (self->argument.ptr != NULL) {
		SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "argument", "Set(%zu)",
				self->argument.size);
	}
	else {
		SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "argument", "(None)");
	}

	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "times", "%d", self->times);

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
