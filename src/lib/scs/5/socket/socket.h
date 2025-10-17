#ifndef SCS_5_SOCKET_SOCKET_H_
#define SCS_5_SOCKET_SOCKET_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/2/logger.h"
#include "scs/5/callback/socket/feedback/types.h"
#include "scs/5/callback/socket/redundancy/types.h"
#include "scs/5/feature/traffic/counter.h"
#include "scs/5/feedback.h"
#include "scs/5/socket/datagram/socket.h"
#include "scs/5/socket/settings.h"
#include "scs/5/types.h"
#include "scs/types.h"

/* ---------------------------------------------------------------------------------------------- */

typedef union SCSSocket {
	SCSSocketType type;
	SCSDatagramSocket datagram;
} SCSSocket;

/* ---------------------------------------------------------------------------------------------- */

extern SCSSocket * SCSSocketCreate(SCSSocketType type, SCSProtocolType protocol);
extern void SCSSocketDestroy(SCSSocket * socket);

extern bool SCSSocketHold(SCSSocket * self);
extern void SCSSocketFree(SCSSocket * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSSocketStandBy( 																	//
		SCSSocket * __restrict self, 															//
		SCSSocketSettings * __restrict settings);

extern bool SCSSocketBind(SCSSocket * __restrict self, scs_sockaddr * __restrict addr);

extern bool SCSSocketConnect(SCSSocket * __restrict self, scs_sockaddr * __restrict addr);

extern bool SCSSocketListen(SCSSocket * __restrict self, scs_sockaddr * __restrict addr);

extern bool SCSSocketAccept(SCSSocket * __restrict self, scs_sockaddr * __restrict addr);

extern bool SCSSocketSend( 																		//
		SCSSocket * __restrict self, 															//
		SCSNetworkTrafficCounter * __restrict traffic);

extern bool SCSSocketReceive( 																	//
		SCSSocket * __restrict self, 															//
		int sd, 																				//
		SCSNetworkTrafficCounter * __restrict traffic);

extern void SCSSocketNotify(SCSSocket * self);

extern void SCSSocketTimeOut(SCSSocket * self);

extern void SCSSocketClear(SCSSocket * self, SCSSocketDirection direction);

extern void SCSSocketShutdown(SCSSocket * self, SCSSocketDirection direction);

extern void SCSSocketBroken(SCSSocket * self);

extern bool SCSSocketClose(SCSSocket * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSSocketEnablePathMTUDiscovery(SCSSocket * self);
extern bool SCSSocketDisablePathMTUDiscovery(SCSSocket * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSSocketSetId(SCSSocket * self, SCSSocketId id);
extern SCSSocketId SCSSocketGetId(SCSSocket * self);

extern scs_socket_desc SCSSocketGetSocketDescriptor(SCSSocket * self);

extern bool SCSSocketGetNetworkTrafficInfo(														//
		SCSSocket * __restrict self, 															//
		SCSNetworkTrafficInfo * __restrict out);

extern bool SCSSocketGetFeedbackInfo(															//
		SCSSocket * __restrict self, 															//
		SCSFeedbackInfo * __restrict out);

extern bool SCSSocketGetExtendedFeedbackInfo(													//
		SCSSocket * __restrict self, 															//
		uint8_t ** __restrict out_ptr,  														//
		size_t * __restrict out_size);

extern uint32_t SCSSocketGetPLR(SCSSocket * self);

extern scs_timespec SCSSocketGetRTT(SCSSocket * self);

extern int SCSSocketGetMTU(SCSSocket * self);

extern int SCSSocketGetPathMTU(SCSSocket * self);

extern bool SCSSocketSetFeedbackInterval(														//
		SCSSocket * __restrict self,															//
		scs_timespec * __restrict interval);
extern scs_timespec SCSSocketGetFeedbackInterval(SCSSocket * self);

extern bool SCSSocketSetBandwidth(																//
		SCSSocket * __restrict self,															//
		uint64_t * __restrict bps);
extern uint64_t SCSSocketGetBandwidth(SCSSocket * self);

extern bool SCSSocketGetBufferStatus(															//
		SCSSocket * __restrict self, 															//
		SCSSocketBufferStatus * __restrict out);

extern bool SCSSocketGetInfo(SCSSocket * __restrict self, SCSSocketInfo * __restrict out);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSSocketSetHastyData(																//
		SCSSocket * __restrict self,															//
		uint8_t * __restrict ptr,																//
		size_t size);
extern bool SCSSocketGetHastyData(																//
		SCSSocket * __restrict self,															//
		uint8_t * __restrict ptr,																//
		size_t size);
extern size_t SCSSocketGetHastyDataSize(SCSSocket * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSSocketSetConnectionCallback(														//
		SCSSocket * __restrict self,															//
		SCSRedundancyCallbackConfig * __restrict config);

extern bool SCSSocketSetRedundancyCallback(														//
		SCSSocket * __restrict self,															//
		SCSRedundancyCallbackConfig * __restrict config);

extern bool SCSSocketSetFeedbackCallback(														//
		SCSSocket * __restrict self,															//
		SCSFeedbackCallbackConfig * __restrict config);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSSocketIsClosing(SCSSocket * __restrict self);
extern bool SCSSocketIsClosed(SCSSocket * __restrict self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSSocketLoggingStatistics(SCSSocket * self, SCSLogType type);

/* ============================================================================================== */

#endif /* SCS_5_SOCKET_SOCKET_H_ */
