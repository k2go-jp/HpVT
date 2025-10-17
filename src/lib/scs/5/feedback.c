#define SCS_SOURCECODE_FILEID	"5PKTFBK"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/inttypes.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/2/pointer.h"
#include "scs/5/feedback.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

inline void SCSFeedbackInfoInitialize(SCSFeedbackInfo * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSFeedbackInfo));

	//self->sent.control.packets = 0;
	//self->sent.control.bytes = 0;
	//self->sent.payload.packets = 0;
	//self->sent.payload.bytes = 0;
	//self->sent.redundancy.packets = 0;
	//self->sent.redundancy.bytes = 0;
	//self->sent.dropped.packets = 0;
	//self->sent.dropped.bytes = 0;

	//self->received.control.packets = 0;
	//self->received.control.bytes = 0;
	//self->received.payload.packets = 0;
	//self->received.payload.bytes = 0;
	//self->received.redundancy.packets = 0;
	//self->received.redundancy.bytes = 0;
	//self->received.dropped.total.packets = 0;
	//self->received.dropped.total.bytes = 0;
	//self->received.dropped.duplicate.packets = 0;
	//self->received.dropped.duplicate.bytes = 0;
	//self->received.dropped.outofrange.packets = 0;
	//self->received.dropped.outofrange.bytes = 0;
	//self->received.dropped.overrun.packets = 0;
	//self->received.dropped.overrun.bytes = 0;
	//self->received.dropped.invalid.packets = 0;
	//self->received.dropped.invalid.bytes = 0;
	//self->received.dropped.broken.packets = 0;
	//self->received.dropped.broken.bytes = 0;

	//self->rtt.minimum = 0;
	//self->rtt.maximum = 0;
	//self->rtt.current = 0;

	//self->plr.minimum = 0;
	//self->plr.maximum = 0;
	//self->plr.current = 0;

	//self->mtu = 0;

	//self->timestamp = 0;

}
inline void SCSFeedbackInfoFinalize(SCSFeedbackInfo * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	//self->sent.control.packets = 0;
	//self->sent.control.bytes = 0;
	//self->sent.payload.packets = 0;
	//self->sent.payload.bytes = 0;
	//self->sent.redundancy.packets = 0;
	//self->sent.redundancy.bytes = 0;
	//self->sent.dropped.packets = 0;
	//self->sent.dropped.bytes = 0;

	//self->received.control.packets = 0;
	//self->received.control.bytes = 0;
	//self->received.payload.packets = 0;
	//self->received.payload.bytes = 0;
	//self->received.redundancy.packets = 0;
	//self->received.redundancy.bytes = 0;
	//self->received.dropped.total.packets = 0;
	//self->received.dropped.total.bytes = 0;
	//self->received.dropped.duplicate.packets = 0;
	//self->received.dropped.duplicate.bytes = 0;
	//self->received.dropped.outofrange.packets = 0;
	//self->received.dropped.outofrange.bytes = 0;
	//self->received.dropped.overrun.packets = 0;
	//self->received.dropped.overrun.bytes = 0;
	//self->received.dropped.invalid.packets = 0;
	//self->received.dropped.invalid.bytes = 0;
	//self->received.dropped.broken.packets = 0;
	//self->received.dropped.broken.bytes = 0;

	//self->rtt.minimum = 0;
	//self->rtt.maximum = 0;
	//self->rtt.current = 0;

	//self->plr.minimum = 0;
	//self->plr.maximum = 0;
	//self->plr.current = 0;

	//self->mtu = 0;

	//self->timestamp = 0;

	memset(self, 0, sizeof(SCSFeedbackInfo));

}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSFeedbackInfoLogging(SCSFeedbackInfo * self) {
	char tmp_message[1024];
	SCSPointer tmp_ptr;
	int tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	SCSPointerInitialize(tmp_ptr, tmp_message, sizeof(tmp_message), 0);
	SCSPointerPrint(tmp_ptr, tmp_result, "%20"PRIu64" SEND:", self->timestamp);
	SCSPointerPrint(tmp_ptr, tmp_result, "CTL=%"PRIu64"(%"PRIu64"),", //
			self->traffic.sent.control.packets,//
			self->traffic.sent.control.bytes);
	SCSPointerPrint(tmp_ptr, tmp_result, "PLD=%"PRIu64"(%"PRIu64"),", //
			self->traffic.sent.payload.packets,//
			self->traffic.sent.payload.bytes);
	SCSPointerPrint(tmp_ptr, tmp_result, "RED=%"PRIu64"(%"PRIu64"),", //
			self->traffic.sent.redundancy.packets,//
			self->traffic.sent.redundancy.bytes);
	SCSPointerPrint(tmp_ptr, tmp_result, "DRP=%"PRIu64"(%"PRIu64"),", //
			self->traffic.sent.dropped.packets,//
			self->traffic.sent.dropped.bytes);
	SCS_LOG(NOTICE, SOCKET, 00000, "%s", tmp_message);

	SCSPointerInitialize(tmp_ptr, tmp_message, sizeof(tmp_message), 0);
	SCSPointerPrint(tmp_ptr, tmp_result, "%20"PRIu64" RECV:", self->timestamp);
	SCSPointerPrint(tmp_ptr, tmp_result, "CTL=%"PRIu64"(%"PRIu64"),", //
			self->traffic.received.control.packets,//
			self->traffic.received.control.bytes);
	SCSPointerPrint(tmp_ptr, tmp_result, "PLD=%"PRIu64"(%"PRIu64"),", //
			self->traffic.received.payload.packets,//
			self->traffic.received.payload.bytes);
	SCSPointerPrint(tmp_ptr, tmp_result, "RED=%"PRIu64"(%"PRIu64"),", //
			self->traffic.received.redundancy.packets,//
			self->traffic.received.redundancy.bytes);
	SCSPointerPrint(tmp_ptr, tmp_result, "DRP=%"PRIu64"(%"PRIu64"),", //
			self->traffic.received.dropped.total.packets,//
			self->traffic.received.dropped.total.bytes);
	SCSPointerPrint(tmp_ptr, tmp_result, "DUP=%"PRIu64"(%"PRIu64"),", //
			self->traffic.received.dropped.duplicate.packets,//
			self->traffic.received.dropped.duplicate.bytes);
	SCSPointerPrint(tmp_ptr, tmp_result, "OOR=%"PRIu64"(%"PRIu64"),", //
			self->traffic.received.dropped.outofrange.packets,//
			self->traffic.received.dropped.outofrange.bytes);
	SCSPointerPrint(tmp_ptr, tmp_result, "OVR=%"PRIu64"(%"PRIu64"),", //
			self->traffic.received.dropped.overrun.packets,//
			self->traffic.received.dropped.overrun.bytes);
	SCSPointerPrint(tmp_ptr, tmp_result, "INV=%"PRIu64"(%"PRIu64"),", //
			self->traffic.received.dropped.invalid.packets,//
			self->traffic.received.dropped.invalid.bytes);
	SCSPointerPrint(tmp_ptr, tmp_result, "BRK=%"PRIu64"(%"PRIu64"),", //
			self->traffic.received.dropped.broken.packets,//
			self->traffic.received.dropped.broken.bytes);
	SCS_LOG(NOTICE, SOCKET, 00000, "%s", tmp_message);

	SCSPointerInitialize(tmp_ptr, tmp_message, sizeof(tmp_message), 0);
	SCSPointerPrint(tmp_ptr, tmp_result, "%20"PRIu64" RTT:", self->timestamp);
	SCSPointerPrint(tmp_ptr, tmp_result, "MIN=%"PRIu32",", self->rtt.minimum);
	SCSPointerPrint(tmp_ptr, tmp_result, "MAX=%"PRIu32",", self->rtt.maximum);
	SCSPointerPrint(tmp_ptr, tmp_result, "LAST=%"PRIu32",", self->rtt.current);
	SCS_LOG(NOTICE, SOCKET, 00000, "%s", tmp_message);

	SCSPointerInitialize(tmp_ptr, tmp_message, sizeof(tmp_message), 0);
	SCSPointerPrint(tmp_ptr, tmp_result, "%20"PRIu64" PLR:", self->timestamp);
	SCSPointerPrint(tmp_ptr, tmp_result, "MIN=%"PRIu32",", self->plr.minimum);
	SCSPointerPrint(tmp_ptr, tmp_result, "MAX=%"PRIu32",", self->plr.maximum);
	SCSPointerPrint(tmp_ptr, tmp_result, "LAST=%"PRIu32",", self->plr.current);
	SCS_LOG(NOTICE, SOCKET, 00000, "%s", tmp_message);

	SCSPointerInitialize(tmp_ptr, tmp_message, sizeof(tmp_message), 0);
	SCS_LOG(NOTICE, SOCKET, 00000, "%20"PRIu64" MTU: %"PRIu32",", self->timestamp, self->mtu);

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
