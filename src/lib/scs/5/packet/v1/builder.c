#define SCS_SOURCECODE_FILEID	"5PKTBV1"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/in.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/2/pointer.h"
#include "scs/3/utils.h"
#include "scs/5/callback/socket/feedback/config.h"
#include "scs/5/defines.h"
#include "scs/5/feedback.h"
#include "scs/5/packet/packet.h"
#include "scs/5/packet/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_WRITE
#error
#endif
#ifdef _SCS_WRITE_PAD
#error
#endif
#ifdef _SCS_WRITE_UINT8
#error
#endif
#ifdef _SCS_WRITE_UINT16
#error
#endif
#ifdef _SCS_WRITE_UINT24
#error
#endif
#ifdef _SCS_WRITE_UINT32
#error
#endif
#ifdef SCS_WRITE_UINT64
#error
#endif

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_WRITE(xxx_ptr, xxx_out, xxx_outlen) \
		if (SCSPointerWrite(xxx_ptr, xxx_out, xxx_outlen) == false) {\
			SCS_LOG(ERROR, PACKET, 79921, "");\
			return false;\
		}
#define _SCS_WRITE_PAD(xxx_ptr, xxx_length, xxx_value) \
		if (SCSPointerWritePad(xxx_ptr, xxx_length, xxx_value) == false) {\
			SCS_LOG(WARN, PACKET, 79921, "");\
			return false;\
		}
#define _SCS_WRITE_UINT8(xxx_ptr, xxx_value) \
		if (SCSPointerWriteUint8(xxx_ptr, xxx_value) == false) {\
			SCS_LOG(WARN, PACKET, 79921, "");\
			return false;\
		}
#define _SCS_WRITE_UINT16(xxx_ptr, xxx_value) \
		if (SCSPointerWriteUint16(xxx_ptr, htons(xxx_value)) == false) {\
			SCS_LOG(WARN, PACKET, 79921, "");\
			return false;\
		}
#define _SCS_WRITE_UINT24(xxx_ptr, xxx_value) \
		if (SCSPointerWriteUint24(xxx_ptr, htonl(xxx_value)) == false) {\
			SCS_LOG(WARN, PACKET, 79921, "");\
			return false;\
		}
#define _SCS_WRITE_UINT32(xxx_ptr, xxx_value) \
		if (SCSPointerWriteUint32(xxx_ptr, htonl(xxx_value)) == false) {\
			SCS_LOG(WARN, PACKET, 79921, "");\
			return false;\
		}
#define _SCS_WRITE_UINT64(xxx_ptr, xxx_value) \
		if (SCSPointerWriteUint64(xxx_ptr, htonll(xxx_value)) == false) {\
			SCS_LOG(WARN, PACKET, 79921, "");\
			return false;\
		}

/* ---------------------------------------------------------------------------------------------- */

static bool _SCSWriteNotificationInfo(SCSPacket * __restrict packet, SCSPointer * __restrict ptr) {
	SCSPointer tmp_ptr;
	SCSFeedbackInfo * tmp_info;

	if ((tmp_info = packet->notification.info) == NULL) {
		SCS_LOG(WARN, PACKET, 99999, "");
		return false;
	}

	SCSPointerCopy(*ptr, tmp_ptr);

	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->timestamp);

	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.sent.control.packets);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.sent.control.bytes);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.sent.payload.packets);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.sent.payload.bytes);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.sent.redundancy.packets);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.sent.redundancy.bytes);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.sent.dropped.packets);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.sent.dropped.bytes);

	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.control.packets);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.control.bytes);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.payload.packets);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.payload.bytes);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.redundancy.packets);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.redundancy.bytes);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.unexpected.packets);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.reordering.packets);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.dropped.total.packets);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.dropped.total.bytes);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.dropped.duplicate.packets);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.dropped.duplicate.bytes);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.dropped.overrun.packets);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.dropped.overrun.bytes);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.dropped.outofrange.packets);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.dropped.outofrange.bytes);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.dropped.invalid.packets);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.dropped.invalid.bytes);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.dropped.broken.packets);
	_SCS_WRITE_UINT64(tmp_ptr, tmp_info->traffic.received.dropped.broken.bytes);

	_SCS_WRITE_UINT32(tmp_ptr, tmp_info->rtt.minimum);
	_SCS_WRITE_UINT32(tmp_ptr, tmp_info->rtt.maximum);
	_SCS_WRITE_UINT32(tmp_ptr, tmp_info->rtt.current);

	_SCS_WRITE_UINT32(tmp_ptr, tmp_info->plr.minimum);
	_SCS_WRITE_UINT32(tmp_ptr, tmp_info->plr.maximum);
	_SCS_WRITE_UINT32(tmp_ptr, tmp_info->plr.current);

	_SCS_WRITE_UINT32(tmp_ptr, tmp_info->mtu);

	SCSPointerCopy(tmp_ptr, *ptr);

	return true;
}
static bool _SCSWriteExtendedNotificationInfo(SCSPacket * __restrict packet,
		SCSPointer * __restrict ptr) {
	SCSPointer tmp_ptr;
	SCSPointer tmp_tagged_ptr;
	uint8_t * tmp_payload_ptr;
	size_t tmp_payload_size;

	SCSPointerCopy(*ptr, tmp_ptr);

	SCSPointerCopy(tmp_ptr, tmp_tagged_ptr);
	_SCS_WRITE_UINT16(tmp_ptr, 0);

	SCSPointerCopy(tmp_ptr, *ptr);
			
	if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_PAD) == false) {
		tmp_payload_ptr = SCSPointerGetWritePointer(tmp_ptr);
		tmp_payload_size = SCSPointerGetWritableLength(tmp_ptr);
	}
	else {
		tmp_payload_ptr = SCSPointerGetWritePointer(tmp_ptr);
		tmp_payload_size = packet->pad.length - SCSPointerGetOffset(tmp_ptr);
	}

	if (SCS_PACKET_MAXPAYLOADSIZE < tmp_payload_size) {
		tmp_payload_size = SCS_PACKET_MAXPAYLOADSIZE;
	}

	if (SCSFeedbackCallbackConfigCanCall(&packet->notification.config)) {
		size_t tmp_write_len;

		tmp_write_len = SCSFeedbackCallbackConfigCall( //
				&packet->notification.config, tmp_payload_ptr, tmp_payload_size);
		if (tmp_write_len == ((size_t) -1)) {
			SCS_LOG(WARN, PACKET, 79921, "");
			return true;
		}

		if (tmp_payload_size < tmp_write_len) {
			SCS_LOG(WARN, PACKET, 79921, "");
			return true;
		}

		if (SCSPointerMove(tmp_ptr, tmp_write_len) == false) {
			SCS_LOG(WARN, PACKET, 79921, "");
			return true;
		}

		_SCS_WRITE_UINT16(tmp_tagged_ptr, tmp_write_len);
	}

	SCSPointerCopy(tmp_ptr, *ptr);

	return true;
}
static bool _SCSWriteVerificationCode(SCSPacket * __restrict packet, SCSPointer * __restrict ptr,
		SCSPointer * __restrict tagged_ptr) {
	SCSPointer tmp_ptr;
	SCSPacketVerificationMethod tmp_method;

	SCSPointerCopy(*ptr, tmp_ptr);

	switch ((tmp_method = SCSPacketGetVerificationMethod(packet))) {
		case SCS_PKTVRFMETHOD_CRC16: {
			uint8_t * tmp_data_ptr;
			size_t tmp_data_size;
			uint16_t tmp_code;

			tmp_data_ptr = SCSPointerGetPointer(tmp_ptr);
			tmp_data_size = SCSPointerGetOffset(tmp_ptr);

			if (SCSGenerateCRC16(tmp_data_ptr, tmp_data_size, &tmp_code) == false) {
				SCS_LOG(WARN, PACKET, 99999, "");
				return false;
			}

			_SCS_WRITE_UINT8(*tagged_ptr, tmp_method);
			_SCS_WRITE_UINT8(*tagged_ptr, 0x00); // Pad
			_SCS_WRITE_UINT16(*tagged_ptr, tmp_code);

			break;
		}
		case SCS_PKTVRFMETHOD_NONE: {
			_SCS_WRITE_UINT8(*tagged_ptr, tmp_method);
			_SCS_WRITE_UINT24(*tagged_ptr, 0x00000000);
			break;
		}
		default: {
			SCS_LOG(WARN, PACKET, 99999, "<<0x%02X>>", SCSPacketGetVerificationMethod(packet));
			return false;
		}
	}

	SCSPointerCopy(tmp_ptr, *ptr);

	return true;
}

bool SCSBuildPacketV1(SCSPacket * __restrict packet, SCSPointer * __restrict ptr) {
	SCSPointer tmp_ptr;
	struct {
		SCSPointer verify;
		SCSPointer pad;
	} tmp_tagged_ptr;

//	if (packet == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (ptr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (SCSPacketGetPacketVersion(packet) != SCS_PKTMODE_V1) {
//		SCS_LOG(WARN, SYSTEM, 99999,  "<<%d>>", tmp_version);
//		return false;
//	}

	SCSPointerInitialize(tmp_ptr, //
			SCSPointerGetWritePointer(*ptr), SCSPointerGetWritableLength(*ptr), 0);
	SCSPointerInitialize(tmp_tagged_ptr.verify, 0, 0, 0);
	SCSPointerInitialize(tmp_tagged_ptr.pad, 0, 0, 0);

	_SCS_WRITE_UINT16(tmp_ptr, packet->mode);
	_SCS_WRITE_UINT16(tmp_ptr, packet->flags);
	_SCS_WRITE_UINT32(tmp_ptr, packet->connid);

	switch (SCSPacketGetPacketMode(packet)) {
		case SCS_PKTMODE_LOWBAND: {
			uint16_t tmp_value16;

			tmp_value16 = (uint16_t) SCSTimespecGetSec(packet->timestamp.self);
			_SCS_WRITE_UINT16(tmp_ptr, tmp_value16);

			tmp_value16 = (uint16_t) packet->seqno;
			_SCS_WRITE_UINT16(tmp_ptr, tmp_value16);

			break;
		}
		case SCS_PKTMODE_STANDARD: {
			uint32_t tmp_value32;

			tmp_value32 = (uint32_t) SCSTimespecConvertToMillisec32(packet->timestamp.self);
			_SCS_WRITE_UINT32(tmp_ptr, tmp_value32);

			tmp_value32 = (uint32_t) packet->seqno;
			_SCS_WRITE_UINT32(tmp_ptr, tmp_value32);

			break;
		}
		case SCS_PKTMODE_HIGHBAND: {
			uint32_t tmp_value32;

			tmp_value32 = (uint32_t) SCSTimespecConvertToMillisec32(packet->timestamp.self);
			_SCS_WRITE_UINT32(tmp_ptr, tmp_value32);

			_SCS_WRITE_UINT64(tmp_ptr, packet->seqno);

			break;
		}
		default: {
			SCS_LOG(WARN, PACKET, 79996, "<<%d>>", SCSPacketGetPacketMode(packet));
			return false;
		}
	}

	//SCS_PKTFLAG_SYN
	//SCS_PKTFLAG_ACK
	//SCS_PKTFLAG_PLD Must be last.
	//SCS_PKTFLAG_FIN
	//SCS_PKTFLAG_RST
	//SCS_PKTFLAG_RED

	if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_NTY)) {
		if (_SCSWriteNotificationInfo(packet, &tmp_ptr) == false) {
			return false;
		}
	}

	//SCS_PKTFLAG_PMD

	if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_RTT)) {
		_SCS_WRITE_UINT8(tmp_ptr, packet->rttmeas.flag);
		_SCS_WRITE_UINT8(tmp_ptr, 0x00);	// Pad
		_SCS_WRITE_UINT16(tmp_ptr, packet->rttmeas.id);
	}

	if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_VRF)) {
		SCSPointerCopy(tmp_ptr, tmp_tagged_ptr.verify);
		_SCS_WRITE_UINT32(tmp_ptr, 0x00000000);
	}

	if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_PAD)) {
		SCSPointerCopy(tmp_ptr, tmp_tagged_ptr.pad);
		_SCS_WRITE_UINT32(tmp_ptr, 0x00000000);
	}

	if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_NTY) == false) {
		if (SCSPacketHasPayload(packet)) {
			//SCS_HEXDUMP(packet->payload.ptr, packet->payload.length, 16);
			if (SCS_PACKET_MAXPAYLOADSIZE < packet->payload.length) {
				SCS_LOG(WARN, PACKET, 99999, "<<%zu>>", packet->payload.length);
				return false;
			}

			_SCS_WRITE_UINT16(tmp_ptr, packet->payload.length);
			_SCS_WRITE(tmp_ptr, packet->payload.ptr, packet->payload.length);
		}
	}
	else {
		if (_SCSWriteExtendedNotificationInfo(packet, &tmp_ptr) == false) {
			return false;
		}
	}

	if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_PAD)) {
		size_t tmp_offset;

		if (!SCSPointerCanWrite(tmp_ptr, packet->pad.length)) {
			SCS_LOG(WARN, PACKET, 99999, "<<%zu>>", packet->pad.length);
			return false;
		}

		if ((tmp_offset = SCSPointerGetOffset(tmp_ptr)) <= packet->pad.length) {
			size_t tmp_pad_len;

			tmp_pad_len = packet->pad.length - tmp_offset;

			_SCS_WRITE_UINT16(tmp_tagged_ptr.pad, SCSPointerGetOffset(tmp_ptr));
			_SCS_WRITE_UINT16(tmp_tagged_ptr.pad, tmp_pad_len);
			_SCS_WRITE_PAD(tmp_ptr, packet->pad.length, 0x00);
		}
		else {
			SCS_LOG(WARN, PACKET, 99999, "<<%zu>>", packet->pad.length);
			return false;
		}
	}

	if (SCSPacketCheckFlags(packet, SCS_PKTFLAG_VRF)) {
		if (_SCSWriteVerificationCode(packet, &tmp_ptr, &tmp_tagged_ptr.verify) == false) {
			return false;
		}
	}

	SCSPointerMove(*ptr, SCSPointerGetOffset(tmp_ptr));

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_WRITE
#undef _SCS_WRITE_PAD
#undef _SCS_WRITE_UINT8
#undef _SCS_WRITE_UINT16
#undef _SCS_WRITE_UINT24
#undef _SCS_WRITE_UINT32
#undef _SCS_WRITE_UINT64

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
