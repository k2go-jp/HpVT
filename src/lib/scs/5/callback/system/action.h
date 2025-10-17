#ifndef SCS_5_CALLBACK_ACTION_H_
#define SCS_5_CALLBACK_ACTION_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/5/callback/system/types.h"

/* ---------------------------------------------------------------------------------------------- */

extern void SCSCallbackConfigFinalize(SCSCallbackType type, void * ptr, size_t size);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSCallbackConfigValidate(SCSCallbackType type, void * ptr, size_t size);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSCallCallbackFunction( 															//
		SCSCallbackType type,  																	//
		void * __restrict ptr,  																//
		size_t size,  																			//
		SCSCallbackId id,  																		//
		scs_time * __restrict interval);

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_ACTION_H_ */
