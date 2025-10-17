#define SCS_SOURCECODE_FILEID	"5PKTPV1"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/in.h"
#include "scs/1/inttypes.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/1/unistd.h"
#include "scs/2/logger.h"
#include "scs/2/pointer.h"
#include "scs/3/utils.h"
#include "scs/5/defines.h"
#include "scs/5/packet/packet.h"
#include "scs/5/packet/types.h"
#include "scs/5/packet/v1/parser.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_MOVE
#error
#endif

#ifdef _SCS_READ
#error
#endif
#ifdef _SCS_READ_UINT8
#error
#endif
#ifdef _SCS_READ_UINT16
#error
#endif
#ifdef _SCS_READ_UINT24
#error
#endif
#ifdef _SCS_READ_UINT32
#error
#endif
#ifdef _SCS_READ_UINT64
#error
#endif

#ifdef _SCS_WRITE_UINT32
#error
#endif

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_MOVE(xxx_ptr, xxx_length) \
		if (SCSPointerMove(xxx_ptr, xxx_length) == false) {\
			SCS_LOG(ERROR, PACKET, 79911, "");\
			return false;\
		}

#define _SCS_READ(xxx_ptr, xxx_out, xxx_outlen) \
		if (SCSPointerRead(xxx_ptr, xxx_out, xxx_outlen) == false) {\
			SCS_LOG(ERROR, PACKET, 79911, "");\
			return false;\
		}
#define _SCS_READ_UINT8(xxx_ptr, xxx_value) \
		if (SCSPointerReadUint8(xxx_ptr, xxx_value) == false) {\
			SCS_LOG(ERROR, PACKET, 79911, "");\
			return false;\
		}
#define _SCS_READ_UINT16(xxx_ptr, xxx_value) \
		if (SCSPointerReadUint16(xxx_ptr, xxx_value) == true) {\
			xxx_value = ntohs(xxx_value);\
		}\
		else {\
			SCS_LOG(ERROR, PACKET, 79911, "");\
			return false;\
		}
#define _SCS_READ_UINT24(xxx_ptr, xxx_value) \
		if (SCSPointerReadUint24(xxx_ptr, xxx_value) == true) {\
			xxx_value = ntohl(xxx_value);\
		}\
		else {\
			SCS_LOG(ERROR, PACKET, 79911, "");\
			return false;\
		}
#define _SCS_READ_UINT32(xxx_ptr, xxx_value) \
		if (SCSPointerReadUint32(xxx_ptr, xxx_value) == true) {\
			xxx_value = ntohl(xxx_value);\
		}\
		else {\
			SCS_LOG(ERROR, PACKET, 79911, "");\
			return false;\
		}
#define _SCS_READ_UINT64(xxx_ptr, xxx_value) \
		if (SCSPointerReadUint64(xxx_ptr, xxx_value) == true) {\
			xxx_value = ntohll(xxx_value);\
		}\
		else {\
			SCS_LOG(ERROR, PACKET, 79911, "");\
			return false;\
		}

#define _SCS_WRITE_UINT32(xxx_ptr, xxx_value) \
		if (SCSPointerWriteUint32(xxx_ptr, htonl(xxx_value)) == false) {\
			SCS_LOG(ERROR, PACKET, 79911, "");\
			return false;\
		}

/* ---------------------------------------------------------------------------------------------- */

static bool _SCSReadNotificationInfo(SCSPointer * __restrict ptr, SCSPacket * __restrict packet) {
	SCSPointer tmp_ptr;
	SCSFeedbackInfo * tmp_info;
	uint32_t tmp_value32;
	uint64_t tmp_value64;

	tmp_ptr = *ptr;

	if ((tmp_info = (SCSFeedbackInfo *) malloc(sizeof(SCSFeedbackInfo))) != NULL) {
		SCSFeedbackInfoInitialize(tmp_info);
	}
	else {
		SCS_LOG(WARN, PACKET, 99999, "");
		return false;
	}

	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->timestamp = tmp_value64;

	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.sent.control.packets = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.sent.control.bytes = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.sent.payload.packets = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.sent.payload.bytes = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.sent.redundancy.packets = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.sent.redundancy.bytes = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.sent.dropped.packets = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.sent.dropped.bytes = tmp_value64;

	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.control.packets = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.control.bytes = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.payload.packets = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.payload.bytes = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.redundancy.packets = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.redundancy.bytes = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.unexpected.packets = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.reordering.packets = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.dropped.total.packets = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.dropped.total.bytes = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.dropped.duplicate.packets = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.dropped.duplicate.bytes = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.dropped.overrun.packets = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.dropped.overrun.bytes = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.dropped.outofrange.packets = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.dropped.outofrange.bytes = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.dropped.invalid.packets = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.dropped.invalid.bytes = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.dropped.broken.packets = tmp_value64;
	_SCS_READ_UINT64(tmp_ptr, tmp_value64);
	tmp_info->traffic.received.dropped.broken.bytes = tmp_value64;

	_SCS_READ_UINT32(tmp_ptr, tmp_value32);
	tmp_info->rtt.minimum = tmp_value32;
	_SCS_READ_UINT32(tmp_ptr, tmp_value32);
	tmp_info->rtt.maximum = tmp_value32;
	_SCS_READ_UINT32(tmp_ptr, tmp_value32);
	tmp_info->rtt.current = tmp_value32;

	_SCS_READ_UINT32(tmp_ptr, tmp_value32);
	tmp_info->plr.minimum = tmp_value32;
	_SCS_READ_UINT32(tmp_ptr, tmp_value32);
	tmp_info->plr.maximum = tmp_value32;
	_SCS_READ_UINT32(tmp_ptr, tmp_value32);
	tmp_info->plr.current = tmp_value32;

	_SCS_READ_UINT32(tmp_ptr, tmp_value32);
	tmp_info->mtu = tmp_value32;

	packet->notification.info = tmp_info;

	*ptr = tmp_ptr;

	return true;
}
static bool _SCSReadPayload(SCSPointer * __restrict ptr, SCSPacket * __restrict packet) {
	SCSPointer tmp_ptr;
	uint8_t * tmp_data_ptr;
	size_t tmp_data_size;
	uint16_t tmp_value16;

	SCSPointerCopy(*ptr, tmp_ptr);

	_SCS_READ_UINT16(tmp_ptr, tmp_value16);
	tmp_data_size = tmp_value16;

	if (SCS_PACKET_MAXPAYLOADSIZE <= tmp_data_size) {
		SCS_LOG(ERROR, PACKET, 79996, "<<%zu>>", tmp_data_size);
		return false;
	}

	if (0 < tmp_data_size) {
		if ((tmp_data_ptr = (uint8_t *) malloc(tmp_data_size)) == NULL) {
			SCS_LOG(ERROR, MEMORY, 00002, "<<%zu>>", tmp_data_size);
			return false;
		}

		_SCS_READ(tmp_ptr, tmp_data_ptr, tmp_data_size);

		if (SCSPacketSetPayload(packet, tmp_data_ptr, tmp_data_size) == false) {
			SCS_LOG(WARN, PACKET, 00000, "<<%zu>>", tmp_data_size);
		}
	}

	SCSPointerCopy(tmp_ptr, *ptr);

	return true;
}
bool SCSParsePacketV1(SCSPointer * __restrict ptr, SCSPacket * __restrict packet) {
	SCSPointer tmp_ptr;
	SCSPointer tmp_tagged_ptr;
	uint8_t tmp_value8;
	uint16_t tmp_value16;
	uint32_t tmp_value32;
	uint64_t tmp_value64;

//	if (ptr == NULL) {
//		SCS_LOG(ERROR, SYSTEM, 99998, "");
//		return false;
//	}

//	if (packet == NULL) {
//		SCS_LOG(ERROR, SYSTEM, 99998, "");
//		return false;
//	}

	SCSPointerInitialize(tmp_ptr, //
			SCSPointerGetReadPointer(*ptr), SCSPointerGetReadableLength(*ptr), 0);
	SCSPointerInitialize(tmp_tagged_ptr, 0, 0, 0);

	_SCS_READ_UINT16(tmp_ptr, tmp_value16);
	if (SCSPacketModeValidate(tmp_value16)) {
		packet->mode = tmp_value16;
	}
	else {
		SCS_LOG(ERROR, PACKET, 79996, "<<0x%04X>>", tmp_value16);
		return false;
	}

	_SCS_READ_UINT16(tmp_ptr, tmp_value16);
	if (SCSPacketFlagValidate(tmp_value16)) {
		packet->flags = tmp_value16;
	}
	else {
		SCS_LOG(ERROR, PACKET, 79996, "<<0x%04X>>", tmp_value16);
		return false;
	}

	_SCS_READ_UINT32(tmp_ptr, tmp_value32);
	if (SCSConnectionIdValidate(tmp_value32)) {
		packet->connid = tmp_value32;
	}
	else {
		SCS_LOG(ERROR, PACKET, 99997, "<<%"PRIu32">>", tmp_value32);
		return false;
	}

	switch (SCSPacketGetPacketMode(packet)) {
		case SCS_PKTMODE_LOWBAND: {
			_SCS_READ_UINT16(tmp_ptr, tmp_value16);
			packet->timestamp.peer = tmp_value16;
			_SCS_READ_UINT16(tmp_ptr, tmp_value16);
			packet->seqno = tmp_value16;
			break;
		}
		case SCS_PKTMODE_STANDARD: {
			_SCS_READ_UINT32(tmp_ptr, tmp_value32);
			packet->timestamp.peer = tmp_value32;
			_SCS_READ_UINT32(tmp_ptr, tmp_value32);
			packet->seqno = tmp_value32;
			break;
		}
		case SCS_PKTMODE_HIGHBAND: {
			_SCS_READ_UINT32(tmp_ptr, tmp_value32);
			packet->timestamp.peer = tmp_value32;
			_SCS_READ_UINT64(tmp_ptr, tmp_value64);
			packet->seqno = tmp_value64;
			break;
		}
		default: {
			SCS_LOG(ERROR, PACKET, 79996, "<<0x%04X>>", tmp_value16);
			return false;
		}
	}

	SCSTimespecSetCurrentTime(packet->timestamp.self, CLOCK_MONOTONIC);

	// SCS_PKTFLAG_SYN
	// SCS_PKTFLAG_ACK
	// SCS_PKTFLAG_PLD Must be last.
	// SCS_PKTFLAG_FIN
	// SCS_PKTFLAG_RST
	// SCS_PKTFLAG_RED

	if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_NTY)) {
		if (_SCSReadNotificationInfo(&tmp_ptr, packet) == false) {
			return false;
		}
	}

	// SCS_PKTFLAG_PMD

	if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_RTT)) {
		_SCS_READ_UINT8(tmp_ptr, tmp_value8);
		if (SCSRTTMeasFlagValidate(tmp_value8)) {
			packet->rttmeas.flag = tmp_value8;
		}
		else {
			SCS_LOG(ERROR, PACKET, 79996, "<<0x%02X>>", tmp_value8);
			return false;
		}

		_SCS_READ_UINT8(tmp_ptr, tmp_value8);
		// Pad

		_SCS_READ_UINT16(tmp_ptr, tmp_value16);
		if (SCSRTTMeasIdValidate(tmp_value16)) {
			packet->rttmeas.id = tmp_value16;
		}
		else {
			SCS_LOG(ERROR, PACKET, 79996, "<<0x%04X>>", tmp_value16);
			return false;
		}
	}

	if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_VRF)) {
		SCSPointerCopy(tmp_ptr, tmp_tagged_ptr);

		_SCS_READ_UINT8(tmp_ptr, tmp_value8);
		if (SCSPacketVerificationMethodValidate(tmp_value8)) {
			packet->verification.method = tmp_value8;
		}
		else {
			SCS_LOG(ERROR, PACKET, 79996, "<<0x%02X>>", tmp_value8);
			return false;
		}

		switch (SCSPacketGetVerificationMethod(packet)) {
			case SCS_PKTVRFMETHOD_CRC16: {
				_SCS_READ_UINT8(tmp_ptr, tmp_value8);
				// Pad
				_SCS_READ_UINT16(tmp_ptr, tmp_value16);
				packet->verification.code.crc16.value1 = tmp_value16;
				break;
			}
			default: {
				SCS_LOG(ALERT, PACKET, 79996, "<<0x%02X>>", SCSPacketGetVerificationMethod(packet));
				return false;
			}
		}
	}

	if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_PAD)) {
		_SCS_READ_UINT16(tmp_ptr, tmp_value16);
		packet->pad.offset = tmp_value16;
		_SCS_READ_UINT16(tmp_ptr, tmp_value16);
		packet->pad.length = tmp_value16;
	}

	if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_PLD)) {
		if (_SCSReadPayload(&tmp_ptr, packet) == false) {
			return false;
		}
	}

	if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_PAD)) {
		if (!SCSPointerCheckOffset(tmp_ptr, packet->pad.offset)) {
			SCS_LOG(ERROR, PACKET, 79996, "<<%zu,%zu>>", //
					packet->pad.offset, SCSPointerGetOffset(tmp_ptr));
			return false;
		}

		if (!SCSPointerCanRead(tmp_ptr, packet->pad.length)) {
			SCS_LOG(ERROR, PACKET, 79996, "<<%zu/%zu>>", //
					packet->pad.length, SCSPointerGetReadableLength(tmp_ptr));
			return false;
		}

		_SCS_MOVE(tmp_ptr, packet->pad.length);
	}

	if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_VRF)) {
		switch (SCSPacketGetVerificationMethod(packet)) {
			case SCS_PKTVRFMETHOD_CRC16: {
				uint8_t * tmp_data_ptr;
				size_t tmp_data_size;
				uint16_t tmp_code;

				_SCS_WRITE_UINT32(tmp_tagged_ptr, 0x00000000); // Pad

				tmp_data_ptr = SCSPointerGetPointer(tmp_ptr);
				tmp_data_size = SCSPointerGetOffset(tmp_ptr);

				if (SCSGenerateCRC16(tmp_data_ptr, tmp_data_size, &tmp_code) == false) {
					SCS_LOG(ERROR, PACKET, 79931, "");
					return false;
				}

				packet->verification.code.crc16.value2 = tmp_code;

				break;
			}
			default: {
				SCS_LOG(ERROR, PACKET, 79996, "<<0x%02X>>", SCSPacketGetVerificationMethod(packet));
				return false;
			}
		}
	}

	SCSPointerMove(*ptr, SCSPointerGetOffset(tmp_ptr));

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_MOVE

#undef _SCS_READ
#undef _SCS_READ_UINT8
#undef _SCS_READ_UINT16
#undef _SCS_READ_UINT32
#undef _SCS_READ_UINT64

#undef _SCS_WRITE_UINT32

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
