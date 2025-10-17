#define SCS_SOURCECODE_FILEID	"5PKTPSR"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/in.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/2/pointer.h"
#include "scs/5/packet/packet.h"
#include "scs/5/packet/types.h"
#include "scs/5/packet/v1/parser.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

bool SCSParserPacket(SCSPointer * __restrict ptr, SCSPacket * __restrict packet) {
	bool tmp_result;
	SCSPointer tmp_ptr;
	uint16_t tmp_value;
	int tmp_version;

	if (ptr == NULL) {
		SCS_LOG(ERROR, SYSTEM, 99998, "");
		return false;
	}

	if (packet == NULL) {
		SCS_LOG(ERROR, SYSTEM, 99998, "");
		return false;
	}

	SCSPointerCopy(*ptr, tmp_ptr);

	if (SCSPointerReadUint16(tmp_ptr, tmp_value) == true) {
		tmp_value = ntohs(tmp_value);
	}
	else {
		SCS_LOG(ERROR, PACKET, 79911, "");
		return false;
	}

	if (SCSPacketModeValidate(tmp_value) == true) {
		packet->mode = tmp_value;
	}
	else {
		SCS_LOG(ERROR, PACKET, 79996, "<<0x%04X>>", tmp_value);
		return false;
	}

	switch ((tmp_version = SCSPacketGetPacketVersion(packet))) {
		case SCS_PKTMODE_V1: {
			tmp_result = SCSParsePacketV1(ptr, packet);
			break;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", tmp_version);
			return false;
		}
	}

	return tmp_result;
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
