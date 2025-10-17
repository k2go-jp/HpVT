#ifndef SCS_5_SOCKET_SETTINGS_H_
#define SCS_5_SOCKET_SETTINGS_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/5/socket/datagram/settings.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSSocketSettings {
	SCSDatagramSocketSettings datagram;
} SCSSocketSettings;

extern void SCSSocketSettingsInitialize(SCSSocketSettings * self);
extern void SCSSocketSettingsFinalize(SCSSocketSettings * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSSocketSettingsAdapt(SCSSocketSettings * self);

extern bool SCSSocketSettingsValidate(SCSSocketSettings * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSSocketSettingsDump( //
		SCSSocketSettings * __restrict self, //
		__const char * __restrict prefix);

/* ---------------------------------------------------------------------------------------------- */

#define SCSSocketSettingsCopy(xxx_dst, xxx_src) \
		memcpy(xxx_dst, xxx_src, sizeof(SCSSocketSettings))

/* ============================================================================================== */

#endif /* SCS_5_SOCKET_SETTINGS_H_ */
