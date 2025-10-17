#define SCS_SOURCECODE_FILEID	"5CHNTYP"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/channel/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSChannelConfigValidate(SCSChannelConfig value) {

	if (value != SCS_CHNLSTATE_NONE) {
		switch (value & SCS_CHNLCNFG_MASK1) {
			case SCS_CHNLCNFG_NONE:
			case SCS_CHNLCNFG_IP:
			case SCS_CHNLCNFG_UDP:
				break;
			default:
				return false;
		}

		switch (value & SCS_CHNLCNFG_MASK2) {
			case SCS_CHNLCNFG_NONE:
			case SCS_CHNLCNFG_IPv4:
			case SCS_CHNLCNFG_IPv6:
				break;
			default:
				return false;
		}

		switch (value & SCS_CHNLCNFG_MASK3) {
			case SCS_CHNLCNFG_NONE:
			case SCS_CHNLCNFG_CLIENT:
			case SCS_CHNLCNFG_SERVER:
				break;
			default:
				return false;
		}

		switch (value & SCS_CHNLCNFG_MASK4) {
			case SCS_CHNLCNFG_NONE:
			case SCS_CHNLCNFG_LOWBAND:
			case SCS_CHNLCNFG_STANDARD:
			case SCS_CHNLCNFG_HIGHBAND:
				break;
			default:
				return false;
		}
	}

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSChannelStateValidate(SCSChannelState value) {

	return (value & ~SCS_CHNLSTATE_ALL) ? false : true;

}

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSConnectionIdValidate(SCSConnectionId value) {

	if (value == SCS_CONNID_INVVAL) {
		return false;
	}

//	if (value < SCS_CONNID_MINVAL) {
//		return false;
//	}

//	if (SCS_CONNID_MAXVAL < value) {
//		return false;
//	}

	return true;
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
