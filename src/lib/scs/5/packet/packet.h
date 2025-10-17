#ifndef SCS_5_PACKET_PACKET_H_
#define SCS_5_PACKET_PACKET_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/2/atomic.h"
#include "scs/5/channel/types.h"
#include "scs/5/callback/socket/feedback/types.h"
#include "scs/5/callback/socket/redundancy/types.h"
#include "scs/5/feedback.h"
#include "scs/5/packet/types.h"
#include "scs/5/types.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPacket {
	scs_atomic_reference reference;

	scs_mutex mutex;

	SCSSocketId sockid;
	SCSConnectionId connid;
	SCSSocketOption option;
	SCSPacketFlag flags;
	SCSPacketSeqno seqno;
	struct {
		SCSRedundancyCallbackConfig config;
		int times;
	} redundancy;
	scs_timespec interval;

	struct {
		void * ptr;
		size_t length;
	} payload;

	SCSPacketMode mode;
	struct {
		scs_timespec self;
		SCSPacketTimestamp peer;
	} timestamp;
	struct {
		SCSFeedbackInfo * info;
		SCSFeedbackCallbackConfig config;
	} notification;
	struct {
		SCSPacketVerificationMethod method;
		union {
			struct {
				uint16_t value1;
				uint16_t value2;
			} crc16;
		} code;
	} verification;
	struct {
		SCSRTTMeasFlag flag;
		SCSRTTMeasId id;
	} rttmeas;
	struct {
		size_t offset;
		size_t length;
	} pad;

	struct {
		bool canceled;bool connected;
	} state;
} SCSPacket;

/* ---------------------------------------------------------------------------------------------- */

extern void SCSPacketInitialize(SCSPacket * self);
extern void SCSPacketFinalize(SCSPacket * self);

/* ---------------------------------------------------------------------------------------------- */

extern SCSPacket * SCSPacketCreate(void);
extern void SCSPacketDestroy(SCSPacket * self);

extern bool SCSPacketHold(SCSPacket * self);
extern void SCSPacketFree(SCSPacket * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSPacketBecomeRedundancy(SCSPacket * self);

extern bool SCSPacketVerify(SCSPacket * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSPacketCanSend(SCSPacket * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSPacketSetRedundancyCallback(														//
		SCSPacket * __restrict self,															//
		SCSRedundancyCallbackConfig * __restrict config);

extern bool SCSPacketSetRTTMeas(																//
		SCSPacket * self, 																		//
		SCSRTTMeasFlag flag,																	//
		SCSRTTMeasId id);

extern bool SCSPacketSetVerification(SCSPacket * self, SCSPacketVerificationMethod method);

/* ---------------------------------------------------------------------------------------------- */

#define SCSPacketSetFlags(xxx_self, xxx_value) \
		((xxx_self)->flags |= xxx_value)

#define SCSPacketUnsetFlags(xxx_self, xxx_value) \
		((xxx_self)->flags &= ~xxx_value)

/* ---------------------------------------------------------------------------------------------- */

#define SCSPacketSetCancel(xxx_self) \
		((xxx_self)->state.canceled = true)

#define SCSPacketGetSocketId(xxx_self) \
		((xxx_self)->sockid)

#define SCSPacketGetPacketType(xxx_self) \
		((xxx_self)->type)

#define SCSPacketGetPacketVersion(xxx_self) \
		((xxx_self)->mode & SCS_PKTMODE_MASK1)

#define SCSPacketGetPacketMode(xxx_self) \
		((xxx_self)->mode & SCS_PKTMODE_MASK2)

#define SCSPacketGetSocketType(xxx_self) \
		((xxx_self)->mode & SCS_PKTMODE_MASK3)

#define SCSPacketGetMode(xxx_self, xxx_mask) \
		((xxx_self)->mode & xxx_mask)

#define SCSPacketGetFlags(xxx_self) \
		((xxx_self)->flags)

#define SCSPacketGetConnectionId(xxx_self) \
		((xxx_self)->connid)

#define SCSPacketGetTimestamp(xxx_self) \
		((xxx_self)->timestamp.self)

#define SCSPacketGetSeqno(xxx_self) \
		((xxx_self)->seqno)

extern bool SCSPacketSetPayload(SCSPacket * __restrict self, void * __restrict ptr, size_t length);
#define SCSPacketGetPayloadPointer(xxx_self) \
		((xxx_self)->payload.ptr)
#define SCSPacketGetPayloadSize(xxx_self) \
		((xxx_self)->payload.length)

#define SCSPacketGetVerificationMethod(xxx_self) \
		((xxx_self)->verification.method)

#define SCSPacketGetRTTMeasurementId(xxx_self) \
		((xxx_self)->rttmeas.id)

extern bool SCSPacketSetFeedbackInfo(															//
		SCSPacket * __restrict self,															//
		SCSFeedbackInfo * __restrict info);
#define SCSPacketGetFeedbackInfo(xxx_self) \
		((xxx_self)->notification.info)

extern bool SCSPacketSetFeedbackCallback(														//
		SCSPacket * __restrict self,															//
		SCSFeedbackCallbackConfig * __restrict config);

extern bool SCSPacketSetPad(SCSPacket * self, size_t offset, size_t length);
#define SCSPacketGetPadLength(xxx_self) \
		((xxx_self)->pad.length)

/* ---------------------------------------------------------------------------------------------- */

#define SCSPacketCheckMode(xxx_self, xxx_value, xxx_mask) \
		(((xxx_self)->mode & xxx_mask) == xxx_value ? true : false)

#define SCSPacketCheckFlags(xxx_self, xxx_value) \
		(((xxx_self)->flags & xxx_value) == xxx_value ? true : false)

#define SCSPacketCheckConnectionId(xxx_self, xxx_value) \
		((xxx_self)->connid == xxx_value ? true : false)

#define SCSPacketCheckTimestamp(xxx_self, xxx_value) \
		((xxx_self)->timestamp == xxx_value ? true : false)

#define SCSPacketCheckSequence(xxx_self, xxx_value) \
		((xxx_self)->seqno == xxx_value ? true : false)

#define SCSPacketCheckOption(xxx_self, xxx_value) \
		((xxx_self)->option | xxx_value)

#define SCSPacketCheckRTTMeasurementFlag(xxx_self, xxx_value, xxx_mask) \
		(((xxx_self)->rttmeas.flag & xxx_mask) == xxx_value)

/* ---------------------------------------------------------------------------------------------- */

#define SCSPacketIsSyn(xxx_self) \
		(((xxx_self)->flags & (SCS_PKTFLAG_SYN | SCS_PKTFLAG_ACK)) \
				== SCS_PKTFLAG_SYN ? true : false)

#define SCSPacketIsSynAck(xxx_self) \
		(((xxx_self)->flags & (SCS_PKTFLAG_SYN | SCS_PKTFLAG_ACK)) \
				== (SCS_PKTFLAG_SYN | SCS_PKTFLAG_ACK) ? true : false)

#define SCSPacketIsAck(xxx_self) \
		(((xxx_self)->flags & (SCS_PKTFLAG_SYN | SCS_PKTFLAG_ACK)) \
				== SCS_PKTFLAG_ACK ? true : false)

#define SCSPacketIsFin(xxx_self) \
		(((xxx_self)->flags & (SCS_PKTFLAG_FIN | SCS_PKTFLAG_ACK)) \
				== SCS_PKTFLAG_FIN ? true : false)

#define SCSPacketIsFinAck(xxx_self) \
		(((xxx_self)->flags & (SCS_PKTFLAG_FIN | SCS_PKTFLAG_ACK)) \
				== (SCS_PKTFLAG_FIN | SCS_PKTFLAG_ACK) ? true : false)

/* ---------------------------------------------------------------------------------------------- */

#define SCSPacketIsConnected(xxx_self) \
		((xxx_self)->state.connected)

#define SCSPacketIsCancel(xxx_self) \
		((xxx_self)->state.canceled)

#define SCSPacketIsControlPacket(xxx_self) \
		((xxx_self)->flags & SCS_PKTFLAG_CTL ? true : false)

/* ---------------------------------------------------------------------------------------------- */

#define SCSPacketHasPayload(xxx_self) \
		((((xxx_self)->flags & SCS_PKTFLAG_PLD) && \
		(0 < (xxx_self)->payload.length) && \
		((xxx_self)->payload.ptr != NULL)) ? true : false)

#define SCSPacketHasRedundancyCallbackFunction(xxx_self) \
		SCSRedundancyCallbackConfigCanCall(&(xxx_self)->redundancy.config)

/* ---------------------------------------------------------------------------------------------- */

extern void SCSPacketMonitor(void);

/* ============================================================================================== */

#endif /* SCS_5_PACKET_PACKET_H_ */
