#define SCS_SOURCECODE_FILEID	"5FTRTCU"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/2/pointer.h"
#include "scs/5/feature/traffic/counter.h"
#include "scs/5/packet/types.h"
#include "scs/5/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_GETBYTES
#error
#endif
#ifdef _SCS_LOG
#error
#endif

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_GETBYTES(xxx_packet, xxx_bytes, xxx_result) \
	if ((xxx_result = xxx_bytes) < 1) {\
		if (xxx_packet != NULL) {\
			xxx_result = SCSPacketGetPayloadSize(xxx_packet);\
		}\
		else {\
			xxx_result = 0;\
		}\
	}\
	else {\
			xxx_result = 0;\
	}
#define _SCS_LOG(xxx_type, xxx_format, xxx_args...) \
		switch(type) {\
			case SCS_LOGTYPE_DEBUG: {\
				SCS_LOG(NOTICE, DEBUG, 00000, xxx_format, xxx_args);\
				break;\
			}\
			case SCS_LOGTYPE_MEMORY: {\
				SCS_LOG(NOTICE, MEMORY, 00000, xxx_format, xxx_args);\
				break;\
			}\
			case SCS_LOGTYPE_PACKET: {\
				SCS_LOG(NOTICE, PACKET, 00000, xxx_format, xxx_args);\
				break;\
			}\
			case SCS_LOGTYPE_SOCKET: {\
				SCS_LOG(NOTICE, SOCKET, 00000, xxx_format, xxx_args);\
				break;\
			}\
			case SCS_LOGTYPE_SYSTEM: {\
				SCS_LOG(NOTICE, SYSTEM, 00000, xxx_format, xxx_args);\
				break;\
			}\
		}

/* ---------------------------------------------------------------------------------------------- */

static inline void _SCSNetworkTrafficIncreaseSentPacket(SCSNetworkTrafficCounter * __restrict self,
		SCSPacket * __restrict packet, size_t bytes) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	if (SCSPacketIsControlPacket(packet)) {
		SCSAtomicIncrease(self->sent.control.packets);
		SCSAtomicAdd(self->sent.control.bytes, bytes);
	}
	else {
		SCSAtomicIncrease(self->sent.payload.packets);
		SCSAtomicAdd(self->sent.payload.bytes, bytes);

		if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_RED)) {
			SCSAtomicIncrease(self->sent.redundancy.packets);
			SCSAtomicAdd(self->sent.redundancy.bytes, bytes);
		}
	}

}

static inline void _SCSNetworkTrafficIncreaseDroppedPacketInSending(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSAtomicIncrease(self->sent.dropped.packets);
	SCSAtomicAdd(self->sent.dropped.bytes, bytes);

}

static inline void _SCSNetworkTrafficIncreaseCancelPacketInSending(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSAtomicIncrease(self->sent.cancel.packets);
	SCSAtomicAdd(self->sent.cancel.bytes, bytes);

}

static inline void _SCSNetworkTrafficIncreaseReceivedPacket(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	if (SCSPacketIsControlPacket(packet)) {
		SCSAtomicIncrease(self->received.control.packets);
		SCSAtomicAdd(self->received.control.bytes, bytes);
	}
	else {
		SCSAtomicIncrease(self->received.payload.packets);
		SCSAtomicAdd(self->received.payload.bytes, bytes);

		if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_RED)) {
			SCSAtomicIncrease(self->received.redundancy.packets);
			SCSAtomicAdd(self->received.redundancy.bytes, bytes);
		}
	}

}

static inline void _SCSNetworkTrafficIncreaseDroppedPacketInReceiving(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSAtomicIncrease(self->received.dropped.total.packets);
	SCSAtomicAdd(self->received.dropped.total.bytes, bytes);

}

static inline void _SCSNetworkTrafficIncreaseDuplicatedPacketInReceiving(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSAtomicIncrease(self->received.dropped.duplicate.packets);
	SCSAtomicAdd(self->received.dropped.duplicate.bytes, bytes);

}

static inline void _SCSNetworkTrafficIncreaseOverrunPacketInReceiving(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSAtomicIncrease(self->received.dropped.overrun.packets);
	SCSAtomicAdd(self->received.dropped.overrun.bytes, bytes);

}

static inline void _SCSNetworkTrafficIncreaseOutOfRangePacketInReceiving(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSAtomicIncrease(self->received.dropped.outofrange.packets);
	SCSAtomicAdd(self->received.dropped.outofrange.bytes, bytes);

}

static inline void _SCSNetworkTrafficIncreaseInvalidPacketInReceiving(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSAtomicIncrease(self->received.dropped.invalid.packets);
	SCSAtomicAdd(self->received.dropped.invalid.bytes, bytes);

}

static inline void _SCSNetworkTrafficIncreaseBrokenPacketInReceiving(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSAtomicIncrease(self->received.dropped.broken.packets);
	SCSAtomicAdd(self->received.dropped.broken.bytes, bytes);

}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSNetworkTrafficCounterInitialize(SCSNetworkTrafficCounter * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	memset(self, 0, sizeof(SCSNetworkTrafficCounter));

	SCSAtomicInitialize(self->sent.control.packets, 0);
	SCSAtomicInitialize(self->sent.control.bytes, 0);
	SCSAtomicInitialize(self->sent.payload.packets, 0);
	SCSAtomicInitialize(self->sent.payload.bytes, 0);
	SCSAtomicInitialize(self->sent.redundancy.packets, 0);
	SCSAtomicInitialize(self->sent.redundancy.bytes, 0);
	SCSAtomicInitialize(self->sent.dropped.packets, 0);
	SCSAtomicInitialize(self->sent.dropped.bytes, 0);

	SCSAtomicInitialize(self->received.control.packets, 0);
	SCSAtomicInitialize(self->received.control.bytes, 0);
	SCSAtomicInitialize(self->received.payload.packets, 0);
	SCSAtomicInitialize(self->received.payload.bytes, 0);
	SCSAtomicInitialize(self->received.redundancy.packets, 0);
	SCSAtomicInitialize(self->received.redundancy.bytes, 0);
	SCSAtomicInitialize(self->received.unexpected.packets, 0);
	SCSAtomicInitialize(self->received.reordering.packets, 0);
	SCSAtomicInitialize(self->received.dropped.total.packets, 0);
	SCSAtomicInitialize(self->received.dropped.total.bytes, 0);
	SCSAtomicInitialize(self->received.dropped.duplicate.packets, 0);
	SCSAtomicInitialize(self->received.dropped.duplicate.bytes, 0);
	SCSAtomicInitialize(self->received.dropped.overrun.packets, 0);
	SCSAtomicInitialize(self->received.dropped.overrun.bytes, 0);
	SCSAtomicInitialize(self->received.dropped.outofrange.packets, 0);
	SCSAtomicInitialize(self->received.dropped.outofrange.bytes, 0);
	SCSAtomicInitialize(self->received.dropped.invalid.packets, 0);
	SCSAtomicInitialize(self->received.dropped.invalid.bytes, 0);
	SCSAtomicInitialize(self->received.dropped.broken.packets, 0);
	SCSAtomicInitialize(self->received.dropped.broken.bytes, 0);

}
inline void SCSNetworkTrafficCounterFinalize(SCSNetworkTrafficCounter * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSAtomicFinalize(self->sent.control.packets, 0);
	SCSAtomicFinalize(self->sent.payload.packets, 0);
	SCSAtomicFinalize(self->sent.payload.bytes, 0);
	SCSAtomicFinalize(self->sent.redundancy.packets, 0);
	SCSAtomicFinalize(self->sent.redundancy.bytes, 0);
	SCSAtomicFinalize(self->sent.dropped.packets, 0);
	SCSAtomicFinalize(self->sent.dropped.bytes, 0);

	SCSAtomicFinalize(self->received.control.packets, 0);
	SCSAtomicFinalize(self->received.control.bytes, 0);
	SCSAtomicFinalize(self->received.payload.packets, 0);
	SCSAtomicFinalize(self->received.payload.bytes, 0);
	SCSAtomicFinalize(self->received.redundancy.packets, 0);
	SCSAtomicFinalize(self->received.redundancy.bytes, 0);
	SCSAtomicFinalize(self->received.unexpected.packets, 0);
	SCSAtomicFinalize(self->received.reordering.packets, 0);
	SCSAtomicFinalize(self->received.dropped.total.packets, 0);
	SCSAtomicFinalize(self->received.dropped.total.bytes, 0);
	SCSAtomicFinalize(self->received.dropped.duplicate.packets, 0);
	SCSAtomicFinalize(self->received.dropped.duplicate.bytes, 0);
	SCSAtomicFinalize(self->received.dropped.overrun.packets, 0);
	SCSAtomicFinalize(self->received.dropped.overrun.bytes, 0);
	SCSAtomicFinalize(self->received.dropped.outofrange.packets, 0);
	SCSAtomicFinalize(self->received.dropped.outofrange.bytes, 0);
	SCSAtomicFinalize(self->received.dropped.invalid.packets, 0);
	SCSAtomicFinalize(self->received.dropped.invalid.bytes, 0);
	SCSAtomicFinalize(self->received.dropped.broken.packets, 0);
	SCSAtomicFinalize(self->received.dropped.broken.bytes, 0);

	memset(self, 0, sizeof(SCSNetworkTrafficCounter));

}

/* ---------------------------------------------------------------------------------------------- */

void SCSNetworkTrafficCounterIncreaseSentPacket(SCSNetworkTrafficCounter * __restrict self,
		SCSPacket * __restrict packet, size_t bytes) {
	size_t tmp_bytes;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	_SCS_GETBYTES(packet, bytes, tmp_bytes);
	_SCSNetworkTrafficIncreaseSentPacket(self, packet, bytes);

}

void SCSNetworkTrafficCounterIncreaseDroppedPacketInSending(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {
	size_t tmp_bytes;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	_SCS_GETBYTES(packet, bytes, tmp_bytes);
	_SCSNetworkTrafficIncreaseSentPacket(self, packet, bytes);
	_SCSNetworkTrafficIncreaseDroppedPacketInSending(self, packet, bytes);

}

void SCSNetworkTrafficCounterIncreaseCancelPacketInSending(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {
	size_t tmp_bytes;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	_SCS_GETBYTES(packet, bytes, tmp_bytes);
	_SCSNetworkTrafficIncreaseSentPacket(self, packet, bytes);
	_SCSNetworkTrafficIncreaseCancelPacketInSending(self, packet, bytes);

}

void SCSNetworkTrafficCounterIncreaseReceivedPacket(SCSNetworkTrafficCounter * __restrict self,
		SCSPacket * __restrict packet, size_t bytes) {
	size_t tmp_bytes;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	_SCS_GETBYTES(packet, bytes, tmp_bytes);
	_SCSNetworkTrafficIncreaseReceivedPacket(self, packet, bytes);

}

void SCSNetworkTrafficCounterIncreaseUnexpectedPacketInReceiving(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSAtomicIncrease(self->received.unexpected.packets);

}

void SCSNetworkTrafficCounterIncreaseReorderingPacketInReceiving(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSAtomicIncrease(self->received.reordering.packets);

}

void SCSNetworkTrafficCounterIncreaseDroppedPacketInReceiving(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {
	size_t tmp_bytes;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	_SCS_GETBYTES(packet, bytes, tmp_bytes);
	_SCSNetworkTrafficIncreaseReceivedPacket(self, packet, bytes);
	_SCSNetworkTrafficIncreaseDroppedPacketInReceiving(self, packet, bytes);

}

void SCSNetworkTrafficCounterIncreaseDuplicatedPacketInReceiving(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {
	size_t tmp_bytes;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	_SCS_GETBYTES(packet, bytes, tmp_bytes);
	_SCSNetworkTrafficIncreaseReceivedPacket(self, packet, bytes);
	_SCSNetworkTrafficIncreaseDroppedPacketInReceiving(self, packet, bytes);
	_SCSNetworkTrafficIncreaseDuplicatedPacketInReceiving(self, packet, bytes);

}

void SCSNetworkTrafficCounterIncreaseOverrunPacketInReceiving(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {
	size_t tmp_bytes;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	_SCS_GETBYTES(packet, bytes, tmp_bytes);
	_SCSNetworkTrafficIncreaseReceivedPacket(self, packet, bytes);
	_SCSNetworkTrafficIncreaseDroppedPacketInReceiving(self, packet, bytes);
	_SCSNetworkTrafficIncreaseOverrunPacketInReceiving(self, packet, bytes);

}

void SCSNetworkTrafficCounterIncreaseOutOfRangePacketInReceiving(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {
	size_t tmp_bytes;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	_SCS_GETBYTES(packet, bytes, tmp_bytes);
	_SCSNetworkTrafficIncreaseReceivedPacket(self, packet, bytes);
	_SCSNetworkTrafficIncreaseDroppedPacketInReceiving(self, packet, bytes);
	_SCSNetworkTrafficIncreaseOutOfRangePacketInReceiving(self, packet, bytes);

}

void SCSNetworkTrafficCounterIncreaseInvalidPacketInReceiving(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {
	size_t tmp_bytes;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	_SCS_GETBYTES(packet, bytes, tmp_bytes);
	_SCSNetworkTrafficIncreaseReceivedPacket(self, packet, bytes);
	_SCSNetworkTrafficIncreaseDroppedPacketInReceiving(self, packet, bytes);
	_SCSNetworkTrafficIncreaseInvalidPacketInReceiving(self, packet, bytes);

}

void SCSNetworkTrafficCounterIncreaseBrokenPacketInReceiving(
		SCSNetworkTrafficCounter * __restrict self, SCSPacket * __restrict packet, size_t bytes) {
	size_t tmp_bytes;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	_SCS_GETBYTES(packet, bytes, tmp_bytes);
	_SCSNetworkTrafficIncreaseReceivedPacket(self, packet, bytes);
	_SCSNetworkTrafficIncreaseDroppedPacketInReceiving(self, packet, bytes);
	_SCSNetworkTrafficIncreaseBrokenPacketInReceiving(self, packet, bytes);

}

/* ---------------------------------------------------------------------------------------------- */

void SCSNetworkTrafficCounterGet(SCSNetworkTrafficCounter * __restrict self,
		SCSNetworkTrafficInfo * __restrict out) {
	SCSNetworkTrafficInfo tmp_value;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSNetworkTrafficInfoInitalize(&tmp_value);

	tmp_value.sent.control.packets = SCSAtomicGet(self->sent.control.packets);
	tmp_value.sent.payload.packets = SCSAtomicGet(self->sent.payload.packets);
	tmp_value.sent.redundancy.packets = SCSAtomicGet(self->sent.redundancy.packets);
	tmp_value.sent.dropped.packets = SCSAtomicGet(self->sent.dropped.packets);
	tmp_value.received.control.packets = SCSAtomicGet(self->received.control.packets);
	tmp_value.received.payload.packets = SCSAtomicGet(self->received.payload.packets);
	tmp_value.received.redundancy.packets = SCSAtomicGet(self->received.redundancy.packets);
	tmp_value.received.unexpected.packets = SCSAtomicGet(self->received.unexpected.packets);
	tmp_value.received.reordering.packets = SCSAtomicGet(self->received.reordering.packets);
	tmp_value.received.dropped.total.packets = SCSAtomicGet(self->received.dropped.total.packets);
	tmp_value.received.dropped.duplicate.packets = SCSAtomicGet(self->received.dropped.duplicate.packets);
	tmp_value.received.dropped.overrun.packets = SCSAtomicGet(self->received.dropped.overrun.packets);
	tmp_value.received.dropped.outofrange.packets = SCSAtomicGet(self->received.dropped.outofrange.packets);
	tmp_value.received.dropped.invalid.packets = SCSAtomicGet(self->received.dropped.invalid.packets);
	tmp_value.received.dropped.broken.packets = SCSAtomicGet(self->received.dropped.broken.packets);

	tmp_value.sent.control.bytes = SCSAtomicGet(self->sent.control.bytes);
	tmp_value.sent.payload.bytes = SCSAtomicGet(self->sent.payload.bytes);
	tmp_value.sent.redundancy.bytes = SCSAtomicGet(self->sent.redundancy.bytes);
	tmp_value.sent.dropped.bytes = SCSAtomicGet(self->sent.dropped.bytes);
	tmp_value.received.control.bytes = SCSAtomicGet(self->received.control.bytes);
	tmp_value.received.payload.bytes = SCSAtomicGet(self->received.payload.bytes);
	tmp_value.received.redundancy.bytes = SCSAtomicGet(self->received.redundancy.bytes);
	tmp_value.received.dropped.total.bytes = SCSAtomicGet(self->received.dropped.total.bytes);
	tmp_value.received.dropped.duplicate.bytes = SCSAtomicGet(self->received.dropped.duplicate.bytes);
	tmp_value.received.dropped.overrun.bytes = SCSAtomicGet(self->received.dropped.overrun.bytes);
	tmp_value.received.dropped.outofrange.bytes = SCSAtomicGet(self->received.dropped.outofrange.bytes);
	tmp_value.received.dropped.invalid.bytes = SCSAtomicGet(self->received.dropped.invalid.bytes);
	tmp_value.received.dropped.broken.bytes = SCSAtomicGet(self->received.dropped.broken.bytes);

	memcpy(out, &tmp_value, sizeof(SCSNetworkTrafficInfo));

}

/* ---------------------------------------------------------------------------------------------- */

void SCSNetworkTrafficCounterLogging(SCSNetworkTrafficCounter * self, SCSLogType type) {
	char tmp_message[512];
	SCSPointer tmp_ptr;
	int tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSPointerInitialize(tmp_ptr, tmp_message, sizeof(tmp_message), 0);
	SCSPointerPrint(tmp_ptr, tmp_result, "SEND:");
	SCSPointerPrint(tmp_ptr, tmp_result, "CTL=%"PRIu64"(%"PRIu64"),", //
			SCSAtomicGet(self->sent.control.packets),//
			SCSAtomicGet(self->sent.control.bytes));
	SCSPointerPrint(tmp_ptr, tmp_result, "PLD=%"PRIu64"(%"PRIu64"),", //
			SCSAtomicGet(self->sent.payload.packets),//
			SCSAtomicGet(self->sent.payload.bytes));
	SCSPointerPrint(tmp_ptr, tmp_result, "RED=%"PRIu64"(%"PRIu64"),", //
			SCSAtomicGet(self->sent.redundancy.packets),//
			SCSAtomicGet(self->sent.redundancy.bytes));
	SCSPointerPrint(tmp_ptr, tmp_result, "DRP=%"PRIu64"(%"PRIu64"),", //
			SCSAtomicGet(self->sent.dropped.packets),//
			SCSAtomicGet(self->sent.dropped.bytes));
	_SCS_LOG(type, "%s", tmp_message);

	SCSPointerInitialize(tmp_ptr, tmp_message, sizeof(tmp_message), 0);
	SCSPointerPrint(tmp_ptr, tmp_result, "RECV:");
	SCSPointerPrint(tmp_ptr, tmp_result, "CTL=%"PRIu64"(%"PRIu64"),", //
			SCSAtomicGet(self->received.control.packets),//
			SCSAtomicGet(self->received.control.bytes));
	SCSPointerPrint(tmp_ptr, tmp_result, "PLD=%"PRIu64"(%"PRIu64"),", //
			SCSAtomicGet(self->received.payload.packets),//
			SCSAtomicGet(self->received.payload.bytes));
	SCSPointerPrint(tmp_ptr, tmp_result, "RED=%"PRIu64"(%"PRIu64"),", //
			SCSAtomicGet(self->received.redundancy.packets),//
			SCSAtomicGet(self->received.redundancy.bytes));
	SCSPointerPrint(tmp_ptr, tmp_result, "UEX=%"PRIu64",", //
			SCSAtomicGet(self->received.unexpected.packets));
	SCSPointerPrint(tmp_ptr, tmp_result, "REO=%"PRIu64",", //
			SCSAtomicGet(self->received.reordering.packets));
	SCSPointerPrint(tmp_ptr, tmp_result, "DRP=%"PRIu64"(%"PRIu64"),", //
			SCSAtomicGet(self->received.dropped.total.packets),//
			SCSAtomicGet(self->received.dropped.total.bytes));
	SCSPointerPrint(tmp_ptr, tmp_result, "DUP=%"PRIu64"(%"PRIu64"),", //
			SCSAtomicGet(self->received.dropped.duplicate.packets),//
			SCSAtomicGet(self->received.dropped.duplicate.bytes));
	SCSPointerPrint(tmp_ptr, tmp_result, "OOR=%"PRIu64"(%"PRIu64"),", //
			SCSAtomicGet(self->received.dropped.outofrange.packets),//
			SCSAtomicGet(self->received.dropped.outofrange.bytes));
	SCSPointerPrint(tmp_ptr, tmp_result, "OVR=%"PRIu64"(%"PRIu64"),", //
			SCSAtomicGet(self->received.dropped.overrun.packets),//
			SCSAtomicGet(self->received.dropped.overrun.bytes));
	SCSPointerPrint(tmp_ptr, tmp_result, "INV=%"PRIu64"(%"PRIu64"),", //
			SCSAtomicGet(self->received.dropped.invalid.packets),//
			SCSAtomicGet(self->received.dropped.invalid.bytes));
	SCSPointerPrint(tmp_ptr, tmp_result, "BRK=%"PRIu64"(%"PRIu64"),", //
			SCSAtomicGet(self->received.dropped.broken.packets),//
			SCSAtomicGet(self->received.dropped.broken.bytes));
	_SCS_LOG(type, "%s", tmp_message);

}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_GETBYTES
#undef _SCS_LOG

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
