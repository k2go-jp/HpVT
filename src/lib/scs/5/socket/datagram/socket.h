#ifndef SCS_5_SOCKET_DATAGRAM_SOCKET_H_
#define SCS_5_SOCKET_DATAGRAM_SOCKET_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/2/atomic.h"
#include "scs/2/lock.h"
#include "scs/2/pointer.h"
#include "scs/5/callback/socket/feedback/config.h"
#include "scs/5/channel/channel.h"
#include "scs/5/feature/jitter.h"
#include "scs/5/feature/plr.h"
#include "scs/5/feature/pmtu.h"
#include "scs/5/feature/rtt.h"
#include "scs/5/feature/traffic/counter.h"
#include "scs/5/packet/queue.h"
#include "scs/5/packet/sequencer.h"
#include "scs/5/socket/datagram/settings.h"
#include "scs/5/socket/types.h"
#include "scs/types.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSDatagramSocket {
	SCSSocketType type; // Must be first

	scs_atomic_reference reference;

	scs_mutex mutex;
	SCSSocketState state;
	SCSSocketId id;

	SCSDatagramSocketSettings settings;
	SCSChannel channel;
	SCSNetworkTrafficCounter traffic;

	SCSConnectionId connid;
	struct {
		SCSPacketMode mode;
		struct {
			SCSPacketSeqno maximum;
			SCSPacketSeqno initial;
			SCSPacketSeqno next;
			SCSPacketSeqnoAtomic self;
			SCSPacketSeqnoAtomic peer;
		} seqno;
	} packet;
	struct {
		struct {
			uint8_t * ptr;
			size_t size;
		} hasty;
		struct {
			SCSPacket * syn;
			SCSPacket * synack;
		} packet;
		struct {
			scs_timespec syn;
			scs_timespec synack;
			scs_timespec ack;
		} timestamp;
	} connect;
	struct {
		scs_timespec timestamp;
		struct {
			SCSPacketQueue control;
			SCSPacketQueue retransmit;
			SCSPacketQueue transmit;
		} queue;
	} send;
	struct {
		scs_timespec timestamp;
		SCSPacketQueue queue;
		SCSPacketSequencer sequencer;
	} receive;
	struct {
		struct {
			struct {
				scs_timespec last;
				scs_timespec next;
			} timestamp;
			SCSFeedbackInfo feedback;
		} send;
		struct {
			scs_mutex mutex;
			scs_timespec timestamp;
			struct {
				SCSFeedbackInfo basic;
				struct {
					uint8_t * ptr;
					size_t size;
					size_t length;
				} extension;
			} feedback;
		} receive;
	} notification;
	struct {
		scs_timespec timestamp;
		scs_atomic_uint64 packets;
		scs_atomic_uint64 bytes;
		SCSPMTUMeasurer measurer;
	} pmtu;
	struct {
		struct {
			struct {
				scs_timespec request;
				scs_timespec response;
			} sent;
			struct {
				scs_timespec request;
				scs_timespec response;
			} receive;
		} timestamp;
		SCSRTTMeasurer measurer;
		SCSPacketWatcher watcher;
	} rtt;
	struct {
		SCSPLRMeasurer measurer;
		uint64_t sent;
	} plr;
	struct {
		SCSJitterMeasurer measurer;
	} jitter;
	struct {
		bool done;
		SCSPacket * packet;
		scs_timespec timeout;
	} close;

} SCSDatagramSocket;

/* ---------------------------------------------------------------------------------------------- */

extern void SCSDatagramSocketInitialize(SCSDatagramSocket * self);
extern void SCSDatagramSocketFinalize(SCSDatagramSocket * self);

/* ---------------------------------------------------------------------------------------------- */

extern SCSDatagramSocket * SCSDatagramSocketCreate(SCSProtocolType protocol);
extern void SCSDatagramSocketDestroy(SCSDatagramSocket * socket);

extern bool SCSDatagramSocketHold(SCSDatagramSocket * self);
extern void SCSDatagramSocketFree(SCSDatagramSocket * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSDatagramSocketStandBy( 															//
		SCSDatagramSocket * __restrict self, 													//
		SCSDatagramSocketSettings * __restrict settings);

extern bool SCSDatagramSocketBind( 																//
		SCSDatagramSocket * __restrict self, 													//
		scs_sockaddr * __restrict addr);

extern bool SCSDatagramSocketConnect( 															//
		SCSDatagramSocket * __restrict self, 													//
		scs_sockaddr * __restrict addr);

extern bool SCSDatagramSocketListen( 															//
		SCSDatagramSocket * __restrict self, 													//
		scs_sockaddr * __restrict addr);

extern bool SCSDatagramSocketAccept( 															//
		SCSDatagramSocket * __restrict self, 													//
		scs_sockaddr * __restrict addr);

extern bool SCSDatagramSocketEnqueue( 															//
		SCSDatagramSocket * __restrict self, 													//
		SCSPointer ptr, 																		//
		SCSSocketOption option, 																//
		int redundancy, 																		//
		SCSPacketSeqno * __restrict seqno);

extern bool SCSDatagramSocketDequeue( 															//
		SCSDatagramSocket * __restrict self, 													//
		SCSPointer * __restrict ptr, 															//
		SCSSocketOption option, 																//
		SCSPacketSeqno * __restrict seqno);

extern bool SCSDatagramSocketWiatToSendable(SCSDatagramSocket * self);

extern bool SCSDatagramSocketSend( 																//
		SCSDatagramSocket * __restrict self, 													//
		SCSNetworkTrafficCounter * __restrict statistics);

extern bool SCSDatagramSocketReceive( 															//
		SCSDatagramSocket * __restrict self, 													//
		int sd, 																				//
		SCSNetworkTrafficCounter * __restrict statistics);

extern void SCSDatagramSocketNotify(SCSDatagramSocket * self);

extern void SCSDatagramSocketTimeOut(SCSDatagramSocket * self);

extern void SCSDatagramSocketClear(SCSDatagramSocket * self, SCSSocketDirection direction);

extern void SCSDatagramSocketShutdown(SCSDatagramSocket * self, SCSSocketDirection direction);

extern void SCSDatagramSocketBroken(SCSDatagramSocket * self);

extern bool SCSDatagramSocketClose(SCSDatagramSocket * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSDatagramSocketEnablePathMTUDiscovery(SCSDatagramSocket * self);
extern bool SCSDatagramSocketDisablePathMTUDiscovery(SCSDatagramSocket * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSDatagramSocketSetId(SCSDatagramSocket * self, SCSSocketId id);
extern SCSSocketId SCSDatagramSocketGetId(SCSDatagramSocket * self);

extern scs_socket_desc SCSDatagramSocketGetSocketDescriptor(SCSDatagramSocket * self);

extern bool SCSDatagramSocketGetNetworkTrafficInfo(												//
		SCSDatagramSocket * __restrict self, 													//
		SCSNetworkTrafficInfo * __restrict out);

extern bool SCSDatagramSocketGetFeedbackInfo(													//
		SCSDatagramSocket * __restrict self, 													//
		SCSFeedbackInfo * __restrict out);

extern bool SCSDatagramSocketGetExtendedFeedbackInfo(											//
		SCSDatagramSocket * __restrict self, 													//
		uint8_t ** __restrict out_ptr,															//
		size_t * __restrict out_size);

extern uint32_t SCSDatagramSocketGetPLR(SCSDatagramSocket * self);

extern scs_timespec SCSDatagramSocketGetRTT(SCSDatagramSocket * self);

extern int SCSDatagramSocketGetMTU(SCSDatagramSocket * self);

extern int SCSDatagramSocketGetPathMTU(SCSDatagramSocket * self);

extern bool SCSDatagramSocketSetFeedbackInterval(												//
		SCSDatagramSocket * __restrict self,													//
		scs_timespec * __restrict interval);
extern scs_timespec SCSDatagramSocketGetFeedbackInterval(SCSDatagramSocket * self);

extern bool SCSDatagramSocketSetBandwidth(														//
		SCSDatagramSocket * __restrict self,													//
		uint64_t * __restrict bps);
extern uint64_t SCSDatagramSocketGetBandwidth(SCSDatagramSocket * self);

extern bool SCSDatagramSocketGetBufferStatus(													//
		SCSDatagramSocket * __restrict self,													//
		SCSSocketBufferStatus * __restrict out);

extern bool SCSDatagramSocketGetInfo(															//
		SCSDatagramSocket * __restrict self, 													//
		SCSSocketInfo * __restrict out);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSDatagramSocketSetHastyData(														//
		SCSDatagramSocket * __restrict self, 													//
		uint8_t * __restrict ptr,																//
		size_t size);

extern bool SCSDatagramSocketGetHastyData(														//
		SCSDatagramSocket * __restrict self, 													//
		uint8_t * __restrict ptr,																//
		size_t size);

extern size_t SCSDatagramSocketGetHastyDataSize(SCSDatagramSocket * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSDatagramSocketSetConnectionCallback(												//
		SCSDatagramSocket * __restrict self,													//
		SCSRedundancyCallbackConfig * __restrict config);

extern bool SCSDatagramSocketSetRedundancyCallback(												//
		SCSDatagramSocket * __restrict self,													//
		SCSRedundancyCallbackConfig * __restrict config);

extern bool SCSDatagramSocketSetFeedbackCallback(												//
		SCSDatagramSocket * __restrict self,													//
		SCSFeedbackCallbackConfig * __restrict config);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSDatagramSocketIsClosing(SCSDatagramSocket * self);
extern bool SCSDatagramSocketIsClosed(SCSDatagramSocket * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSDatagramSocketCheckType(SCSDatagramSocket * self, SCSSocketType type);

extern bool SCSDatagramSocketCheckSendable(SCSDatagramSocket * self);

extern bool SCSDatagramSocketCheckReceivable(SCSDatagramSocket * self);

extern bool SCSDatagramSocketCheckSelfAddr( 													//
		SCSDatagramSocket * __restrict self, 													//
		scs_sockaddr * __restrict addr);

extern bool SCSDatagramSocketCheckPeerAddr( 													//
		SCSDatagramSocket * __restrict self, 													//
		scs_sockaddr * __restrict addr);

extern bool SCSDatagramSocketCheckPayloadLength(SCSDatagramSocket * self, size_t length);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSDatagramSocketLoggingStatistics(SCSDatagramSocket * self, SCSLogType type);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSDatagramSocketMonitor(void);

/* ============================================================================================== */

#endif /* SCS_5_SOCKET_DATAGRAM_SOCKET_H_ */
