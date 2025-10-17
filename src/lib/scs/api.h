#ifndef SCS_API_H_
#define SCS_API_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/comatibility.h"
#include "scs/1/inttypes.h"
#include "scs/1/socket.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/callback/socket/feedback/config.h"
#include "scs/5/callback/socket/redundancy/config.h"
#include "scs/5/callback/socket/types.h"
#include "scs/5/callback/system/error/config.h"
#include "scs/5/callback/system/socket/config.h"
#include "scs/5/callback/system/system/config.h"
#include "scs/5/callback/system/types.h"
#include "scs/5/packet/types.h"
#include "scs/5/socket/types.h"
#include "scs/5/settings.h"
#include "scs/5/types.h"
#include "scs/types.h"

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool WINAPI SCSStart(SCSSettings * settings);
EXTERN void WINAPI SCSStop(void);

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool WINAPI SCSSetLogLevel(SCSLogType type, SCSLogLevel level);

EXTERN bool WINAPI SCSOpenLogFile(const char * path);

EXTERN void WINAPI SCSDumpLogger(void);

/* ---------------------------------------------------------------------------------------------- */

EXTERN void WINAPI SCSGetSettings(SCSSettings * settings);

EXTERN void WINAPI SCSDumpSettings(SCSSettings * settings);

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool WINAPI SCSAddCallback( 																//
		SCSCallbackType type,  																	//
		void * __restrict ptr,  																//
		size_t size, 																			//
		SCSCallbackId * __restrict out);
EXTERN bool WINAPI SCSRemoveCallback(SCSCallbackType type, SCSCallbackId id);

/* ---------------------------------------------------------------------------------------------- */

EXTERN SCSSocketId WINAPI SCSCreateSocket(SCSSocketType type, SCSProtocolType protocol);
EXTERN void WINAPI SCSDestroySocket(SCSSocketId id);

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool WINAPI SCSBind(SCSSocketId id, scs_sockaddr addr);

EXTERN bool WINAPI SCSConnect(SCSSocketId id, scs_sockaddr addr);

EXTERN bool WINAPI SCSListen(SCSSocketId id, scs_sockaddr addr);

EXTERN bool WINAPI SCSAccept(SCSSocketId id, scs_sockaddr * addr);

EXTERN void WINAPI SCSShutdown(SCSSocketId id, SCSSocketDirection option);

/* ---------------------------------------------------------------------------------------------- */

EXTERN void WINAPI SCSClear(SCSSocketId id, SCSSocketDirection option);

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool WINAPI SCSGetSocketInfo(SCSSocketId id, SCSSocketInfo * out);

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool WINAPI SCSSetSocketOption(															//
		SCSSocketId id, 																		//
		SCSSocketOptionName name, 																//
		void * ptr, 																			//
		size_t size);
EXTERN bool WINAPI SCSGetSocketOption(															//
		SCSSocketId id, 																		//
		SCSSocketOptionName name, 																//
		void * ptr, 																			//
		size_t size);

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool WINAPI SCSSetSocketCallback(														//
		SCSSocketId id, 																		//
		SCSSocketCallbackType type,  															//
		void * __restrict ptr,  																//
		size_t size);

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool WINAPI DGSSend( 																	//
		SCSSocketId id, 																		//
		void * __restrict ptr, 																	//
		size_t length, 																			//
		size_t offset, 																			//
		SCSSocketOption option, 																//
		int redundancy, 																		//
		SCSPacketSeqno * __restrict seqno);
EXTERN bool WINAPI DGSRecv(																		//
		SCSSocketId id, 																		//
		void * __restrict ptr, 																	//
		size_t length, 																			//
		size_t * __restrict offset, 															//
		SCSSocketOption option, 																//
		SCSPacketSeqno * __restrict seqno);

/* ============================================================================================== */

#endif /* SCS_API_H_ */
