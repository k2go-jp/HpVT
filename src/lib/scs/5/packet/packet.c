#define SCS_SOURCECODE_FILEID	"5PACKET"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/1/unistd.h"
#include "scs/2/logger.h"
#include "scs/3/counter.h"
#include "scs/4/gc.h"
#include "scs/5/callback/socket/feedback/config.h"
#include "scs/5/callback/socket/redundancy/config.h"
#include "scs/5/callback/socket/redundancy/result.h"
#include "scs/5/callback/socket/redundancy/status.h"
#include "scs/5/channel/types.h"
#include "scs/5/packet/packet.h"
#include "scs/5/packet/types.h"
#include "scs/5/socket/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_LOCK
#error
#endif
#ifdef _SCS_UNLOCK
#error
#endif

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_LOCK(xxx_self) \
	if (SCSMutexLock((xxx_self)->mutex) != 0) {\
		SCS_LOG(ALERT, SYSTEM, 99999, "");\
		abort();\
	}
#define _SCS_UNLOCK(xxx_self) \
	if (SCSMutexUnlock((xxx_self)->mutex) != 0) {\
		SCS_LOG(ALERT, SYSTEM, 99999, "");\
		abort();\
	}

/* ---------------------------------------------------------------------------------------------- */

static SCSObjectCounter _counter = SCSObjectCounterInitializer;

/* ---------------------------------------------------------------------------------------------- */

inline void SCSPacketInitialize(SCSPacket * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSPacket));

	SCSAtomicReferenceInitialize(self->reference);

	SCSMutexInitialize(self->mutex);

	self->sockid = SCS_SKTID_INVVAL;
	self->connid = SCS_CONNID_INVVAL;
	self->option = SCS_SKTOPTN_NONE;
	self->flags = SCS_PKTFLAG_NONE;
	self->seqno = SCS_PKTSEQNO_MINVAL;

	//self->payload.ptr = NULL;
	//self->payload.length = 0;

	SCSRedundancyCallbackConfigInitialize(&self->redundancy.config);
	//self->redundancy.times = 0;

	SCSTimespecInitialize(self->interval);

	self->mode = SCS_PKTMODE_NONE;

	SCSTimespecInitialize(self->timestamp.self);
	//self->timestamp.peer = 0

	//self->notification.info = NULL;

	self->verification.method = SCS_PKTVRFMETHOD_INVVAL;
	//self->verification.code;

	self->rttmeas.flag = SCS_PKTRTTMFLAG_INVVAL;
	self->rttmeas.id = SCS_PKTRTTMID_INVVAL;

	//self->pad.offset = 0;
	//self->pad.length = 0;

	//self->connected = false;
	//self->canceled = false;

}
inline void SCSPacketFinalize(SCSPacket * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	SCSAtomicReferenceFinalize(self->reference);

	SCSMutexFinalize(self->mutex);

	self->sockid = SCS_SKTID_INVVAL;
	self->connid = SCS_CONNID_INVVAL;
	self->option = SCS_SKTOPTN_NONE;
	self->flags = SCS_PKTFLAG_NONE;
	self->seqno = SCS_PKTSEQNO_MINVAL;

	if (self->payload.ptr != NULL) {
		free(self->payload.ptr);
		self->payload.ptr = NULL;
	}
	//self->payload.length = 0;

	SCSRedundancyCallbackConfigFinalize(&self->redundancy.config);
	//self->redundancy.times = 0;

	SCSTimespecFinalize(self->interval);

	self->mode = SCS_PKTMODE_NONE;

	SCSTimespecFinalize(self->timestamp.self);
	//self->timestamp.peer = 0

	if (self->notification.info != NULL) {
		free(self->notification.info);
		self->notification.info = NULL;
	}

	self->verification.method = SCS_PKTVRFMETHOD_INVVAL;
	//self->verification.code;

	self->rttmeas.flag = SCS_PKTRTTMFLAG_INVVAL;
	self->rttmeas.id = SCS_PKTRTTMID_INVVAL;

	//self->pad.offset = 0;
	//self->pad.length = 0;

	//self->connected = false;
	//self->canceled = false;

	memset(self, 0, sizeof(SCSPacket));
}

/* ---------------------------------------------------------------------------------------------- */

static bool _SCSPacketCanDestroy(SCSPacket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	if (SCSAtomicReferenceCanDestroy(self->reference) == false) {
		return false;
	}

	return true;
}

SCSPacket * SCSPacketCreate(void) {
	SCSPacket * tmp_self;

	if ((tmp_self = (SCSPacket *) malloc(sizeof(SCSPacket))) == NULL) {
		SCS_LOG(ALERT, MEMORY, 00002, "<<%zu>>", sizeof(SCSPacket));
		return NULL;
	}

	SCSPacketInitialize(tmp_self);

	SCSObjectCounterIncreaseCreate(_counter);

	return tmp_self;
}
static bool _SCSPacketDestroy(void * self) {
	SCSPacket * tmp_self;

	if ((tmp_self = (SCSPacket *) self) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99998, "");
		return true;
	}

	if (_SCSPacketCanDestroy(tmp_self) == false) {
		return false;
	}

	SCSObjectCounterIncreaseDestroyed(_counter);

	SCSPacketFinalize(tmp_self);
	free(tmp_self);

	return true;
}
void SCSPacketDestroy(SCSPacket * self) {
	SCSGarbage * tmp_garbage;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	bool tmp_destroyed;

	_SCS_LOCK(self);

	tmp_destroyed = SCSAtomicReferenceIsDestroy(self->reference);

	if (tmp_destroyed == false) {
		SCSAtomicReferenceForbid(self->reference);
	}

	_SCS_UNLOCK(self);

	if (tmp_destroyed == true) {
		// Already destroyed.
		return;
	}

	SCSObjectCounterIncreaseDestroy(_counter);

	SCSAtomicReferenceForbid(self->reference);

	if ((tmp_garbage = SCSGarbageCreate(self, _SCSPacketDestroy)) == NULL) {
		SCS_LOG(ALERT, MEMORY, 00002, "<<%zu>>", sizeof(SCSPacket));
		return;
	}

	SCSGarbageCollectorPush(&_scs_global_gc, tmp_garbage);

}

inline bool SCSPacketHold(SCSPacket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	return SCSAtomicReferenceIncrease((self)->reference);
}
inline void SCSPacketFree(SCSPacket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSAtomicReferenceDecrease((self)->reference);

}

/* ---------------------------------------------------------------------------------------------- */

bool SCSPacketBecomeRedundancy(SCSPacket * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (!SCSRedundancyCallbackConfigIsSet(&self->redundancy.config)) {
		return false;
	}

	if (SCSRedundancyCallbackConfigCompareTimes( //
			&self->redundancy.config, self->redundancy.times, >)) {
		self->redundancy.times++;
	}
	else {
		return false;
	}

	SCSTimespecSetCurrentTime(self->timestamp.self, CLOCK_MONOTONIC);

	if (SCSRedundancyCallbackConfigCanCall(&self->redundancy.config)) {
		SCSRedundancyCallbackStatus tmp_status;
		SCSRedundancyCallbackResult tmp_result;

		SCSRedundancyCallbackStatusInitialize(&tmp_status);
		tmp_status.interval = self->interval;
		tmp_status.times = self->redundancy.times;

		tmp_result = SCSRedundancyCallbackConfigCall(&self->redundancy.config, &tmp_status);

		if (SCSRedundancyCallbackResultValidate(&tmp_result)) {
			SCSTimespecCopy(self->interval, tmp_result.interval);
			self->state.canceled = tmp_result.cancel;
		}

		SCSRedundancyCallbackConfigAdapt(&self->redundancy.config);
	}
	else {
		SCSTimespecSetZero(self->interval);
	}

	self->flags |= SCS_PKTFLAG_RED;

	return true;
}

bool SCSPacketVerify(SCSPacket * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (SCSPacketCheckFlags(self, SCS_PKTFLAG_VRF)) {
		switch (self->verification.method) {
			case SCS_PKTVRFMETHOD_CRC16: {
				return (self->verification.code.crc16.value1
						== self->verification.code.crc16.value2 ? true : false);
			}
			default: {
				return false;
			}
		}
	}

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

bool SCSPacketCanSend(SCSPacket * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (self->state.canceled == true) {
		return false;
	}

	if (SCSTimespecIsSet(self->interval)) {
		scs_timespec tmp_timestamp;
		scs_timespec tmp_interval;

		SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);
		SCSTimespecSub(tmp_timestamp, self->timestamp.self, tmp_interval);

		if (SCSTimespecCompare(tmp_interval, self->interval, <)) {
			return false;
		}
	}

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

bool SCSPacketSetRedundancyCallback(SCSPacket * __restrict self,
		SCSRedundancyCallbackConfig * __restrict config) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (config == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	SCSRedundancyCallbackConfigCopy(config, &self->redundancy.config);
	self->redundancy.times = 0;

	return true;
}

bool SCSPacketSetRTTMeas(SCSPacket * self, SCSRTTMeasFlag flag, SCSRTTMeasId id) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (!SCSRTTMeasFlagValidate(flag)) {
		SCS_LOG(WARN, SYSTEM, 99998, "<<%zu>>", flag);
		return false;
	}

	if (!SCSRTTMeasIdValidate(id)) {
		SCS_LOG(WARN, SYSTEM, 99998, "<<%zu>>", id);
		return false;
	}

	SCSPacketSetFlags(self, SCS_PKTFLAG_RTT);
	self->rttmeas.flag = flag;
	self->rttmeas.id = id;

	return true;
}

bool SCSPacketSetVerification(SCSPacket * self, SCSPacketVerificationMethod method) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (!SCSPacketVerificationMethodValidate(method)) {
		return false;
	}

	SCSPacketSetFlags(self, SCS_PKTFLAG_VRF);
	(self)->verification.method = (SCS_PKTVRFMETHOD_FIXVAL | method);

	return true;
}

bool SCSPacketSetPad(SCSPacket * self, size_t offset, size_t length) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (length < 1) {
		SCS_LOG(WARN, SYSTEM, 99997, "<<%zu>>", length);
		return false;
	}

	SCSPacketSetFlags(self, SCS_PKTFLAG_PAD);
	self->pad.offset = offset;
	self->pad.length = length;

	return true;
}

bool SCSPacketSetPayload(SCSPacket * __restrict self, void * __restrict ptr, size_t length) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (ptr == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (length < 1) {
		SCS_LOG(WARN, SYSTEM, 99997, "<<%zu>>", length);
		return false;
	}

	SCSPacketSetFlags(self, SCS_PKTFLAG_PLD);
	self->payload.ptr = ptr;
	self->payload.length = length;

	return true;
}

bool SCSPacketSetFeedbackInfo(SCSPacket * __restrict self, SCSFeedbackInfo * __restrict info) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (info != NULL) {
		SCSPacketSetFlags(self, SCS_PKTFLAG_NTY);
		self->notification.info = info;
	}
	else {
		SCSPacketUnsetFlags(self, SCS_PKTFLAG_NTY);
		free(self->notification.info);
		self->notification.info = NULL;
	}

	return true;
}

bool SCSPacketSetFeedbackCallback(SCSPacket * __restrict self,
		SCSFeedbackCallbackConfig * __restrict config) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (config == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	SCSPacketSetFlags(self, (SCS_PKTFLAG_NTY | SCS_PKTFLAG_PLD));
	SCSFeedbackCallbackConfigCopy(config, &self->notification.config);

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSPacketMonitor(void) {
	SCSObjectCounterMonitor("Packet");
}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
