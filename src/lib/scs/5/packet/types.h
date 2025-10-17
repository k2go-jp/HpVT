#ifndef SCS_5_PACKET_TYPES_H_
#define SCS_5_PACKET_TYPES_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/2/atomic.h"

/* ---------------------------------------------------------------------------------------------- */

typedef int SCSPacketMode;

#define SCS_PKTMODE_NONE			(0x0000)	// 0000 0000 0000 0000

#define SCS_PKTMODE_V1				(0x1000)	// 0001 0000 0000 0000

#define SCS_PKTMODE_LOWBAND			(0x0100)	// 0000 0001 0000 0000
#define SCS_PKTMODE_STANDARD		(0x0200)	// 0000 0010 0000 0000
#define SCS_PKTMODE_HIGHBAND		(0x0400)	// 0000 0100 0000 0000

#define SCS_PKTMODE_DATAGRAM		(0x0010)	// 0000 0000 0001 0000
#define SCS_PKTMODE_DATACELL		(0x0020)	// 0000 0000 0010 0000
#define SCS_PKTMODE_STREAM			(0x0040)	// 0000 0000 0100 0000

#define SCS_PKTMODE_MASKFULL		(0xFFFF)	// 1111 1111 1111 1111
#define SCS_PKTMODE_MASK1			(0xF000)	// 1111 0000 0000 0000
#define SCS_PKTMODE_MASK2			(0x0F00)	// 0000 1111 0000 0000
#define SCS_PKTMODE_MASK3			(0x00FF)	// 0000 0000 1111 1111

extern bool SCSPacketModeValidate(SCSPacketMode value);

/* ---------------------------------------------------------------------------------------------- */

typedef int SCSPacketFlag;

#define SCS_PKTFLAG_NONE			(0x0000)	// 0000 0000 0000 0000

#define SCS_PKTFLAG_SYN				(0x0001)	// 0000 0000 0000 0001
#define SCS_PKTFLAG_ACK				(0x0002)	// 0000 0000 0000 0010
#define SCS_PKTFLAG_PLD				(0x0004)	// 0000 0000 0000 0100
#define SCS_PKTFLAG_FIN				(0x0008)	// 0000 0000 0000 1000
#define SCS_PKTFLAG_RST				(0x0010)	// 0000 0000 0001 0000
#define SCS_PKTFLAG_RED				(0x0020)	// 0000 0000 0010 0000

#define SCS_PKTFLAG_NTY				(0x0100)	// 0000 0001 0000 0000
#define SCS_PKTFLAG_PMD				(0x0200)	// 0000 0010 0000 0000
#define SCS_PKTFLAG_RTT				(0x0400)	// 0000 0100 0000 0000
#define SCS_PKTFLAG_VRF				(0x0800)	// 0000 1000 0000 0000
#define SCS_PKTFLAG_PAD				(0x1000)	// 0001 0000 0000 0000

#define SCS_PKTFLAG_CTL				(0x8000)	// 1000 0000 0000 0000

#define SCS_PKTFLAG_ALL ( \
		SCS_PKTFLAG_SYN | \
		SCS_PKTFLAG_ACK | \
		SCS_PKTFLAG_PLD | \
		SCS_PKTFLAG_FIN | \
		SCS_PKTFLAG_RST | \
		SCS_PKTFLAG_RED | \
		SCS_PKTFLAG_NTY | \
		SCS_PKTFLAG_PMD | \
		SCS_PKTFLAG_RTT | \
		SCS_PKTFLAG_VRF | \
		SCS_PKTFLAG_PAD | \
		SCS_PKTFLAG_CTL)

#define SCSPacketFlagValidate(xxx_value) \
		((xxx_value & ~SCS_PKTFLAG_ALL) == SCS_PKTFLAG_NONE)

/* ---------------------------------------------------------------------------------------------- */

typedef uint64_t SCSPacketSeqno;
typedef scs_atomic_uint64 SCSPacketSeqnoAtomic;

#define SCS_PKTSEQNO_MINVAL			(0)
#define SCS_PKTSEQNO_MAXVALLB		((uint64_t) UINT16_MAX)
#define SCS_PKTSEQNO_MAXVALSTD		((uint64_t) UINT32_MAX)
#define SCS_PKTSEQNO_MAXVALHB		UINT64_MAX

extern SCSPacketSeqno SCSPacketSeqnoIncrease( //
		SCSPacketSeqnoAtomic * self, //
		SCSPacketSeqno maximum);
extern SCSPacketSeqno SCSPacketSeqnoGetMax(SCSPacketMode value);

/* ---------------------------------------------------------------------------------------------- */

typedef uint32_t SCSPacketTimestamp;

#define SCS_PKTTS_INVVAL			(0x00000000)	// 0000 0000 0000 0000 0000 0000 0000 0000

/* ---------------------------------------------------------------------------------------------- */

typedef uint32_t SCSPathMTUDiscoveryId;

#define SCS_PKTPMTUDID_INVVAL		(0x00000000)
#define SCS_PKTPMTUDID_MINVAL		(0x00000001)
#define SCS_PKTPMTUDID_MAXVAL		(0xFFFFFFFF)

#define SCSPathMTUDiscoveryIdValidate(xxx_value) \
		(SCS_PKTPMTUDID_MINVAL <= xxx_value && xxx_value <= SCS_PKTPMTUDID_MAXVAL)

/* ---------------------------------------------------------------------------------------------- */

typedef uint8_t SCSRTTMeasFlag;

#define SCS_PKTRTTMFLAG_INVVAL		(0x00)	// 0000 0000
#define SCS_PKTRTTMFLAG_FIXVAL		(0x30)	// 0111 0000

#define SCS_PKTRTTMFLAG_NONE		(SCS_PKTVRFMETHOD_FIXVAL | 0x00)	// 0000 0000
#define SCS_PKTRTTMFLAG_REQUEST		(SCS_PKTVRFMETHOD_FIXVAL | 0x01)	// 0000 0001

#define SCS_PKTVRFMETHOD_MASKFULL	(0xFF)	// 1111 1111

extern bool SCSRTTMeasFlagValidate(SCSRTTMeasFlag value);

/* ---------------------------------------------------------------------------------------------- */

typedef uint16_t SCSRTTMeasId;

#define SCS_PKTRTTMID_INVVAL		(0x0000)
#define SCS_PKTRTTMID_MINVAL		(0x0001)
#define SCS_PKTRTTMID_MAXVAL		(0xFFFF)

#define SCSRTTMeasIdValidate(xxx_value) \
		(SCS_PKTRTTMID_MINVAL <= xxx_value && xxx_value <= SCS_PKTRTTMID_MAXVAL)

/* ---------------------------------------------------------------------------------------------- */

typedef uint8_t SCSPacketVerificationMethod;

#define SCS_PKTVRFMETHOD_INVVAL		(0x00)	// 0000 0000
#define SCS_PKTVRFMETHOD_FIXVAL		(0x70)	// 0111 0000

#define SCS_PKTVRFMETHOD_NONE		(SCS_PKTVRFMETHOD_FIXVAL | 0x00)	// 0000 0000
#define SCS_PKTVRFMETHOD_CRC16		(SCS_PKTVRFMETHOD_FIXVAL | 0x01)	// 0000 0001

#define SCS_PKTVRFMETHOD_MASKFULL	(0xFF)	// 1111 1111

extern bool SCSPacketVerificationMethodValidate(SCSPacketVerificationMethod value);

#define SCSPacketVerificationMethodToString(xxx_value) \
		(xxx_value == SCS_PKTVRFMETHOD_CRC16 ? "CRC16" : \
		 xxx_value == SCS_PKTVRFMETHOD_NONE ? "(None)" : \
		 "?")

/* ============================================================================================== */

#endif /* SCS_5_PACKET_TYPES_H_ */
