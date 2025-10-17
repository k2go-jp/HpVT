#define SCS_SOURCECODE_FILEID	"5SCKTYP"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/socket/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSSocketTypeValidate(SCSSocketType value) {

	switch (value) {
		//case SCS_SKTTYPE_NONE:
		case SCS_SKTTYPE_DATAGRAM:
		case SCS_SKTTYPE_STREAM:
			return true;
		default:
			return false;
	}

}

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSSocketDirectionValidate(SCSSocketDirection value) {

	switch (value) {
		case SCS_SKTDRCTN_BOTH:
		case SCS_SKTDRCTN_RECV:
		case SCS_SKTDRCTN_SEND:
			return true;
		default:
			return false;
	}

}

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSSocketOptionNameValidate(SCSSocketOptionName value) {

	switch (value) {
		case SCS_SKTOPTNAME_BANDWIDTH:
		case SCS_SKTOPTNAME_FEEDBACK_DISABLE:
		case SCS_SKTOPTNAME_FEEDBACK_INTERVAL:
		case SCS_SKTOPTNAME_HASTYDATA:
		case SCS_SKTOPTNAME_HASTYDATASIZE:
		//case SCS_SKTOPTNAME_INVVAL:
		case SCS_SKTOPTNAME_PATHMTUD_DISABLE:
		case SCS_SKTOPTNAME_PATHMTUD_ENABLE:
		//case SCS_SKTOPTNAME_:
		return true;
	default:
		return false;
	}

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
