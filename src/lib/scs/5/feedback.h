#ifndef SCS_5_FEEDBACK_H_
#define SCS_5_FEEDBACK_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/5/packet/types.h"
#include "scs/5/types.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSFeedbackInfo {
	uint64_t timestamp;
	SCSNetworkTrafficInfo traffic;
	struct {
		SCSPacketTimestamp minimum;
		SCSPacketTimestamp maximum;
		SCSPacketTimestamp current;
	} rtt;
	struct {
		uint32_t minimum;
		uint32_t maximum;
		uint32_t current;
	} plr;
	uint32_t mtu;
} SCSFeedbackInfo;

extern void SCSFeedbackInfoInitialize(SCSFeedbackInfo * self);
extern void SCSFeedbackInfoFinalize(SCSFeedbackInfo * self);

/* ---------------------------------------------------------------------------------------------- */

#define SCSFeedbackInfoGetTotalSentPackets(xxx_self) \
		((xxx_self)->traffic.sent.control.packets + \
		 (xxx_self)->traffic.sent.payload.packets)
#define SCSFeedbackInfoGetSentDataPackets(xxx_self) \
		 ((xxx_self)->traffic.sent.payload.packets)
#define SCSFeedbackInfoGetSentControlPackets(xxx_self) \
		 ((xxx_self)->traffic.sent.control.packets)
#define SCSFeedbackInfoGetRTT(xxx_self) \
		((xxx_self)->rtt.current)
#define SCSFeedbackInfoGetMTU(xxx_self) \
		((xxx_self)->mtu)

/* ---------------------------------------------------------------------------------------------- */

#define SCSFeedbackInfoCopy(xxx_self, xxx_target) \
		memcpy(xxx_target, xxx_self, sizeof(SCSFeedbackInfo))

/* ---------------------------------------------------------------------------------------------- */

extern void SCSFeedbackInfoLogging(SCSFeedbackInfo * self);

/* ============================================================================================== */

#endif /* SCS_5_FEEDBACK_H_ */
