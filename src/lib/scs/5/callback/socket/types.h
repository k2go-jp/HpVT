#ifndef SCS_5_SOCKET_CALLBACK_TYPES_H_
#define SCS_5_SOCKET_CALLBACK_TYPES_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

/* ---------------------------------------------------------------------------------------------- */

typedef int SCSSocketCallbackType;

#define SCS_SKTCBTYPE_NONE						0
#define SCS_SKTCBTYPE_REDUNDANCY_CALLBACK		11
#define SCS_SKTCBTYPE_CONNECTION_CALLBACK		12
#define SCS_SKTCBTYPE_CLOSING_CALLBACK			13
#define SCS_SKTCBTYPE_FEEDBACK_CALLBACK			21

extern bool SCSSocketCallbackTypeValidate(SCSSocketCallbackType value);

/* ============================================================================================== */

#endif /* SCS_5_SOCKET_CALLBACK_TYPES_H_ */
