#define SCS_SOURCECODE_FILEID	"5STG"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/defaults.h"
#include "scs/5/defines.h"
#include "scs/5/settings.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

void SCSSettingsInitialize(SCSSettings * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSSettings));

	self->callback.entries = SCS_DEFAULT_CALLBACK_MAXENTRIES;
	self->queue.size.send = SCS_DEFAULT_SEND_QUEUESIZE;
	self->queue.size.close = SCS_DEFAULT_CLOSE_QUEUESIZE;
	SCSSocketSettingsInitialize(&self->socket);

}
void SCSSettingsFinalize(SCSSettings * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	//self->callback.entries = 0;
	//self->queue.size.send = 0;
	//self->queue.size.close = 0;
	SCSSocketSettingsFinalize(&self->socket);

	memset(self, 0, sizeof(SCSSettings));

}

/* ---------------------------------------------------------------------------------------------- */

void SCSSettingsAdapt(SCSSettings * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	if (self->callback.entries < 1) {
		self->callback.entries = SCS_DEFAULT_CALLBACK_MAXENTRIES;
	}

	if (self->queue.size.send < SCS_SKTQUEUE_MINSIZE) {
		self->queue.size.send = SCS_SKTQUEUE_MINSIZE;
	}

	if (self->queue.size.close < SCS_SKTQUEUE_MINSIZE) {
		self->queue.size.close = SCS_SKTQUEUE_MINSIZE;
	}

	SCSSocketSettingsAdapt(&self->socket);

}

bool SCSSettingsValidate(SCSSettings * self) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	tmp_result = true;

	if (self->callback.entries < 1) {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%d>>", self->callback.entries);
		return false;
	}

	if (self->queue.size.send < SCS_SKTQUEUE_MINSIZE) {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%d>>", self->queue.size.send);
		return false;
	}

	if (self->queue.size.close < SCS_SKTQUEUE_MINSIZE) {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%d>>", self->queue.size.close);
		return false;
	}

	if (SCSSocketSettingsValidate(&self->socket) == false) {
		tmp_result = false;
	}

	return tmp_result;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSSettingsDump(SCSSettings * __restrict self, __const char * __restrict prefix) {
	const char tmp_caption[] = "settings.";
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
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "callback.entries", "%zu",
			self->callback.entries);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s", prefix, tmp_caption);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "queue.size.send", "%zu",
			self->queue.size.send);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "queue.size.close", "%zu",
			self->queue.size.close);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s", prefix, tmp_caption);
	SCSSocketSettingsDump(&self->socket, tmp_prefix);

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
