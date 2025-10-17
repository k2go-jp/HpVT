#ifndef SCS_5_SOCKET_DATAGRAM_SETTINGS_H_
#define SCS_5_SOCKET_DATAGRAM_SETTINGS_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/5/callback/socket/feedback/types.h"
#include "scs/5/callback/socket/redundancy/types.h"
#include "scs/5/packet/types.h"
#include "scs/5/packet/watcher.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSDatagramSocketSettings {
	uint64_t bandwidth;
	struct {
		scs_time timeout;
		SCSRedundancyCallbackConfig redundancy;
	} connect;
	struct {
		scs_time timeout;
		SCSRedundancyCallbackConfig redundancy;
		struct {
			size_t length;
		} queue;
	} send;
	struct {
		scs_time timeout;
		struct {
			size_t length;
		} queue;
	} receive;
	struct {
		SCSFeedbackCallbackConfig callback;
		scs_timespec timeout;
		scs_timespec interval;
	} notification;
	struct {
		bool enabled;
		SCSPacketVerificationMethod method;
	} verification;
	struct {
		bool enabled;
		struct {
			scs_time seconds;
			uint64_t packets;
			uint64_t bytes;
		} interval;
	} pmtudisc;
	struct {
		bool enabled;
		SCSPacketWatcherSettings watcher;
	} rttmeas;
	struct {
		bool enabled;
		size_t samples;
	} plrmeas;
	struct {
		bool enabled;
		size_t samples;
	} jittermeas;
	struct {
		bool enabled;
		size_t size;
	} pad;
	struct {
		scs_time delaytime;
		SCSRedundancyCallbackConfig redundancy;
	} close;
} SCSDatagramSocketSettings;

extern void SCSDatagramSocketSettingsInitialize(SCSDatagramSocketSettings * self);
extern void SCSDatagramSocketSettingsFinalize(SCSDatagramSocketSettings * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSDatagramSocketSettingsAdapt(SCSDatagramSocketSettings * self);

extern bool SCSDatagramSocketSettingsValidate(SCSDatagramSocketSettings * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSDatagramSocketSettingsDump(														//
		SCSDatagramSocketSettings * __restrict self, 											//
		__const char * __restrict prefix);

/* ---------------------------------------------------------------------------------------------- */

#define SCSDatagramSocketSettingsCopy(xxx_dst, xxx_src) \
		memcpy(xxx_dst, xxx_src, sizeof(SCSDatagramSocketSettings))

/* ============================================================================================== */

#endif /* SCS_5_SOCKET_DATAGRAM_SETTINGS_H_ */
