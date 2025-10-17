#define SCS_SOURCECODE_FILEID	"5PKTUTL"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/2/logger.h"
#include "scs/5/channel/types.h"
#include "scs/5/packet/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

SCSConnectionId SCSGenerateConnectionId() {
#ifdef WIN32
#error
#else
	static uint16_t tmp_sn = 0;

	SCSConnectionId tmp_retval;
	struct timespec tmp_timestamp;
	uint32_t tmp_random;

	SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_REALTIME);
	tmp_random = ((uint32_t) (tmp_timestamp.tv_sec & 0x0000FFFF)) << 16;
	tmp_retval = tmp_random + tmp_sn;
	tmp_sn++;

	return tmp_retval;
#endif
}
SCSPacketSeqno SCSGeneratePacketSeqno(SCSPacketMode mode) {
#ifdef WIN32
#error
#else
	static uint16_t tmp_sn = 0;

	SCSPacketSeqno tmp_retval;
	struct timespec tmp_timestamp;
	uint64_t tmp_high;
	uint64_t tmp_low;

	SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_REALTIME);
	tmp_high = ((uint64_t) (tmp_timestamp.tv_sec & 0x00000000FFFFFFFF)) << 32;
	tmp_low = ((uint64_t) (tmp_timestamp.tv_nsec & 0x00000000FFFF0000));
	tmp_retval = tmp_high + tmp_low + tmp_sn;
	tmp_sn++;

	return tmp_retval;
#endif
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
