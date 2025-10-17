#define SCS_SOURCECODE_FILEID	"5PKTBLD"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/2/pointer.h"
#include "scs/5/packet/packet.h"
#include "scs/5/packet/types.h"
#include "scs/5/packet/v1/builder.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

bool SCSBuildPacket(SCSPacket * __restrict packet, SCSPointer * __restrict ptr) {
	bool tmp_result;
	int tmp_version;

	if (packet == NULL) {
		SCS_LOG(ERROR, SYSTEM, 99998, "");
		return false;
	}

	if (ptr == NULL) {
		SCS_LOG(ERROR, SYSTEM, 99998, "");
		return false;
	}

	switch ((tmp_version = SCSPacketGetPacketVersion(packet))) {
		case SCS_PKTMODE_V1: {
			tmp_result = SCSBuildPacketV1(packet, ptr);
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
