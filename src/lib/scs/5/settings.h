#ifndef SCS_5_SETTINGS_H_
#define SCS_5_SETTINGS_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/5/socket/socket.h"
#include "scs/5/socket/settings.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSSettings {
	struct {
		size_t entries;
	} callback;
	struct {
		struct {
			size_t send;
			size_t close;
		} size;
	} queue;
	SCSSocketSettings socket;
} SCSSettings;

extern void SCSSettingsInitialize(SCSSettings * self);
extern void SCSSettingsFinalize(SCSSettings * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSSettingsAdapt(SCSSettings * self);

extern bool SCSSettingsValidate(SCSSettings * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSSettingsDump(SCSSettings * __restrict self, __const char * __restrict prefix);

/* ---------------------------------------------------------------------------------------------- */

#define SCSSettingsCopy(xxx_dst, xxx_src) \
		memcpy(xxx_dst, xxx_src, sizeof(SCSSettings))

/* ============================================================================================== */

#endif /* SCS_5_SETTINGS_H_ */
