#ifndef SCS_5_PACKET_BUILDER_H_
#define SCS_5_PACKET_BUILDER_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/2/pointer.h"
#include "scs/5/packet/packet.h"

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSBuildPacket(SCSPacket * __restrict packet, SCSPointer * __restrict ptr);

/* ============================================================================================== */

#endif /* SCS_5_PACKET_BUILDER_H_ */
