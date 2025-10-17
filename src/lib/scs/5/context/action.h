#ifndef SCS_5_CONTEXT_ACTION_H_
#define SCS_5_CONTEXT_ACTION_H_ 1

/* ============================================================================================== */

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/5/socket/socket.h"

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSSenderWakeUp(SCSSocket * __restrict socket, scs_time timeout_ms);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSCloserWakeUp(SCSSocket * __restrict socket, scs_time timeout_ms);

/* ============================================================================================== */

#endif /* SCS_5_CONTEXT_ACTION_H_ */
