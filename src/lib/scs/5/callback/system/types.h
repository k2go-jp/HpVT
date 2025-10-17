#ifndef SCS_5_CALLBACK_TYPES_H_
#define SCS_5_CALLBACK_TYPES_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"

/* ---------------------------------------------------------------------------------------------- */

#define SCS_CALLBACKLIST_MINENTRIES (1)
#define SCS_CALLBACKLIST_MAXENTRIES (1024)

#if (SCS_CALLBACKLIST_MAXENTRIES - 1) < 1
#error
#endif

/* ---------------------------------------------------------------------------------------------- */

typedef size_t SCSCallbackId;

#define SCS_CALLBACKID_INVVAL		(0)
#define SCS_CALLBACKID_MINVAL		(1)
#define SCS_CALLBACKID_MAXVAL		(SCS_CALLBACKLIST_MAXENTRIES - 1)

#define SCSCallbackIdValidate(value) \
		((SCS_CALLBACKID_MINVAL <= value) && (value <= SCS_CALLBACKID_MAXVAL))

/* ---------------------------------------------------------------------------------------------- */

typedef int SCSCallbackType;

#define SCS_CALLBACKTYPE_NONE		(0)

#define SCS_CALLBACKTYPE_ERROR		(1)
#define SCS_CALLBACKTYPE_SYSTEM		(2)
#define SCS_CALLBACKTYPE_SOCKET		(3)

extern bool SCSCallbackTypeValidate(SCSCallbackType value);
extern const char * SCSCallbackTypeToString(SCSCallbackType value);

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_TYPES_H_ */
