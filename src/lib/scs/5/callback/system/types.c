#define SCS_SOURCECODE_FILEID	"5CBKTYP"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/callback/system/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

bool SCSCallbackTypeValidate(SCSCallbackType value) {

	switch (value) {
		case SCS_CALLBACKTYPE_ERROR:
		case SCS_CALLBACKTYPE_SOCKET:
		case SCS_CALLBACKTYPE_SYSTEM:
			return true;
		default:
			return false;
	}

}

const char * SCSCallbackTypeToString(SCSCallbackType value) {

	switch (value) {
		case SCS_CALLBACKTYPE_ERROR:
			return "ErrorCallback";
		case SCS_CALLBACKTYPE_SOCKET:
			return "SocketCallback";
		case SCS_CALLBACKTYPE_SYSTEM:
			return "SystemCallback";
		default:
			return "?";
	}

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
