#define SCS_SOURCECODE_FILEID	"5SKTCBKTYP"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/callback/socket/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSSocketCallbackTypeValidate(SCSSocketCallbackType value) {

	switch (value) {
		//case SCS_SKTCBNAME_NONE:
		case SCS_SKTCBTYPE_CLOSING_CALLBACK:
		case SCS_SKTCBTYPE_CONNECTION_CALLBACK:
		case SCS_SKTCBTYPE_FEEDBACK_CALLBACK:
		case SCS_SKTCBTYPE_REDUNDANCY_CALLBACK:
			return true;
		default:
			return false;
	}

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
