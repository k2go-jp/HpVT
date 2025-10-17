#ifndef SCS_5_PACKET_UTILS_H_
#define SCS_5_PACKET_UTILS_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/5/channel/types.h"

/* ---------------------------------------------------------------------------------------------- */

extern SCSConnectionId SCSGenerateConnectionId(void);

/* ---------------------------------------------------------------------------------------------- */

extern SCSPacketSeqno SCSGeneratePacketSeqno(SCSPacketMode mode);

/* ============================================================================================== */

#endif /* SCS_5_PACKET_UTILS_H_ */
