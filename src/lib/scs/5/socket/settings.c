#define SCS_SOURCECODE_FILEID	"5SOKSTG"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/socket/settings.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

void SCSSocketSettingsInitialize(SCSSocketSettings * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSSocketSettings));

	SCSDatagramSocketSettingsInitialize(&self->datagram);

}
void SCSSocketSettingsFinalize(SCSSocketSettings * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	SCSDatagramSocketSettingsFinalize(&self->datagram);

	memset(self, 0, sizeof(SCSSocketSettings));

}

/* ---------------------------------------------------------------------------------------------- */

void SCSSocketSettingsAdapt(SCSSocketSettings * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	SCSDatagramSocketSettingsAdapt(&self->datagram);

}

bool SCSSocketSettingsValidate(SCSSocketSettings * self) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	tmp_result = true;

	if (SCSDatagramSocketSettingsValidate(&self->datagram) == false) {
		tmp_result = false;
	}

	return tmp_result;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSSocketSettingsDump( //
		SCSSocketSettings * __restrict self, //
		__const char * __restrict prefix) {
	const char tmp_caption[] = "socket.";
	char tmp_prefix[UINT8_MAX];
	//char tmp_buffer[UINT8_MAX];

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	if (prefix == NULL) {
		prefix = "";
	}

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s", prefix, tmp_caption);
	SCSDatagramSocketSettingsDump(&self->datagram, tmp_prefix);

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
