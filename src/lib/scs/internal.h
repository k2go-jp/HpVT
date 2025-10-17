#ifndef SCS_INTERNAL_H_
#define SCS_INTERNAL_H_ 1

/* ============================================================================================== */

#include <stdbool.h>

#include "scs/5/settings.h"
#include "scs/5/context/context.h"

/* ---------------------------------------------------------------------------------------------- */

extern bool _SCSStartUp(SCSSettings * settings);
extern void _SCSCleanUp(void);

extern SCSContext * _SCSGetContext(void);

/* ============================================================================================== */

#endif /* SCS_INTERNAL_H_ */
