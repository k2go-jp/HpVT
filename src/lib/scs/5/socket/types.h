#ifndef SCS_5_SOCKET_TYPES_H_
#define SCS_5_SOCKET_TYPES_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"

/* ---------------------------------------------------------------------------------------------- */

typedef int SCSSocketType;

#define SCS_SKTTYPE_NONE			0
#define SCS_SKTTYPE_DATAGRAM 		1
#define SCS_SKTTYPE_STREAM			3

extern bool SCSSocketTypeValidate(SCSSocketType value);

/* ---------------------------------------------------------------------------------------------- */

typedef int SCSSocketDirection;

#define SCS_SKTDRCTN_BOTH		0
#define SCS_SKTDRCTN_SEND		1
#define SCS_SKTDRCTN_RECV		2

extern bool SCSSocketDirectionValidate(SCSSocketDirection value);

/* ---------------------------------------------------------------------------------------------- */

typedef int SCSSocketState;

#define SCS_SKTSTATE_INITIALIZED	0
#define SCS_SKTSTATE_STANDBY		10
#define SCS_SKTSTATE_BIND			11
#define SCS_SKTSTATE_ACCEPT			12
#define SCS_SKTSTATE_CONNECTED		20
#define SCS_SKTSTATE_SENDCLOSED		21
#define SCS_SKTSTATE_RECVCLOSED		22
#define SCS_SKTSTATE_BROKEN			30
#define SCS_SKTSTATE_CLOSED			40

/* ---------------------------------------------------------------------------------------------- */

typedef int SCSSocketOptionName;

#define SCS_SKTOPTNAME_INVVAL				0
#define SCS_SKTOPTNAME_FEEDBACK_INTERVAL	1
#define SCS_SKTOPTNAME_FEEDBACK_DISABLE		2
#define SCS_SKTOPTNAME_BANDWIDTH			3
#define SCS_SKTOPTNAME_PATHMTUD_ENABLE		4
#define SCS_SKTOPTNAME_PATHMTUD_DISABLE		5
#define SCS_SKTOPTNAME_HASTYDATA			6
#define SCS_SKTOPTNAME_HASTYDATASIZE		7
//#define SCS_SKTOPTNAME_
//#define SCS_SKTOPTNAME_

extern bool SCSSocketOptionNameValidate(SCSSocketOptionName value);

/* ============================================================================================== */

#endif /* SCS_5_SOCKET_TYPES_H_ */
