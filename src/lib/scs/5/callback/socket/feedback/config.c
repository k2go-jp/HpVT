#define SCS_SOURCECODE_FILEID	"5CBKSKTFBKCFG"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/callback/socket/feedback/config.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

void SCSFeedbackCallbackConfigInitialize(SCSFeedbackCallbackConfig * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSFeedbackCallbackConfig));

	//self->func = NULL;
	//self->argument.ptr = NULL;
	//self->argument.size = 0;

}
void SCSFeedbackCallbackConfigFinalize(SCSFeedbackCallbackConfig * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	//self->func.send = NULL;
	//self->func.receive = NULL;
	//self->argument.ptr = NULL;
	//self->argument.size = 0;

	memset(self, 0, sizeof(SCSFeedbackCallbackConfig));

}

/* ---------------------------------------------------------------------------------------------- */

bool SCSFeedbackCallbackConfigValidate(SCSFeedbackCallbackConfig * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	//self->func
	//self->argument.ptr
	//self->argument.size

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSFeedbackCallbackConfigDump(SCSFeedbackCallbackConfig * __restrict self,
		__const char * __restrict prefix) {
	const char tmp_caption[] = "feedback.";
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

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
