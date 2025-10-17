#ifndef SCS_5_CHANNEL_CHANNEL_H_
#define SCS_5_CHANNEL_CHANNEL_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/socket.h"
#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/5/channel/types.h"
#include "scs/5/packet/types.h"

/* ---------------------------------------------------------------------------------------------- */

extern void SCSChannelInitialize(SCSChannel * self, SCSChannelConfig config);
extern void SCSChannelFinalize(SCSChannel * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSChannelOpenAsClient(SCSChannel * __restrict self, scs_sockaddr * __restrict addr);
extern bool SCSChannelOpenAsServer(SCSChannel * __restrict self, scs_sockaddr * __restrict addr);

extern bool SCSChannelConnect(SCSChannel * __restrict self, scs_sockaddr * __restrict addr);
extern bool SCSChannelConnected(SCSChannel * self);

extern bool SCSChannelAccept1(SCSChannel * self);
extern bool SCSChannelAccept2(SCSChannel * __restrict self, scs_sockaddr * __restrict addr);
extern bool SCSChannelAccepted(SCSChannel * self);

extern void SCSChannelSend(SCSChannel * self);

extern void SCSChannelReceive(SCSChannel * self);

extern void SCSChannelBroken(SCSChannel * self);

extern void SCSChannelShutdown(SCSChannel * self, SCSChannelState state);

extern void SCSChannelClose(SCSChannel * self);

/* ---------------------------------------------------------------------------------------------- */

extern const char * SCSChannelGetProfile(SCSChannel * self);

extern void SCSChannelGetIfname(																//
		SCSChannel * __restrict self,															//
		char * __restrict out_ptr,																//
		size_t out_size);

extern void SCSChannelGetSelfAddr(SCSChannel * __restrict self, scs_sockaddr * __restrict out);

extern void SCSChannelGetPeerAddr(SCSChannel * __restrict self, scs_sockaddr * __restrict out);

extern SCSPacketMode SCSChannelGetPacketMode(SCSChannel * self, SCSPacketMode initval);

extern scs_socket_desc SCSChannelGetSocketDescriptor(SCSChannel * self);

extern int SCSChannelGetMTU(SCSChannel * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSChannelCheckSendable(SCSChannel * self);

extern bool SCSChannelCheckReceivable(SCSChannel * self);

extern bool SCSChannelCheckSelfAddr(SCSChannel * __restrict self, scs_sockaddr * __restrict addr);

extern bool SCSChannelCheckPeerAddr(SCSChannel * __restrict self, scs_sockaddr * __restrict addr);

extern bool SCSChannelCheckPayloadLength(SCSChannel * self, size_t length);

extern bool SCSChannelCheckSocketDescripotr(SCSChannel * self, scs_socket_desc sd);

extern bool SCSChannelCheckState(SCSChannel * self, SCSChannelState state, SCSChannelState mask);

/* ---------------------------------------------------------------------------------------------- */

#define SCSChannelIsServer(xxx_self) \
		((SCSAtomicGet((xxx_self)->config) & SCS_CHNLCNFG_SERVER) ? true : false)

#define SCSChannelIsClient(xxx_self) \
		((SCSAtomicGet((xxx_self)->config) & SCS_CHNLCNFG_CLIENT) ? true : false)

/* ---------------------------------------------------------------------------------------------- */

#define SCSChannelIsAvailable(xxx_self) \
		(SCSChannelCheckSendable(xxx_self) || SCSChannelCheckReceivable(xxx_self))

#define SCSChannelIsListening(xxx_self) \
		SCSChannelCheckState(xxx_self, (SCS_CHNLSTATE_LISTEN | SCS_CHNLSTATE_CONNECTING), SCS_CHNLSTATE_MASKFULL)

#define SCSChannelIsConnected(xxx_self) \
		SCSChannelCheckState(xxx_self, SCS_CHNLSTATE_CONNECTED, SCS_CHNLSTATE_CONNECTED)

#define SCSChannelIsBroken(xxx_self) \
		SCSChannelCheckState(xxx_self, SCS_CHNLSTATE_BROKEN, SCS_CHNLSTATE_BROKEN)

#define SCSChannelIsClosed(xxx_self) \
		SCSChannelCheckState(xxx_self, SCS_CHNLSTATE_CLOSED, SCS_CHNLSTATE_CLOSED)

/* ============================================================================================== */

#endif /* SCS_5_CHANNEL_CHANNEL_H_ */
