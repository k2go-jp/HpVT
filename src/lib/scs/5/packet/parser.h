#ifndef SCS_5_PACKET_PARSER_H_
#define SCS_5_PACKET_PARSER_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSParserPacket(SCSPointer * __restrict ptr, SCSPacket * __restrict packet);

/* ============================================================================================== */

#endif /* SCS_5_PACKET_PARSER_H_ */
