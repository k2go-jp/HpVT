#define SCS_SOURCECODE_FILEID	"5PKTTYP"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/packet/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

inline SCSPacketSeqno SCSPacketSeqnoGetMax(SCSPacketMode value) {

	switch ((value & SCS_PKTMODE_MASK2)) {
		case SCS_PKTMODE_LOWBAND:
			return SCS_PKTSEQNO_MAXVALLB;
		case SCS_PKTMODE_STANDARD:
			return SCS_PKTSEQNO_MAXVALSTD;
		case SCS_PKTMODE_HIGHBAND:
			return SCS_PKTSEQNO_MAXVALHB;
		default: {
			SCS_LOG(ALERT, SYSTEM, 99997, "<<%d>>", value);
			return SCS_PKTSEQNO_MINVAL;
		}
	}

}

inline SCSPacketSeqno SCSPacketSeqnoIncrease( //
		SCSPacketSeqnoAtomic * self, //
		SCSPacketSeqno maximum) {
	SCSPacketSeqno tmp_seqno1;
	SCSPacketSeqno tmp_seqno2;

	do {
		tmp_seqno1 = SCSAtomicIncrease(*self);

		if (tmp_seqno1 <= maximum) {
			return tmp_seqno1;
		}
		else {
			tmp_seqno2 = tmp_seqno1 % maximum;
		}
	} while (SCSAtomicCompSet(*self, tmp_seqno1, tmp_seqno2) == false);

	return tmp_seqno2;
}

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSPacketModeValidate(SCSPacketMode value) {

	switch ((value & SCS_PKTMODE_MASK1)) {
		case SCS_PKTMODE_V1:
			break;
		default:
			return false;
	}

	switch ((value & SCS_PKTMODE_MASK2)) {
		case SCS_PKTMODE_LOWBAND:
		case SCS_PKTMODE_STANDARD:
		case SCS_PKTMODE_HIGHBAND:
			break;
		default:
			return false;
	}

	switch ((value & SCS_PKTMODE_MASK3)) {
		case SCS_PKTMODE_DATACELL:
		case SCS_PKTMODE_DATAGRAM:
		case SCS_PKTMODE_STREAM:
			break;
		default:
			return false;
	}

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSRTTMeasFlagValidate(SCSRTTMeasFlag value) {

	switch (value) {
		case SCS_PKTRTTMFLAG_NONE:
		case SCS_PKTRTTMFLAG_REQUEST:
			return true;
		default:
			return false;
	}

}

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSPacketVerificationMethodValidate(SCSPacketVerificationMethod value) {

	switch (value) {
		case SCS_PKTVRFMETHOD_NONE:
		case SCS_PKTVRFMETHOD_CRC16:
			return true;
		default:
			return false;
	}

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
