#define SCS_SOURCECODE_FILEID	"5DGMSKT"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/1/time.h"
#include "scs/1/unistd.h"
#include "scs/2/logger.h"
#include "scs/3/counter.h"
#include "scs/4/gc.h"
#include "scs/5/defines.h"
#include "scs/5/callback/socket/feedback/config.h"
#include "scs/5/callback/socket/redundancy/config.h"
#include "scs/5/channel/types.h"
#include "scs/5/context/action.h"
#include "scs/5/feature/rtt.h"
#include "scs/5/packet/builder.h"
#include "scs/5/packet/packet.h"
#include "scs/5/packet/parser.h"
#include "scs/5/packet/queue.h"
#include "scs/5/packet/sequencer.h"
#include "scs/5/packet/types.h"
#include "scs/5/packet/utils.h"
#include "scs/5/socket/datagram/socket.h"
#include "scs/5/socket/socket.h"
#include "scs/5/socket/types.h"
#include "scs/5/socket/utils.h"
#include "scs/5/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_LOCK
#error
#endif
#ifdef _SCS_UNLOCK
#error
#endif

#ifdef _SCS_LOCK_NOTIFICATION
#error
#endif
#ifdef _SCS_UNLOCK_NOTIFICATION
#error
#endif

#ifdef _SCS_MEMDUP
#error
#endif

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_LOCK(xxx_self) \
	if (SCSMutexLock((xxx_self)->mutex) != 0) {\
		SCS_LOG(ALERT, SYSTEM, 99999, "");\
		abort();\
	}
#define _SCS_UNLOCK(xxx_self) \
	if (SCSMutexUnlock((xxx_self)->mutex) != 0) {\
		SCS_LOG(ALERT, SYSTEM, 99999, "");\
		abort();\
	}

#define _SCS_LOCK_NOTIFICATION(xxx_self) \
	if (SCSMutexLock((xxx_self)->notification.receive.mutex) != 0) {\
		SCS_LOG(ALERT, SOCKET, 99999, "");\
		abort();\
	}
#define _SCS_UNLOCK_NOTIFICATION(xxx_self) \
	if (SCSMutexUnlock((xxx_self)->notification.receive.mutex) != 0) {\
		SCS_LOG(ALERT, SOCKET, 99999, "");\
		abort();\
	}

#define _SCS_MEMDUP(xxx_srcptr, xxx_size, xxx_dstptr) \
		if ((xxx_dstptr = malloc(xxx_size)) != NULL) {\
			memcpy(xxx_dstptr, xxx_srcptr, xxx_size);\
		}\
		else { \
			SCS_LOG(ALERT, SYSTEM, 00002, "<<%"PRIuS">>", xxx_size);\
			abort();\
		}

/* ---------------------------------------------------------------------------------------------- */

static SCSObjectCounter _counter = SCSObjectCounterInitializer;

/* ---------------------------------------------------------------------------------------------- */

#define _SCSDatagramSocketGetPacketMode(xxx_self) \
		(SCSChannelGetPacketMode(&(xxx_self)->channel, SCS_PKTMODE_V1) | SCS_PKTMODE_DATAGRAM)

#define _SCSDatagramSocketBroken(xxx_self) \
		SCSChannelBroken(&(xxx_self)->channel);\
		(xxx_self)->state = SCS_SKTSTATE_BROKEN

/* ---------------------------------------------------------------------------------------------- */

#define _SCSDatagramSocketHold(xxx_self) \
		SCSAtomicReferenceIncrease((xxx_self)->reference)

static inline void _SCSDatagramSocketFree(SCSDatagramSocket * self) {

	if (SCSAtomicGet(self->reference) == 0) {
		SCS_LOG(FATAL, SYSTEM, 99999, "");
		abort();
	}

	SCSAtomicReferenceDecrease(self->reference);

}

/* ---------------------------------------------------------------------------------------------- */

static bool _SCSDatagramSocketCheckSendable(SCSDatagramSocket * self) {
	bool tmp_retval;

	switch (self->state) {
		case SCS_SKTSTATE_ACCEPT: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_BIND: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_BROKEN: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_CLOSED: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_CONNECTED: {
			tmp_retval = SCSChannelCheckSendable(&self->channel);
			break;
		}
		case SCS_SKTSTATE_INITIALIZED: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_RECVCLOSED: {
			tmp_retval = SCSChannelCheckSendable(&self->channel);
			break;
		}
		case SCS_SKTSTATE_SENDCLOSED: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_STANDBY: {
			tmp_retval = false;
			break;
		}
		default: {
			SCS_LOG(ERROR, SOCKET, 99999, "<<%d>>", self->state);
			tmp_retval = false;
			break;
		}
	}

	return tmp_retval;
}

static bool _SCSDatagramSocketCheckReceivable(SCSDatagramSocket * self) {
	bool tmp_retval;

	switch (self->state) {
		case SCS_SKTSTATE_ACCEPT: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_BIND: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_BROKEN: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_CLOSED: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_CONNECTED: {
			tmp_retval = SCSChannelCheckReceivable(&self->channel);
			break;
		}
		case SCS_SKTSTATE_INITIALIZED: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_RECVCLOSED: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_SENDCLOSED: {
			tmp_retval = SCSChannelCheckReceivable(&self->channel);
			break;
		}
		case SCS_SKTSTATE_STANDBY: {
			tmp_retval = false;
			break;
		}
		default: {
			SCS_LOG(ERROR, SOCKET, 99999, "<<%d>>", self->state);
			tmp_retval = false;
			break;
		}
	}

	return tmp_retval;
}

static bool _SCSDatagramSocketCheckAvailable(SCSDatagramSocket * self) {
	bool tmp_retval;

	switch (self->state) {
		case SCS_SKTSTATE_ACCEPT: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_BIND: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_BROKEN: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_CLOSED: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_CONNECTED: {
			tmp_retval = SCSChannelIsAvailable(&self->channel);
			break;
		}
		case SCS_SKTSTATE_INITIALIZED: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_RECVCLOSED: {
			tmp_retval = SCSChannelIsAvailable(&self->channel);
			break;
		}
		case SCS_SKTSTATE_SENDCLOSED: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTSTATE_STANDBY: {
			tmp_retval = false;
			break;
		}
		default: {
			SCS_LOG(ERROR, SOCKET, 99999, "<<%d>>", self->state);
			tmp_retval = false;
			break;
		}
	}

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

static bool _SCSDatagramSocketEnqueueInternally(SCSDatagramSocket * __restrict self,
		scs_time timeout) {
	bool tmp_retval;
	scs_time tmp_counter;
	scs_time tmp_elapsed;

	if (_SCSDatagramSocketHold(self) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	tmp_retval = true;
	tmp_counter = 0;
	tmp_elapsed = 0;

	_SCS_UNLOCK(self);

	while (SCSSenderWakeUp((SCSSocket *) self, 1000) == false) {
		if (10 <= ++tmp_counter) {
			tmp_counter = 0;
			tmp_elapsed++;
		}
		else {
			continue;
		}

		if (!SCSChannelCheckState(&self->channel, SCS_CHNLSTATE_NONE,
				(SCS_CHNLSTATE_BROKEN | SCS_CHNLSTATE_CLOSED))) {
			SCS_LOG(WARN, SOCKET, 79981, "<<%s>>", SCSChannelGetProfile(&self->channel));
			tmp_retval = false;
			break;
		}

		if (0 < timeout) {
			if (timeout < tmp_elapsed) {
				SCS_LOG(WARN, SOCKET, 79982, "<<%s>>", SCSChannelGetProfile(&self->channel));
				tmp_retval = false;
				break;
			}
		}
	}

	_SCS_LOCK(self);

	_SCSDatagramSocketFree(self);

	return tmp_retval;
}

static bool _SCSDatagramSocketSendto(SCSDatagramSocket * __restrict self,
		SCSPointer * __restrict ptr, scs_time timeout) {
	bool tmp_retval;
	scs_socket_desc tmp_sd;
	scs_sockaddr tmp_peer;
	scs_time tmp_elapsed;
	ssize_t tmp_sndlen;

	if (_SCSDatagramSocketHold(self) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	tmp_retval = true;
	tmp_sd = SCSChannelGetSocketDescriptor(&self->channel);
	SCSChannelGetPeerAddr(&self->channel, &tmp_peer);
	tmp_elapsed = 0;
	tmp_sndlen = 0;

	//SCS_PRINT_DEBUG("sendto:: %zd bytes", length);

	_SCS_UNLOCK(self);

	for (;;) {
		//`sendto` timed out after 1 second.
		tmp_sndlen = sendto(tmp_sd, //
				SCSPointerGetReadPointer(*ptr), SCSPointerGetReadableLength(*ptr), //
				0, (struct sockaddr *) &tmp_peer, sizeof(tmp_peer));

		if (tmp_sndlen < 1) {
			if (tmp_sndlen < 0) {
				scs_errno tmp_errno;

				SCSGetLastError(tmp_errno);

				if ((tmp_errno != EAGAIN) && (tmp_errno != EWOULDBLOCK) && (tmp_errno != EINTR)) {
					char tmp_message[(SCS_EMSGSIZE_MAX + 1)];

					SCSGetErrorMessage(tmp_errno, tmp_message, SCS_EMSGSIZE_MAX);
					SCS_LOG(WARN, SOCKET, 00000, "%s. <<%s,%d,%zu>>", tmp_message,
							SCSChannelGetProfile(&self->channel), tmp_errno,
							SCSPointerGetReadableLength(*ptr));

					if (tmp_errno != EMSGSIZE) {
						_SCSDatagramSocketBroken(self);
					}

					tmp_retval = false;
					break;
				}
			}

			if (!SCSChannelCheckState(&self->channel, SCS_CHNLSTATE_NONE,
					(SCS_CHNLSTATE_BROKEN | SCS_CHNLSTATE_CLOSED))) {
				SCS_LOG(WARN, SOCKET, 79981, "<<%s>>", SCSChannelGetProfile(&self->channel));
				tmp_retval = false;
				break;
			}

			if (0 < timeout) {
				if (timeout < ++tmp_elapsed) {
					SCS_LOG(WARN, SOCKET, 79982, "<<%s>>", SCSChannelGetProfile(&self->channel));
					tmp_retval = false;
					break;
				}
			}

			continue;
		}

		break;
	}

	_SCS_LOCK(self);

	if (0 < tmp_sndlen) {
		//SCS_PRINT_DEBUG("sendto:: %zd bytes", tmp_sndlen);
		SCSTimespecSetCurrentTime(self->send.timestamp, CLOCK_MONOTONIC);
	}

	_SCSDatagramSocketFree(self);

	return tmp_retval;
}

static bool _SCSDatagramSocketSendInternally(SCSDatagramSocket * __restrict self,
		SCSPointer * __restrict ptr, scs_time timeout) {
	bool tmp_retval;
	scs_socket_desc tmp_sd;
	scs_time tmp_elapsed;
	ssize_t tmp_sndlen;

	if (_SCSDatagramSocketHold(self) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	tmp_retval = true;
	tmp_sd = SCSChannelGetSocketDescriptor(&self->channel);
	tmp_elapsed = 0;
	tmp_sndlen = 0;

	//SCS_PRINT_DEBUG("send:: %zd bytes", length);

	_SCS_UNLOCK(self);

	for (;;) {
		tmp_sndlen = send(tmp_sd, //
				SCSPointerGetReadPointer(*ptr), SCSPointerGetReadableLength(*ptr), 0);

		if (tmp_sndlen < 1) {
			if (tmp_sndlen < 0) {
				scs_errno tmp_errno;

				SCSGetLastError(tmp_errno);

				if ((tmp_errno != EAGAIN) && (tmp_errno != EWOULDBLOCK) && (tmp_errno != EINTR)) {
					char tmp_message[(SCS_EMSGSIZE_MAX + 1)];

					SCSGetErrorMessage(tmp_errno, tmp_message, SCS_EMSGSIZE_MAX);
					SCS_LOG(WARN, SOCKET, 00000, "%s. <<%s,%d,%zu>>", tmp_message,
							SCSChannelGetProfile(&self->channel), tmp_errno,
							SCSPointerGetReadableLength(*ptr));

					if (tmp_errno != EMSGSIZE) {
						_SCSDatagramSocketBroken(self);
					}

					tmp_retval = false;
					break;
				}
			}

			if (!SCSChannelCheckState(&self->channel, SCS_CHNLSTATE_NONE,
					(SCS_CHNLSTATE_BROKEN | SCS_CHNLSTATE_CLOSED))) {
				SCS_LOG(WARN, SOCKET, 79981, "<<%s>>", SCSChannelGetProfile(&self->channel));
				tmp_retval = false;
				break;
			}

			if (0 < timeout) {
				if (timeout < ++tmp_elapsed) {
					SCS_LOG(WARN, SOCKET, 79982, "<<%s>>", SCSChannelGetProfile(&self->channel));
					tmp_retval = false;
					break;
				}
			}

			continue;
		}

		break;
	}

	_SCS_LOCK(self);

	if (0 < tmp_sndlen) {
		//SCS_PRINT_DEBUG("sendto:: %zd bytes", tmp_sndlen);
		SCSTimespecSetCurrentTime(self->send.timestamp, CLOCK_MONOTONIC);
	}

	_SCSDatagramSocketFree(self);

	return tmp_retval;
}

static bool _SCSDatagramSocketRecvfrom(SCSDatagramSocket * __restrict self,
		SCSPointer * __restrict ptr, scs_sockaddr * __restrict addr, size_t addrlen,
		scs_time timeout) {
	bool tmp_retval;
	scs_socket_desc tmp_sd;
	scs_time tmp_elapsed;

	if (_SCSDatagramSocketHold(self) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	tmp_retval = true;
	tmp_sd = SCSChannelGetSocketDescriptor(&self->channel);
	tmp_elapsed = 0;

	_SCS_UNLOCK(self);

	for (;;) {
		scs_socklen tmp_addrlen;
		ssize_t tmp_rcvlen;

		tmp_addrlen = addrlen;
		tmp_rcvlen = recvfrom(tmp_sd, SCSPointerGetWritePointer(*ptr),
				SCSPointerGetWritableLength(*ptr), 0, (struct sockaddr *) addr, &tmp_addrlen);

		if (tmp_rcvlen < 1) {
			if (tmp_rcvlen < 0) {
				scs_errno tmp_errno;

				SCSGetLastError(tmp_errno);

				if ((tmp_errno != EAGAIN) && (tmp_errno != EWOULDBLOCK) && (tmp_errno != EINTR)) {
					char tmp_message[(SCS_EMSGSIZE_MAX + 1)];

					SCSGetErrorMessage(tmp_errno, tmp_message, SCS_EMSGSIZE_MAX);
					SCS_LOG(WARN, SOCKET, 00000, "%s. <<%s,%d>>", //
							tmp_message, SCSChannelGetProfile(&self->channel), tmp_errno);
					_SCSDatagramSocketBroken(self);
					tmp_retval = false;

					break;
				}
			}

			if (!SCSChannelCheckState(&self->channel, SCS_CHNLSTATE_NONE,
					(SCS_CHNLSTATE_BROKEN | SCS_CHNLSTATE_CLOSED))) {
				SCS_LOG(WARN, SOCKET, 79971, "<<%s>>", SCSChannelGetProfile(&self->channel));
				tmp_retval = false;
				break;
			}

			if (0 < timeout) {
				if (timeout < ++tmp_elapsed) {
					SCS_LOG(NOTICE, SOCKET, 79972, "<<%s>>", SCSChannelGetProfile(&self->channel));
					tmp_retval = false;
					break;
				}
			}

			continue;
		}

		//SCS_PRINT_DEBUG("recvfrom:: %zd bytes", tmp_rcvlen);
		SCSPointerMove(*ptr, tmp_rcvlen);

		break;
	}

	_SCS_LOCK(self);

	_SCSDatagramSocketFree(self);

	return tmp_retval;
}

static bool _SCSDatagramSocketRecvInternally(SCSDatagramSocket * __restrict self,
		SCSPointer * __restrict ptr, scs_time timeout) {
	bool tmp_retval;
	scs_socket_desc tmp_sd;
	scs_time tmp_elapsed;

	if (_SCSDatagramSocketHold(self) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	tmp_retval = true;
	tmp_sd = SCSChannelGetSocketDescriptor(&self->channel);
	tmp_elapsed = 0;

	_SCS_UNLOCK(self);

	for (;;) {
		ssize_t tmp_rcvlen;

		tmp_rcvlen = recv(tmp_sd, //
				SCSPointerGetWritePointer(*ptr), SCSPointerGetWritableLength(*ptr), 0);

		if (tmp_rcvlen < 1) {
			if (tmp_rcvlen < 0) {
				scs_errno tmp_errno;

				SCSGetLastError(tmp_errno);

				if ((tmp_errno != EAGAIN) && (tmp_errno != EWOULDBLOCK) && (tmp_errno != EINTR)) {
					char tmp_message[(SCS_EMSGSIZE_MAX + 1)];

					SCSGetErrorMessage(tmp_errno, tmp_message, SCS_EMSGSIZE_MAX);
					SCS_LOG(WARN, SOCKET, 00000, "%s. <<%s,%d>>", //
							tmp_message, SCSChannelGetProfile(&self->channel), tmp_errno);
					_SCSDatagramSocketBroken(self);
					tmp_retval = false;

					break;
				}
			}
			else
			{
				//TODO
				SCS_LOG(WARN, SOCKET, 79971, "<<%s>>", SCSChannelGetProfile(&self->channel));
				tmp_retval = false;
				break;
			}

			//if (!SCSChannelCheckState(&self->channel, SCS_CHNLSTATE_NONE,
			//		(SCS_CHNLSTATE_BROKEN | SCS_CHNLSTATE_CLOSED))) {
			//	SCS_LOG(WARN, SOCKET, 79971, "<<%s>>", SCSChannelGetProfile(&self->channel));
			//	tmp_retval = false;
			//	break;
			//}

			if (0 < timeout) {
				if (timeout < ++tmp_elapsed) {
					SCS_LOG(WARN, SOCKET, 79972, "<<%s>>", SCSChannelGetProfile(&self->channel));
					tmp_retval = false;
					break;
				}
			}

			continue;
		}

		//SCS_PRINT_DEBUG("recvfrom:: %zd bytes", tmp_retlen);
		//SCS_HEXDUMP(tmp_ptr.buffer.ptr, (size_t) tmp_ptr.buffer.length, 16);
		SCSPointerMove(*ptr, tmp_rcvlen);

		break;
	}

	_SCS_LOCK(self);

	_SCSDatagramSocketFree(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

static bool _SCSDatagramSocketSendSynPacket(SCSDatagramSocket * self) {
	SCSPacket * tmp_packet;
	SCSPacketSeqno tmp_seqno;

	if ((tmp_packet = SCSPacketCreate()) != NULL) {
		SCSPacketHold(tmp_packet);
	}
	else {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	tmp_seqno = SCSAtomicGet(self->packet.seqno.self);

	tmp_packet->sockid = self->id;
	tmp_packet->connid = self->connid;
	tmp_packet->option = SCS_SKTOPTN_NONE;
	tmp_packet->flags = SCS_PKTFLAG_CTL | SCS_PKTFLAG_SYN;
	tmp_packet->seqno = tmp_seqno;
	SCSPacketSetRedundancyCallback(tmp_packet, &self->settings.connect.redundancy);
	SCSTimespecSetZero(tmp_packet->interval);
	tmp_packet->state.connected = false;

	if (self->connect.hasty.ptr != NULL) {
		uint8_t * tmp_ptr;

		_SCS_MEMDUP(self->connect.hasty.ptr, self->connect.hasty.size, tmp_ptr);
		SCSPacketSetPayload(tmp_packet, tmp_ptr, self->connect.hasty.size);
	}

	if (SCSPacketQueueEnqueue(&self->send.queue.control, tmp_packet) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		SCSPacketDestroy(tmp_packet);
		SCSPacketFree(tmp_packet);
		return false;
	}

	if (_SCSDatagramSocketEnqueueInternally(self, self->settings.connect.timeout) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
	}

	SCS_LOG(INFO, SOCKET, 10021, "<<%s,%"PRIu64">>", //
			SCSChannelGetProfile(&self->channel), tmp_seqno);

	if (self->connect.packet.syn != NULL) {
		SCSPacketDestroy(self->connect.packet.syn);
		SCSPacketFree(self->connect.packet.syn);
	}
	self->connect.packet.syn = tmp_packet;

	SCSTimespecSetCurrentTime(self->connect.timestamp.syn, CLOCK_MONOTONIC);

	return true;
}

static bool _SCSDatagramSocketReceiveSynPacket(SCSDatagramSocket * __restrict self,
		scs_sockaddr * __restrict addr) {
	SCSPacket * tmp_packet;

	if ((tmp_packet = SCSPacketCreate()) == NULL) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	while (SCSChannelIsListening(&self->channel)) {
		scs_time tmp_timeout;
		uint8_t tmp_buffer[SCS_PACKET_MAXSIZE];
		scs_sockaddr tmp_peer;
		SCSPointer tmp_wp;
		SCSPointer tmp_rp;

		tmp_timeout = self->settings.connect.timeout;
		memset(tmp_buffer, 0, sizeof(tmp_buffer));
		SCSPointerInitialize(tmp_wp, tmp_buffer, sizeof(tmp_buffer), 0);
		memset(&tmp_peer, 0, sizeof(tmp_peer));

		if (_SCSDatagramSocketRecvfrom(self, //
				&tmp_wp, &tmp_peer, sizeof(tmp_peer), tmp_timeout) == false) {
			continue;
		}

// Peer is not fixed.
//		if (SCSAddrInfoCompare(tmp_addr.peer, SCSChannelGetPeerAddr(&self->channel)) == false) {
//			char tmp_info[2][64];
//			SCSAddrInfoToString(tmp_addr.peer, tmp_info[0], sizeof(tmp_info[0]));
//			SCSAddrInfoToString(SCSChannelGetPeerAddr(&self->channel), tmp_info[1], sizeof(tmp_info[1]));
//			SCS_LOG(INFO, SCS_LOGTYPE_SOCKET, SCS_LOGMSG_99999"<<%s,%s>>", tmp_info[0], tmp_info[1]);
//			continue;
//		}

		SCSPointerInitialize(tmp_rp, SCSPointerGetPointer(tmp_wp), SCSPointerGetOffset(tmp_wp), 0);

		if (SCSParserPacket(&tmp_rp, tmp_packet) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
			continue;
		}

		if (SCSPacketCheckMode(tmp_packet, self->packet.mode, SCS_PKTMODE_MASKFULL) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s,%04X,%04X>>", //
					SCSChannelGetProfile(&self->channel), self->packet.mode, //
					SCSPacketGetMode(tmp_packet, SCS_PKTMODE_MASKFULL));
			continue;
		}

		if (SCSPacketIsSyn(tmp_packet) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s,%04X>>", //
					SCSChannelGetProfile(&self->channel), SCSPacketGetFlags(tmp_packet));
			continue;
		}

		if (SCSPacketVerify(tmp_packet) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
			continue;
		}

		SCS_LOG(INFO, SOCKET, 10022, "<<%s,%"PRIu64">>", //
				SCSChannelGetProfile(&self->channel), SCSPacketGetSeqno(tmp_packet));

		self->connid = SCSPacketGetConnectionId(tmp_packet);

		self->packet.seqno.next = SCSPacketGetSeqno(tmp_packet) + 1;
		if (self->packet.seqno.maximum < self->packet.seqno.next) {
			self->packet.seqno.next = 0;
		}

		SCSAtomicSet(self->packet.seqno.peer, SCSPacketGetSeqno(tmp_packet));
		SCSTimespecCopy(self->connect.timestamp.syn, SCSPacketGetTimestamp(tmp_packet));

		if (SCSPacketSequencerStandBy(&self->receive.sequencer, //
				self->packet.mode, SCSPacketGetSeqno(tmp_packet)) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
			SCSPacketDestroy(tmp_packet);
			return false;
		}

		if (SCSPacketHasPayload(tmp_packet) == true) {
			if (self->connect.packet.syn != NULL) {
				SCSPacketDestroy(self->connect.packet.syn);
				SCSPacketFree(self->connect.packet.syn);
			}

			if (SCSPacketHold(tmp_packet)) {
				self->connect.packet.syn = tmp_packet;
			}
			else {
				SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
				SCSPacketDestroy(tmp_packet);
				return false;
			}
		}
		else {
			SCSPacketDestroy(tmp_packet);
		}

		SCSSockAddrCopy(addr, &tmp_peer);

		return true;
	}

	SCS_LOG(WARN, SOCKET, 10020, "<<%s>>", SCSChannelGetProfile(&self->channel));
	SCSPacketDestroy(tmp_packet);

	return false;
}

static bool _SCSDatagramSocketSendSynAckPacket(SCSDatagramSocket * self) {
	SCSPacket * tmp_packet;
	SCSPacketSeqno tmp_seqno;

	if ((tmp_packet = SCSPacketCreate()) != NULL) {
		SCSPacketHold(tmp_packet);
	}
	else {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	tmp_seqno = SCSAtomicGet(self->packet.seqno.self);

	tmp_packet->sockid = self->id;
	tmp_packet->connid = self->connid;
	tmp_packet->option = SCS_SKTOPTN_NONE;
	tmp_packet->flags = SCS_PKTFLAG_CTL | SCS_PKTFLAG_SYN | SCS_PKTFLAG_ACK;
	tmp_packet->seqno = tmp_seqno;
	SCSPacketSetRedundancyCallback(tmp_packet, &self->settings.connect.redundancy);
	SCSTimespecSetZero(tmp_packet->interval);
	tmp_packet->state.connected = false;

	if (self->connect.hasty.ptr != NULL) {
		uint8_t * tmp_ptr;

		_SCS_MEMDUP(self->connect.hasty.ptr, self->connect.hasty.size, tmp_ptr);
		SCSPacketSetPayload(tmp_packet, tmp_ptr, self->connect.hasty.size);
	}

	if (SCSPacketQueueEnqueue(&self->send.queue.control, tmp_packet) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		SCSPacketDestroy(tmp_packet);
		return false;
	}

	if (_SCSDatagramSocketEnqueueInternally(self, self->settings.connect.timeout) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
	}

	SCS_LOG(INFO, SOCKET, 10023, "<<%s,%"PRIu64">>", //
			SCSChannelGetProfile(&self->channel), tmp_seqno);

	if (self->connect.packet.synack != NULL) {
		SCSPacketDestroy(self->connect.packet.synack);
		SCSPacketFree(self->connect.packet.synack);
	}
	self->connect.packet.synack = tmp_packet;

	SCSTimespecSetCurrentTime(self->connect.timestamp.synack, CLOCK_MONOTONIC);

	return true;
}

static bool _SCSDatagramSocketReceiveSynAckPacket(SCSDatagramSocket * self) {
	uint8_t tmp_buffer[SCS_PACKET_MAXSIZE];
	struct {
		SCSPointer write;
		SCSPointer read;
	} tmp_ptr;
	SCSPacket * tmp_packet;

	if ((tmp_packet = SCSPacketCreate()) == NULL) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", //
				SCSChannelGetProfile(&self->channel));
		return false;
	}

	while (SCSChannelCheckState(&self->channel, //
			(SCS_CHNLSTATE_BINDED | SCS_CHNLSTATE_CONNECTING), SCS_CHNLSTATE_MASKFULL)) {
		scs_sockaddr tmp_peer;

		memset(tmp_buffer, 0, sizeof(tmp_buffer));
		SCSPointerInitialize(tmp_ptr.write, tmp_buffer, sizeof(tmp_buffer), 0);
		memset(&tmp_peer, 0, sizeof(tmp_peer));

		if (_SCSDatagramSocketRecvfrom(self, &tmp_ptr.write, //
				&tmp_peer, sizeof(tmp_peer), self->settings.connect.timeout) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
			SCSPacketDestroy(tmp_packet);
			return false;
		}

		if (SCSChannelCheckPeerAddr(&self->channel, &tmp_peer) == false) {
			char tmp_info[64];
			SCSSockAddrToString(tmp_peer, tmp_info, sizeof(tmp_info));
			SCS_LOG(WARN, SOCKET, 99999, "<<%s,%s>>", //
					SCSChannelGetProfile(&self->channel), tmp_info);
			//TODO Clean packet
			continue;
		}

		SCSPointerInitialize(tmp_ptr.read, //
				SCSPointerGetPointer(tmp_ptr.write), SCSPointerGetOffset(tmp_ptr.write), 0);

		if (SCSParserPacket(&tmp_ptr.read, tmp_packet) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
			//TODO Clean packet
			continue;
		}

		if (SCSPacketCheckMode(tmp_packet, self->packet.mode, SCS_PKTMODE_MASKFULL) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s,%04X,%04X>>", SCSChannelGetProfile(&self->channel),
					self->packet.mode, SCSPacketGetMode(tmp_packet, SCS_PKTMODE_MASKFULL));
			//TODO Clean packet
			continue;
		}

		if (SCSPacketIsSynAck(tmp_packet) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s,%04X>>", //
					SCSChannelGetProfile(&self->channel), SCSPacketGetFlags(tmp_packet));
			//TODO Clean packet
			continue;
		}

		if (SCSPacketCheckConnectionId(tmp_packet, self->connid) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s,%"PRIu32",%"PRIu32">>",
					SCSChannelGetProfile(&self->channel), self->connid,
					SCSPacketGetConnectionId(tmp_packet));
			//TODO Clean packet
			continue;
		}

		if (SCSPacketVerify(tmp_packet) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
			//TODO Clean packet
			continue;
		}

		SCS_LOG(INFO, SOCKET, 10024, "<<%s,%"PRIu64">>", //
				SCSChannelGetProfile(&self->channel), SCSPacketGetSeqno(tmp_packet));

		SCSTimespecCopy(self->connect.timestamp.synack, SCSPacketGetTimestamp(tmp_packet));
		SCSAtomicSet(self->packet.seqno.peer, SCSPacketGetSeqno(tmp_packet));

		self->packet.seqno.next = SCSPacketGetSeqno(tmp_packet) + 1;
		if (self->packet.seqno.maximum < self->packet.seqno.next) {
			self->packet.seqno.next = 0;
		}

		if (SCSPacketSequencerStandBy(&self->receive.sequencer, //
				self->packet.mode, SCSPacketGetSeqno(tmp_packet)) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s,%04X>>", //
					SCSChannelGetProfile(&self->channel), SCSPacketGetFlags(tmp_packet));
			SCSPacketDestroy(tmp_packet);
			return false;
		}

		if (SCSPacketHasPayload(tmp_packet) == true) {
			if (self->connect.packet.synack != NULL) {
				SCSPacketDestroy(self->connect.packet.synack);
				SCSPacketFree(self->connect.packet.synack);
			}

			if (SCSPacketHold(tmp_packet)) {
				self->connect.packet.synack = tmp_packet;
			}
			else {
				SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
				SCSPacketDestroy(tmp_packet);
				return false;
			}
		}
		else {
			SCSPacketDestroy(tmp_packet);
		}

		return true;
	}

	SCS_LOG(WARN, SOCKET, 10020, "<<%s>>", SCSChannelGetProfile(&self->channel));
	SCSPacketDestroy(tmp_packet);

	return false;
}

static bool _SCSDatagramSocketSendAckPacket(SCSDatagramSocket * self) {
	SCSPacket * tmp_packet;
	SCSPacketSeqno tmp_seqno;
	scs_timespec tmp_timestamp;

	if ((tmp_packet = SCSPacketCreate()) == NULL) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	tmp_seqno = SCSAtomicIncrease(self->packet.seqno.self);
	SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);

	tmp_packet->sockid = self->id;
	tmp_packet->connid = self->connid;
	tmp_packet->option = SCS_SKTOPTN_NONE;
	tmp_packet->flags = SCS_PKTFLAG_CTL | SCS_PKTFLAG_ACK;
	tmp_packet->seqno = tmp_seqno;
	SCSPacketSetRedundancyCallback(tmp_packet, &self->settings.send.redundancy);
	SCSTimespecSetZero(tmp_packet->interval);
	tmp_packet->state.connected = false;

	if (SCSPacketQueueEnqueue(&self->send.queue.control, tmp_packet) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		SCSPacketDestroy(tmp_packet);
		return false;
	}

	if (_SCSDatagramSocketEnqueueInternally(self, self->settings.connect.timeout) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
	}

	SCS_LOG(INFO, SOCKET, 10025, "<<%s,%"PRIu64">>", //
			SCSChannelGetProfile(&self->channel), tmp_seqno);

	SCSTimespecCopy(self->connect.timestamp.ack, tmp_timestamp);

	return true;
}

static bool _SCSDatagramSocketReceiveAckPacket(SCSDatagramSocket * self) {
	SCSPacket * tmp_packet;

	if ((tmp_packet = SCSPacketCreate()) == NULL) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	while (SCSChannelIsListening(&self->channel)) {
		uint8_t tmp_buffer[SCS_PACKET_MAXSIZE];
		SCSPointer tmp_wp;
		scs_sockaddr tmp_peer;
		SCSPointer tmp_rp;
		SCSPacketSeqno tmp_seqno;
		SCSPacketSequencerResult tmp_retval;

		memset(tmp_buffer, 0, sizeof(tmp_buffer));
		SCSPointerInitialize(tmp_wp, tmp_buffer, sizeof(tmp_buffer), 0);
		memset(&tmp_peer, 0, sizeof(tmp_peer));

		if (_SCSDatagramSocketRecvfrom(self, &tmp_wp, //
				&tmp_peer, sizeof(tmp_peer), self->settings.connect.timeout) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
			SCSPacketDestroy(tmp_packet);
			return false;
		}

		if (SCSChannelCheckPeerAddr(&self->channel, &tmp_peer) == false) {
			char tmp_info[64];
			SCSSockAddrToString(tmp_peer, tmp_info, sizeof(tmp_info));
			SCS_LOG(INFO, SOCKET, 99999, "<<%s,%s>>", //
					SCSChannelGetProfile(&self->channel), tmp_info);
			continue;
		}

		SCSPointerInitialize(tmp_rp, SCSPointerGetPointer(tmp_wp), SCSPointerGetOffset(tmp_wp), 0);

		if (SCSParserPacket(&tmp_rp, tmp_packet) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
			//TODO Clean packet
			continue;
		}

		if (SCSPacketCheckMode(tmp_packet, self->packet.mode, SCS_PKTMODE_MASKFULL) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s,%04X,%04X>>", //
					SCSChannelGetProfile(&self->channel), self->packet.mode, //
					SCSPacketGetMode(tmp_packet, SCS_PKTMODE_MASKFULL));
			//TODO Clean packet
			continue;
		}

		if (SCSPacketIsAck(tmp_packet) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s,%04X,%"PRIu64">>", //
					SCSChannelGetProfile(&self->channel),//
					SCSPacketGetFlags(tmp_packet), SCSPacketGetSeqno(tmp_packet));
			//TODO Clean packet
			continue;
		}

		if (SCSPacketCheckConnectionId(tmp_packet, self->connid) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s,%"PRIu32",%"PRIu32">>", //
					SCSChannelGetProfile(&self->channel), self->connid,
					SCSPacketGetConnectionId(tmp_packet));
			//TODO Clean packet
			continue;
		}

		if (SCSPacketVerify(tmp_packet) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
			continue;
		}

		tmp_seqno = SCSAtomicGet(self->packet.seqno.peer) + 1;

		if (SCSPacketCheckSequence(tmp_packet, tmp_seqno) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s,%"PRIu64",%"PRIu64">>", //
					SCSChannelGetProfile(&self->channel), tmp_seqno, SCSPacketGetSeqno(tmp_packet));
			//TODO Clean packet
			continue;
		}

		if ((tmp_retval = SCSPacketSequencerUpdate(&self->receive.sequencer,
				SCSPacketGetSeqno(tmp_packet))) != SCS_PKTSEQRES_OK) {
			//TODO Error
		}

		SCS_LOG(INFO, SOCKET, 10026, "<<%s,%"PRIu64">>", //
				SCSChannelGetProfile(&self->channel), SCSPacketGetSeqno(tmp_packet));

		SCSAtomicIncrease(self->packet.seqno.peer);
		SCSTimespecCopy(self->connect.timestamp.ack, SCSPacketGetTimestamp(tmp_packet));

		SCSPacketDestroy(tmp_packet);

		return true;
	}

	SCS_LOG(WARN, SOCKET, 10020, "<<%s>>", SCSChannelGetProfile(&self->channel));
	SCSPacketDestroy(tmp_packet);

	return false;
}

static bool _SCSDatagramSocketSendFinPacket(SCSDatagramSocket * self) {

	if (SCSChannelIsConnected(&self->channel)) {
		SCSPacket * tmp_packet;
		SCSPacketSeqno tmp_seqno;
		scs_timespec tmp_timestamp;

		if ((tmp_packet = SCSPacketCreate()) != NULL) {
			SCSPacketHold(tmp_packet);
		}
		else {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
			return false;
		}

		tmp_seqno = SCSAtomicIncrease(self->packet.seqno.self);
		SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);

		tmp_packet->sockid = self->id;
		tmp_packet->connid = self->connid;
		tmp_packet->option = SCS_SKTOPTN_NONE;
		tmp_packet->flags = SCS_PKTFLAG_CTL | SCS_PKTFLAG_FIN;
		tmp_packet->seqno = tmp_seqno;
		SCSPacketSetRedundancyCallback(tmp_packet, &self->settings.close.redundancy);
		SCSTimespecSetZero(tmp_packet->interval);
		tmp_packet->state.connected = false;

		if (SCSPacketQueueEnqueue(&self->send.queue.transmit, tmp_packet) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
			SCSPacketDestroy(tmp_packet);
			SCSPacketFree(tmp_packet);
			return false;
		}

		if (_SCSDatagramSocketEnqueueInternally(self, self->settings.connect.timeout) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		}

		SCS_LOG(INFO, SOCKET, 10027, "<<%s,%"PRIu64">>", //
				SCSChannelGetProfile(&self->channel), tmp_seqno);

		self->close.packet = tmp_packet;
		SCSTimespecAddSec(tmp_timestamp, self->settings.close.delaytime, self->close.timeout);
	}

	return true;
}

static bool _SCSDatagramSocketSendFinAckPacket(SCSDatagramSocket * self) {
	SCSPacket * tmp_packet;
	SCSPacketSeqno tmp_seqno;
	scs_timespec tmp_timestamp;

	if ((tmp_packet = SCSPacketCreate()) != NULL) {
		SCSPacketHold(tmp_packet);
	}
	else {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	tmp_seqno = SCSAtomicIncrease(self->packet.seqno.self);
	SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);

	tmp_packet->sockid = self->id;
	tmp_packet->connid = self->connid;
	tmp_packet->option = SCS_SKTOPTN_NONE;
	tmp_packet->flags = SCS_PKTFLAG_CTL | SCS_PKTFLAG_FIN | SCS_PKTFLAG_ACK;
	tmp_packet->seqno = tmp_seqno;
	SCSPacketSetRedundancyCallback(tmp_packet, &self->settings.close.redundancy);
	SCSTimespecSetZero(tmp_packet->interval);
	tmp_packet->state.connected = false;

	if (SCSPacketQueueEnqueue(&self->send.queue.transmit, tmp_packet) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		SCSPacketDestroy(tmp_packet);
		SCSPacketFree(tmp_packet);
		return false;
	}

	if (_SCSDatagramSocketEnqueueInternally(self, self->settings.connect.timeout) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
	}

	SCS_LOG(INFO, SOCKET, 10029, "<<%s,%"PRIu64">>", //
			SCSChannelGetProfile(&self->channel), tmp_seqno);

	self->close.packet = tmp_packet;
	SCSTimespecAddSec(tmp_timestamp, self->settings.close.delaytime, self->close.timeout);

	return true;
}

static inline void _SCSDatagramSocketStopSynRetransmitting(SCSDatagramSocket * self) {

	if (self->connect.packet.syn != NULL) {
		SCSPacketSetCancel(self->connect.packet.syn);
	}
	else {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
	}

}

static inline void _SCSDatagramSocketStopSynAckRetransmitting(SCSDatagramSocket * self) {

	if (self->connect.packet.synack != NULL) {
		SCSPacketSetCancel(self->connect.packet.synack);
	}
	else {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
	}

}

/* ---------------------------------------------------------------------------------------------- */

static inline void _SCSDatagramSocketSendPathMTUDiscoveryPacket(SCSDatagramSocket * self,
		scs_timespec timestamp) {
	SCSPacket * tmp_packet;
	struct {
		scs_time seconds;
		uint64_t packets;
		uint64_t bytes;
		scs_timespec timestamp;
	} tmp_condition;
	size_t tmp_length;

	if (_SCSDatagramSocketCheckAvailable(self) == false) {
		return;
	}

	if (self->settings.pmtudisc.enabled == false) {
		return;
	}

	tmp_condition.seconds = self->settings.pmtudisc.interval.seconds;
	tmp_condition.packets = self->settings.pmtudisc.interval.packets;
	tmp_condition.bytes = self->settings.pmtudisc.interval.bytes;

	SCSTimespecCopy(tmp_condition.timestamp, self->pmtu.timestamp);
	SCSTimespecIncreaseSec(tmp_condition.timestamp, tmp_condition.seconds);

	if (!(0 < tmp_condition.packets && tmp_condition.packets < SCSAtomicGet(self->pmtu.packets)) && //
			!(0 < tmp_condition.bytes && tmp_condition.bytes < SCSAtomicGet(self->pmtu.bytes)) && //
			!(0 < tmp_condition.seconds && SCSTimespecCompare(tmp_condition.timestamp, timestamp, <))) {
		return;
	}

	if ((tmp_length = SCSPMTUMeasurerPredict(&self->pmtu.measurer)) < 1) {
		// Regular case
		return;
	}

	if ((tmp_packet = SCSPacketCreate()) == NULL) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return;
	}

	tmp_packet->sockid = self->id;
	tmp_packet->connid = self->connid;
	tmp_packet->option = SCS_SKTOPTN_NONE;
	tmp_packet->flags = SCS_PKTFLAG_CTL;
	tmp_packet->seqno = SCSAtomicIncrease(self->packet.seqno.self);
	//tmp_packet->redundancy.config
	//tmp_packet->redundancy.times = 0;
	SCSTimespecSetZero(tmp_packet->interval);
	tmp_packet->state.connected = true;

	if (SCSPacketSetPad(tmp_packet, 0, tmp_length) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		SCSPacketDestroy(tmp_packet);
		return;
	}

	if (SCSPacketQueueEnqueue(&self->send.queue.control, tmp_packet) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		SCSPacketDestroy(tmp_packet);
		return;
	}

	if (_SCSDatagramSocketEnqueueInternally(self, self->settings.send.timeout) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
	}

	//SCS_LOG(NOTICE, DEBUG, 00000,  "Path MTU Discovery. <<%d>>", tmp_length);

	SCSTimespecCopy(self->pmtu.timestamp, timestamp);

}

static inline void _SCSDatagramSocketSendNotificationPacket(SCSDatagramSocket * self,
		scs_timespec timestamp) {
	scs_timespec tmp_interval;
	SCSPacket * tmp_packet;
	SCSPacketWatcherStatus tmp_status;
	SCSFeedbackInfo * tmp_info;

	if (_SCSDatagramSocketCheckAvailable(self) == false) {
		return;
	}

	SCSTimespecCopy(tmp_interval, self->settings.notification.interval);

	if (SCSTimespecIsInfinity(tmp_interval)) {
		return;
	}

	if (SCSTimespecCompare(timestamp, self->notification.send.timestamp.next, <)) {
		return;
	}

	//SCS_PRINT_DEBUG("%zu.%zu < %zu.%zu", //
	//		timestamp.tv_sec, timestamp.tv_nsec, //
	//		self->notification.send.timestamp.next.tv_sec, self->notification.send.timestamp.next.tv_nsec);

	if ((tmp_info = SCSGenerateFeedbackInfo(self)) != NULL) {
		SCSFeedbackInfoCopy(tmp_info, &self->notification.send.feedback);
	}
	else {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return;
	}

	if ((tmp_packet = SCSPacketCreate()) == NULL) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return;
	}

	tmp_packet->sockid = self->id;
	tmp_packet->connid = self->connid;
	tmp_packet->option = SCS_SKTOPTN_NONE;
	tmp_packet->flags = SCS_PKTFLAG_CTL;
	tmp_packet->seqno = SCSAtomicIncrease(self->packet.seqno.self);
	//tmp_packet->redundancy.config
	//tmp_packet->redundancy.times = 0;
	SCSTimespecSetZero(tmp_packet->interval);
	tmp_packet->state.connected = true;
	//tmp_packet->payload
	SCSPacketSetFeedbackInfo(tmp_packet, tmp_info);
	SCSPacketSetFeedbackCallback(tmp_packet, &self->settings.notification.callback);

	if (self->settings.rttmeas.enabled) {
		if (SCSPacketWatcherAdd(&self->rtt.watcher, tmp_packet, &tmp_status) == true) {
			//SCS_LOG(NOTICE, DEBUG, 00000,  "RTT Measurement Request. <<%d>>", tmp_status.id);
			SCSPacketSetRTTMeas(tmp_packet, SCS_PKTRTTMFLAG_REQUEST, tmp_status.id);
		}
		else {
			SCS_LOG(ERROR, SOCKET, 99999, "");
			SCSPacketDestroy(tmp_packet);
			return;
		}
	}

	if (SCSPacketQueueEnqueue(&self->send.queue.control, tmp_packet) == true) {
		SCSTimespecCopy(self->notification.send.timestamp.last, timestamp);
	}
	else {
		SCS_LOG(ERROR, SOCKET, 99999, "");
		SCSPacketDestroy(tmp_packet);
		return;
	}

	if (_SCSDatagramSocketEnqueueInternally(self, self->settings.send.timeout) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
	}

	SCSTimespecAdd(timestamp, tmp_interval, self->notification.send.timestamp.next);

	if (self->settings.rttmeas.enabled) {
		SCSTimespecCopy(self->rtt.timestamp.sent.request, timestamp);
	}

}

static inline void _SCSDatagramSocketReceiveNotificationPacket(SCSDatagramSocket * __restrict self,
		SCSPacket * __restrict packet, scs_timespec timestamp) {
	SCSFeedbackInfo * tmp_info;
	uint64_t tmp_sent;
	uint64_t tmp_diff;

	if ((tmp_info = SCSPacketGetFeedbackInfo(packet)) == NULL) {
		SCS_LOG(ALERT, SOCKET, 99999, "");
		return;
	}

	SCSTimespecCopy(self->notification.receive.timestamp, timestamp);
	SCSFeedbackInfoCopy(tmp_info, &self->notification.receive.feedback.basic);
	SCSFeedbackInfoLogging(tmp_info);

	tmp_sent = SCSFeedbackInfoGetTotalSentPackets(tmp_info);

	if (self->plr.sent < tmp_sent) {
		tmp_diff = tmp_sent - self->plr.sent;
	}
	else {
		tmp_diff = 0;
	}

	//SCS_LOG(NOTICE, DEBUG, 00000, "%"PRIu64",%"PRIu64",%"PRIu64, //
	//		tmp_sent, self->plr.sent, tmp_diff);

	self->plr.sent = tmp_sent;

	if (10000 < tmp_diff) {
		SCSPLRMeasurerUpdate(&self->plr.measurer, SCSPacketGetSeqno(packet), //
				SCSFeedbackInfoGetTotalSentPackets(tmp_info), //
				SCSNetworkTrafficGetTotalReceivedPacket(&self->traffic));
	}
	else {
		SCSPLRMeasurerUpdate(&self->plr.measurer, SCSPacketGetSeqno(packet), //
				SCSFeedbackInfoGetSentDataPackets(tmp_info), //
				SCSNetworkTrafficGetReceivedDataPacket(&self->traffic));
	}
	SCS_LOG(NOTICE, SOCKET, 10041, "<<%d>>", SCSPLRMeasurerGetLatest(&self->plr.measurer));

	if (SCSPMTUMeasurerUpdate(&self->pmtu.measurer, SCSFeedbackInfoGetMTU(tmp_info)) == true) {
		SCS_LOG(NOTICE, SOCKET, 10043, "<<%zu>>", SCSPMTUMeasurerGet(&self->pmtu.measurer));
	}

}

static inline void _SCSDatagramSocketSendRTTResposePacket(SCSDatagramSocket * self, SCSRTTMeasId id) {
	SCSPacket * tmp_packet;

	if ((tmp_packet = SCSPacketCreate()) == NULL) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return;
	}

	tmp_packet->sockid = self->id;
	tmp_packet->connid = self->connid;
	tmp_packet->option = SCS_SKTOPTN_NONE;
	tmp_packet->flags = SCS_PKTFLAG_CTL | SCS_PKTFLAG_RTT;
	tmp_packet->seqno = SCSAtomicIncrease(self->packet.seqno.self);
	//tmp_packet->redundancy.config
	//tmp_packet->redundancy.times = 0;
	SCSTimespecSetZero(tmp_packet->interval);
	tmp_packet->state.connected = true;

	//tmp_packet->payload
	SCSPacketSetRTTMeas(tmp_packet, SCS_PKTRTTMFLAG_NONE, id);

	if (SCSPacketQueueEnqueue(&self->send.queue.control, tmp_packet) == true) {
		//SCS_LOG(NOTICE, DEBUG, 00000, "RTT Measurement Response. <<%d>>", id);
		SCSTimespecSetCurrentTime(self->rtt.timestamp.sent.response, CLOCK_MONOTONIC);
	}
	else {
		SCS_LOG(ERROR, SOCKET, 99999, "");
		SCSPacketDestroy(tmp_packet);
		return;
	}

	if (_SCSDatagramSocketEnqueueInternally(self, self->settings.send.timeout) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
	}

}

static inline void _SCSDatagramSocketReceiveRTTResponsePacket(SCSDatagramSocket * self,
		SCSRTTMeasId id, scs_timespec timestamp) {
	SCSPacketWatcherStatus tmp_status;
	scs_timespec tmp_rtt;

	if (SCSPacketWatcherRemove(&self->rtt.watcher, id, &tmp_status) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%d>>", id);
		return;
	}

	//SCS_LOG(NOTICE, DEBUG, 00000, "%zu.%09d, %zu.%09d, %zu.%09d", //
	//		timestamp.tv_sec, timestamp.tv_nsec, //
	//		tmp_status.timestamp.tv_sec, tmp_status.timestamp.tv_nsec, //
	//		tmp_rtt.tv_sec, tmp_rtt.tv_nsec);

	SCSTimespecSub(timestamp, tmp_status.timestamp, tmp_rtt);
	SCSRTTMeasurerUpdate(&self->rtt.measurer, tmp_rtt);
	SCS_LOG(NOTICE, SOCKET, 10042, "<<%zu.%09d>>", //
			SCSTimespecGetSec(tmp_rtt), SCSTimespecGetNanosec(tmp_rtt));
	SCSTimespecCopy(self->rtt.timestamp.receive.response, timestamp);

}

/* ---------------------------------------------------------------------------------------------- */

static inline void _SCSDatagramSocketMeasureRTT(SCSDatagramSocket * __restrict self,
		SCSPacket * __restrict packet, scs_timespec timestamp) {

	if (SCSPacketCheckRTTMeasurementFlag(packet, //
			SCS_PKTRTTMFLAG_REQUEST, SCS_PKTRTTMFLAG_REQUEST)) {
		SCSTimespecCopy(self->rtt.timestamp.receive.request, timestamp);
		_SCSDatagramSocketSendRTTResposePacket( //
				self, SCSPacketGetRTTMeasurementId(packet));
	}
	else {
		_SCSDatagramSocketReceiveRTTResponsePacket( //
				self, SCSPacketGetRTTMeasurementId(packet), timestamp);
	}

}

static bool _SCSDatagramSocketWaitToReceive(SCSDatagramSocket * self) {
	bool tmp_result;
	SCSPacketQueue * tmp_queue;
	scs_timespec tmp_base;
	scs_timespec tmp_elapsed;
	scs_time tmp_timeout;

	if (_SCSDatagramSocketHold(self) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	tmp_result = true;
	tmp_queue = &self->receive.queue;
	SCSTimespecSetCurrentTime(tmp_base, CLOCK_MONOTONIC);

	_SCS_UNLOCK(self);

	while (SCSPacketQueueGetCount(tmp_queue) < 1) {
		SCSPacketQueueWaitToEnqueue(tmp_queue, 1000);

		if (_SCSDatagramSocketCheckReceivable(self) == false) {
			tmp_result = false;
			break;
		}

		if (0 < (tmp_timeout = self->settings.receive.timeout)) {
			scs_timespec tmp_current;

			SCSTimespecSetCurrentTime(tmp_current, CLOCK_MONOTONIC);
			SCSTimespecSub(tmp_current, tmp_base, tmp_elapsed);

			if (tmp_timeout < SCSTimespecGetSec(tmp_elapsed)) {
				SCS_LOG(WARN, SOCKET, 79972, "<<%s>>", SCSChannelGetProfile(&self->channel));
				tmp_result = false;
				break;
			}
		}
	}

	_SCS_LOCK(self);

	_SCSDatagramSocketFree(self);

	return tmp_result;
}

static bool _SCSDatagramSocketWaitToSendable(SCSDatagramSocket * self) {
	bool tmp_result;
	SCSPacketQueue * tmp_queue;
	scs_timespec tmp_base;
	scs_timespec tmp_elapsed;
	scs_time tmp_timeout;

	if (_SCSDatagramSocketHold(self) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	tmp_result = true;
	tmp_queue = &self->send.queue.transmit;
	SCSTimespecSetCurrentTime(tmp_base, CLOCK_MONOTONIC);

	_SCS_UNLOCK(self);

	while (SCSPacketQueueIsFull(tmp_queue)) {
		SCSPacketQueueWaitToDequeue(tmp_queue, 1000);

		if (_SCSDatagramSocketCheckSendable(self) == false) {
			tmp_result = false;
			break;
		}

		if (0 < (tmp_timeout = self->settings.send.timeout)) {
			scs_timespec tmp_current;

			SCSTimespecSetCurrentTime(tmp_current, CLOCK_MONOTONIC);
			SCSTimespecSub(tmp_current, tmp_base, tmp_elapsed);

			if (tmp_timeout < SCSTimespecGetSec(tmp_elapsed)) {
				SCS_LOG(WARN, SOCKET, 79972, "<<%s>>", SCSChannelGetProfile(&self->channel));
				tmp_result = false;
				break;
			}
		}
	}

	_SCS_LOCK(self);

	_SCSDatagramSocketFree(self);

	return tmp_result;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSDatagramSocketInitialize(SCSDatagramSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	memset(self, 0, sizeof(SCSDatagramSocket));

	self->type = SCS_SKTTYPE_DATAGRAM;

	SCSAtomicReferenceInitialize(self->reference);

	SCSMutexInitialize(self->mutex);
	self->state = SCS_SKTSTATE_INITIALIZED;
	self->id = SCS_SKTID_INVVAL;
	SCSDatagramSocketSettingsInitialize(&self->settings);
	self->settings.send.timeout = 1; /* to avoid a dead lock */
	SCSChannelInitialize(&self->channel, SCS_CHNLCNFG_NONE);
	SCSNetworkTrafficCounterInitialize(&self->traffic);

	self->connid = SCS_CONNID_INVVAL;

	self->packet.mode = SCS_PKTMODE_NONE;
	self->packet.seqno.maximum = SCS_PKTSEQNO_MINVAL;
	self->packet.seqno.initial = SCS_PKTSEQNO_MINVAL;
	self->packet.seqno.next = SCS_PKTSEQNO_MINVAL;
	SCSAtomicInitialize(self->packet.seqno.self, SCS_PKTSEQNO_MINVAL);
	SCSAtomicInitialize(self->packet.seqno.peer, SCS_PKTSEQNO_MINVAL);

	//self->connect.hasty.ptr = NULL;
	//self->connect.hasty.size = 0;
	//self->connect.packet.syn = NULL;
	//self->connect.packet.synack = NULL;
	SCSTimespecInitialize(self->connect.timestamp.syn);
	SCSTimespecInitialize(self->connect.timestamp.synack);
	SCSTimespecInitialize(self->connect.timestamp.ack);

	SCSTimespecInitialize(self->send.timestamp);
	SCSPacketQueueInitialize(&self->send.queue.control);
	SCSPacketQueueInitialize(&self->send.queue.retransmit);
	SCSPacketQueueInitialize(&self->send.queue.transmit);

	SCSTimespecInitialize(self->receive.timestamp);
	SCSPacketQueueInitialize(&self->receive.queue);
	SCSPacketSequencerInitialize(&self->receive.sequencer);

	SCSTimespecInitialize(self->notification.send.timestamp.last);
	SCSTimespecInitialize(self->notification.send.timestamp.next);
	SCSFeedbackInfoInitialize(&self->notification.send.feedback);
	SCSMutexInitialize(self->notification.receive.mutex);
	SCSTimespecInitialize(self->notification.receive.timestamp);
	SCSFeedbackInfoInitialize(&self->notification.receive.feedback.basic);
	//self->notification.receive.feedback.extension.ptr = NULL;
	//self->notification.receive.feedback.extension.size = 0;
	//self->notification.receive.feedback.extension.length = 0;

	SCSTimespecInitialize(self->pmtu.timestamp);
	SCSAtomicInitialize(self->pmtu.packets, 0);
	SCSAtomicInitialize(self->pmtu.bytes, 0);
	SCSPMTUMeasurerInitialize(&self->pmtu.measurer);

	SCSTimespecInitialize(self->rtt.timestamp.sent.request);
	SCSTimespecInitialize(self->rtt.timestamp.sent.response);
	SCSTimespecInitialize(self->rtt.timestamp.receive.request);
	SCSTimespecInitialize(self->rtt.timestamp.receive.response);
	SCSRTTMeasurerInitialize(&self->rtt.measurer);
	SCSPacketWatcherInitialize(&self->rtt.watcher);

	SCSPLRMeasurerInitialize(&self->plr.measurer);
	self->plr.sent = 0;

	SCSJitterMeasurerInitialize(&self->jitter.measurer);

	//self->close.done = false;
	self->close.packet = NULL;
	SCSTimespecInitialize(self->close.timeout);

}

void SCSDatagramSocketFinalize(SCSDatagramSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	self->type = SCS_SKTTYPE_NONE;

	SCSAtomicReferenceFinalize(self->reference);

	SCSMutexFinalize(self->mutex);
	self->state = SCS_SKTSTATE_INITIALIZED;
	self->id = SCS_SKTID_INVVAL;

	SCSDatagramSocketSettingsFinalize(&self->settings);
	SCSChannelFinalize(&self->channel);
	SCSNetworkTrafficCounterFinalize(&self->traffic);

	self->connid = SCS_CONNID_INVVAL;

	self->packet.mode = SCS_PKTMODE_NONE;
	self->packet.seqno.maximum = SCS_PKTSEQNO_MINVAL;
	self->packet.seqno.initial = SCS_PKTSEQNO_MINVAL;
	self->packet.seqno.next = SCS_PKTSEQNO_MINVAL;
	SCSAtomicFinalize(self->packet.seqno.self, SCS_PKTSEQNO_MINVAL);
	SCSAtomicFinalize(self->packet.seqno.peer, SCS_PKTSEQNO_MINVAL);

	if (self->connect.hasty.ptr != NULL) {
		free(self->connect.hasty.ptr);
		self->connect.hasty.ptr = NULL;
		self->connect.hasty.size = 0;
	}
	if (self->connect.packet.syn != NULL) {
		SCSPacketDestroy(self->connect.packet.syn);
		SCSPacketFree(self->connect.packet.syn);
		self->connect.packet.syn = NULL;
	}
	if (self->connect.packet.synack != NULL) {
		SCSPacketDestroy(self->connect.packet.synack);
		SCSPacketFree(self->connect.packet.synack);
		self->connect.packet.synack = NULL;
	}
	SCSTimespecFinalize(self->connect.timestamp.syn);
	SCSTimespecFinalize(self->connect.timestamp.synack);
	SCSTimespecFinalize(self->connect.timestamp.ack);

	SCSTimespecFinalize(self->connect.timestamp.syn);
	SCSTimespecFinalize(self->connect.timestamp.synack);
	SCSTimespecFinalize(self->connect.timestamp.ack);

	SCSTimespecFinalize(self->send.timestamp);
	SCSPacketQueueFinalize(&self->send.queue.control);
	SCSPacketQueueFinalize(&self->send.queue.retransmit);
	SCSPacketQueueFinalize(&self->send.queue.transmit);

	SCSTimespecFinalize(self->receive.timestamp);
	SCSPacketQueueFinalize(&self->receive.queue);
	SCSPacketSequencerFinalize(&self->receive.sequencer);

	SCSTimespecFinalize(self->notification.send.timestamp.last);
	SCSTimespecFinalize(self->notification.send.timestamp.next);
	SCSFeedbackInfoInitialize(&self->notification.send.feedback);
	SCSMutexFinalize(self->notification.receive.mutex);
	SCSTimespecFinalize(self->notification.receive.timestamp);
	SCSFeedbackInfoInitialize(&self->notification.receive.feedback.basic);
	if (self->notification.receive.feedback.extension.ptr != NULL) {
		free(self->notification.receive.feedback.extension.ptr);
		self->notification.receive.feedback.extension.ptr = NULL;
		self->notification.receive.feedback.extension.size = 0;
		self->notification.receive.feedback.extension.length = 0;
	}

	SCSTimespecFinalize(self->pmtu.timestamp);
	SCSAtomicFinalize(self->pmtu.packets, 0);
	SCSAtomicFinalize(self->pmtu.bytes, 0);
	SCSPMTUMeasurerFinalize(&self->pmtu.measurer);

	SCSTimespecFinalize(self->rtt.timestamp.sent.request);
	SCSTimespecFinalize(self->rtt.timestamp.sent.response);
	SCSTimespecFinalize(self->rtt.timestamp.receive.request);
	SCSTimespecFinalize(self->rtt.timestamp.receive.response);
	SCSRTTMeasurerFinalize(&self->rtt.measurer);
	SCSPacketWatcherFinalize(&self->rtt.watcher);

	SCSPLRMeasurerFinalize(&self->plr.measurer);
	self->plr.sent = 0;

	SCSJitterMeasurerFinalize(&self->jitter.measurer);

	memset(self, 0, sizeof(SCSDatagramSocket));

	//self->close.done = false;
	if (self->close.packet != NULL) {
		SCSPacketDestroy(self->close.packet);
		SCSPacketFree(self->close.packet);
		self->close.packet = NULL;
	}
	SCSTimespecFinalize(self->close.timeout);

}

/* ---------------------------------------------------------------------------------------------- */

static bool _SCSDatagramSocketCanDestroy(SCSDatagramSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	if (SCSAtomicReferenceCanDestroy(self->reference) == false) {
		return false;
	}

	return true;
}

static bool _SCSDatagramSocketCreate(SCSDatagramSocket * self, SCSProtocolType protocol) {
	SCSChannelConfig tmp_config;

	tmp_config = SCS_CHNLCNFG_NONE;

	switch (protocol) {
		case SCS_PROTOCOLTYPE_IP: {
			tmp_config |= SCS_CHNLCNFG_IP;
			break;
		}
		case SCS_PROTOCOLTYPE_UDP: {
			tmp_config |= SCS_CHNLCNFG_UDP;
			break;
		}
		default: {
			SCS_LOG(WARN, SYSTEM, 00003, "<<0x%02X>>", protocol);
			return false;
		}
	}

	SCSDatagramSocketInitialize(self);
	SCSChannelInitialize(&self->channel, tmp_config);

	return true;
}
SCSDatagramSocket * SCSDatagramSocketCreate(SCSProtocolType protocol) {
	SCSDatagramSocket * tmp_self;

//	if (SCSSocketTypeValidate(type) == false) {
//		SCS_LOG(ALERT, SYSTEM, 99997,  "<<%d>>", type);
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return NULL;
//	}

//	if (SCSDatagramSocketProtocolValidate(protocol) == false) {
//		SCS_LOG(ALERT, SYSTEM, 99997,  "<<%d>>", protocol);
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return NULL;
//	}

	if ((tmp_self = (SCSDatagramSocket *) malloc(sizeof(SCSDatagramSocket))) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 00002, "");
		SCSSetLastError(SCS_ERRNO_EMEMORY);
		return NULL;
	}

	if (_SCSDatagramSocketCreate(tmp_self, protocol) == false) {
		free(tmp_self);
		return NULL;
	}

	SCSObjectCounterIncreaseCreate(_counter);

	return tmp_self;
}

static bool _SCSDatagramSocketDestroy(void * self) {
	SCSDatagramSocket * tmp_self;

	if ((tmp_self = (SCSDatagramSocket*) self) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99998, "<<%zu>>", sizeof(SCSDatagramSocket));
		return true;
	}

	if (_SCSDatagramSocketCanDestroy(tmp_self) == false) {
		return false;
	}

	SCSDatagramSocketFinalize(tmp_self);
	free(tmp_self);

	SCSObjectCounterIncreaseDestroyed(_counter);

	return true;
}
void SCSDatagramSocketDestroy(SCSDatagramSocket * self) {
	SCSGarbage * tmp_garbage;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return;
	}

	bool tmp_destroyed;

	_SCS_LOCK(self);

	tmp_destroyed = SCSAtomicReferenceIsDestroy(self->reference);

	if (tmp_destroyed == false) {
		SCSAtomicReferenceForbid(self->reference);
	}

	_SCS_UNLOCK(self);

	if (tmp_destroyed == true) {
		// Already destroyed.
		return;
	}

	SCSObjectCounterIncreaseDestroy(_counter);

	if ((tmp_garbage = SCSGarbageCreate(self, _SCSDatagramSocketDestroy)) == NULL) {
		//TODO Memory leak
		return;
	}

	SCSGarbageCollectorPush(&_scs_global_gc, tmp_garbage);

}

bool SCSDatagramSocketHold(SCSDatagramSocket * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	return _SCSDatagramSocketHold(self);
}

void SCSDatagramSocketFree(SCSDatagramSocket * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return;
	}

	_SCSDatagramSocketFree(self);

}

/* ---------------------------------------------------------------------------------------------- */

static inline bool _SCSDatagramSocketStandBy(SCSDatagramSocket * __restrict self,
		SCSDatagramSocketSettings * __restrict settings) {

	SCSDatagramSocketSettingsCopy(&self->settings, settings);

	SCSTimespecSetCurrentTime(self->receive.timestamp, CLOCK_MONOTONIC);

	if (SCSPacketQueueStandBy(&self->send.queue.control, settings->send.queue.length) == false) {
		SCS_LOG(ERROR, SOCKET, 99999, "");
		return false;
	}

	if (SCSPacketQueueStandBy(&self->send.queue.retransmit, settings->send.queue.length) == false) {
		SCS_LOG(ERROR, SOCKET, 99999, "");
		return false;
	}

	if (SCSPacketQueueStandBy(&self->send.queue.transmit, settings->send.queue.length) == false) {
		SCS_LOG(ERROR, SOCKET, 99999, "");
		return false;
	}

	if (SCSPacketQueueStandBy(&self->receive.queue, settings->receive.queue.length) == false) {
		SCS_LOG(ERROR, SOCKET, 99999, "");
		return false;
	}

	SCSTimespecSetZero(self->notification.send.timestamp.next);

	if (SCSRTTMeasurerStandBy(&self->rtt.measurer, SCS_RTTMEASURER_MAXENTRIES) == false) {
		SCS_LOG(ERROR, SOCKET, 99999, "");
		return false;
	}

	if (SCSPacketWatcherStandBy(&self->rtt.watcher, &self->settings.rttmeas.watcher) == false) {
		SCS_LOG(ERROR, SOCKET, 99999, "");
		return false;
	}

	if (SCSPLRMeasurerStandBy(&self->plr.measurer, self->settings.plrmeas.samples) == false) {
		SCS_LOG(ERROR, SOCKET, 99999, "");
		return false;
	}

	if (SCSJitterMeasurerStandBy(&self->jitter.measurer, self->settings.jittermeas.samples) == false) {
		SCS_LOG(ERROR, SOCKET, 99999, "");
		return false;
	}

	self->state = SCS_SKTSTATE_STANDBY;

	return true;
}
bool SCSDatagramSocketStandBy(SCSDatagramSocket * __restrict self,
		SCSDatagramSocketSettings * __restrict settings) {
	bool tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (context == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (settings == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

	_SCS_LOCK(self);

	tmp_result = _SCSDatagramSocketStandBy(self, settings);

	_SCS_UNLOCK(self);

	return tmp_result;
}

bool SCSDatagramSocketBind(SCSDatagramSocket * __restrict self, scs_sockaddr * __restrict addr) {
	bool tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (addr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

	_SCS_LOCK(self);

	if ((tmp_result = SCSChannelOpenAsClient(&self->channel, addr)) == true) {
		self->state = SCS_SKTSTATE_BIND;
	}

	_SCS_UNLOCK(self);

	return tmp_result;
}

static inline bool _SCSDatagramSocketConnect(SCSDatagramSocket * __restrict self,
		scs_sockaddr * __restrict addr) {

	if (SCSChannelConnect(&self->channel, addr)) {
		SCSPacketMode tmp_mode;
		SCSPacketSeqno tmp_maximum_seqno;
		SCSPacketSeqno tmp_initial_seqno;

		tmp_mode = _SCSDatagramSocketGetPacketMode(self);
		tmp_maximum_seqno = SCSPacketSeqnoGetMax(tmp_mode);
		tmp_initial_seqno = SCSGeneratePacketSeqno(tmp_mode);

		self->connid = SCSGenerateConnectionId();
		self->packet.mode = tmp_mode;
		self->packet.seqno.maximum = tmp_maximum_seqno;
		self->packet.seqno.initial = tmp_initial_seqno;
		SCSAtomicSet(self->packet.seqno.self, tmp_initial_seqno);

		SCS_PRINT_DEBUG("CONNECT");
	}
	else {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	if (_SCSDatagramSocketSendSynPacket(self)) {
		SCS_PRINT_DEBUG("SYN SENT");
	}
	else {
		SCS_LOG(WARN, SOCKET, 00000, "!");
		return false;
	}

	if (_SCSDatagramSocketReceiveSynAckPacket(self)) {
		SCS_PRINT_DEBUG("SYN-ACK RECEIVED");
		_SCSDatagramSocketStopSynRetransmitting(self);
	}
	else {
		_SCSDatagramSocketStopSynRetransmitting(self);
		return false;
	}

	if (_SCSDatagramSocketSendAckPacket(self)) {
		SCS_PRINT_DEBUG("ACK SENT");
	}
	else {
		SCS_LOG(WARN, SOCKET, 00000, "!");
		return false;
	}

	if (SCSChannelConnected(&self->channel)) {
		SCS_PRINT_DEBUG("CONNECTED");
	}
	else {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	self->state = SCS_SKTSTATE_CONNECTED;

	return true;
}
bool SCSDatagramSocketConnect(SCSDatagramSocket * __restrict self, scs_sockaddr * __restrict addr) {
	bool tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (addr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

	_SCS_LOCK(self);

	tmp_result = _SCSDatagramSocketConnect(self, addr);

	_SCS_UNLOCK(self);

	return tmp_result;
}

bool SCSDatagramSocketListen(SCSDatagramSocket * __restrict self, scs_sockaddr * __restrict addr) {
	bool tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (addr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

	_SCS_LOCK(self);

	if ((tmp_result = SCSChannelOpenAsServer(&self->channel, addr)) == true) {
		self->state = SCS_SKTSTATE_BIND;
	}

	_SCS_UNLOCK(self);

	return tmp_result;
}

static inline bool _SCSDatagramSocketAccept(SCSDatagramSocket * __restrict self,
		scs_sockaddr * __restrict addr) {
	scs_sockaddr tmp_peer;

	if (SCSChannelAccept1(&self->channel) == true) {
		SCSPacketMode tmp_mode;
		SCSPacketSeqno tmp_maximum_seqno;
		SCSPacketSeqno tmp_initial_seqno;

		tmp_mode = _SCSDatagramSocketGetPacketMode(self);
		tmp_maximum_seqno = SCSPacketSeqnoGetMax(tmp_mode);
		tmp_initial_seqno = SCSGeneratePacketSeqno(tmp_mode);

		self->packet.mode = tmp_mode;
		self->packet.seqno.maximum = tmp_maximum_seqno;
		self->packet.seqno.initial = tmp_initial_seqno;
		SCSAtomicSet(self->packet.seqno.self, tmp_initial_seqno);

		SCS_PRINT_DEBUG("ACCEPT1");
	}
	else {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		return false;
	}

	while (SCSChannelIsListening(&self->channel)) {

		SCSSockAddrInitialize(tmp_peer);

		if (_SCSDatagramSocketReceiveSynPacket(self, &tmp_peer)) {
			SCS_PRINT_DEBUG("SYN RECEIVED");
		}
		else {
			continue;
		}

		if (SCSChannelAccept2(&self->channel, &tmp_peer)) {
			SCS_PRINT_DEBUG("ACCEPT2");
		}
		else {
			SCS_LOG(WARN, SOCKET, 00000, "!");
			continue;
		}

		if (_SCSDatagramSocketSendSynAckPacket(self)) {
			SCS_PRINT_DEBUG("SYN-ACK SENT");
		}
		else {
			SCS_LOG(WARN, SOCKET, 00000, "!");
			continue;
		}

		if (_SCSDatagramSocketReceiveAckPacket(self)) {
			SCS_PRINT_DEBUG("ACK RECEIVED");
			_SCSDatagramSocketStopSynAckRetransmitting(self);
		}
		else {
			SCS_LOG(WARN, SOCKET, 00000, "!");
			_SCSDatagramSocketStopSynAckRetransmitting(self);
			continue;
		}

		if (SCSChannelAccepted(&self->channel)) {
			SCS_PRINT_DEBUG("ACCEPTED");
		}
		else {
			SCS_LOG(WARN, SOCKET, 00000, "!");
			continue;
		}

		if (addr != NULL) {
			SCSSockAddrCopy(addr, &tmp_peer);
		}

		self->state = SCS_SKTSTATE_CONNECTED;

		return true;
	}

	return false;
}
bool SCSDatagramSocketAccept(SCSDatagramSocket * __restrict self, scs_sockaddr * __restrict addr) {
	bool tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (addr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

	_SCS_LOCK(self);

	tmp_result = _SCSDatagramSocketAccept(self, addr);

	_SCS_UNLOCK(self);

	return tmp_result;
}

static inline bool _SCSDatagramSocketEnqueue(SCSDatagramSocket * __restrict self, SCSPointer ptr,
		SCSSocketOption option, int redundancy, SCSPacketSeqno * __restrict seqno) {
	size_t tmp_length;
	void * tmp_ptr;
	SCSPacket * tmp_packet;
	SCSPacketSeqno tmp_seqno;
	scs_timespec tmp_timestamp;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (ptr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (offset == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	tmp_length = SCSPointerGetReadableLength(ptr);
	_SCS_MEMDUP(SCSPointerGetReadPointer(ptr), tmp_length, tmp_ptr);

	if ((tmp_packet = SCSPacketCreate()) == NULL) {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		free(tmp_ptr);
		return false;
	}

	SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);
	tmp_seqno = SCSPacketSeqnoIncrease(&self->packet.seqno.self, self->packet.seqno.maximum);

	tmp_packet->sockid = self->id;
	tmp_packet->connid = self->connid;
	tmp_packet->option = option;
	tmp_packet->flags = SCS_PKTFLAG_NONE;
	tmp_packet->seqno = tmp_seqno;

	if (0 <= redundancy) {
		SCSPacketSetRedundancyCallback(tmp_packet, &self->settings.send.redundancy);
		SCSRedundancyCallbackConfigSetTimes(&tmp_packet->redundancy.config, redundancy);
	}
	else {
		SCSPacketSetRedundancyCallback(tmp_packet, &self->settings.send.redundancy);
		tmp_packet->redundancy.times = self->settings.send.redundancy.times;
	}

	SCSTimespecSetZero(tmp_packet->interval);
	tmp_packet->state.connected = true;

	if (SCSPacketSetPayload(tmp_packet, tmp_ptr, tmp_length) == false) {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		SCSPacketDestroy(tmp_packet);
		return false;
	}

	if (SCSPacketQueueEnqueue(&self->send.queue.transmit, tmp_packet) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
		SCSPacketDestroy(tmp_packet);
		return false;
	}

	if (_SCSDatagramSocketEnqueueInternally(self, self->settings.send.timeout) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
	}

	if (seqno != NULL) {
		*seqno = tmp_seqno;
	}

	return true;
}
bool SCSDatagramSocketEnqueue(SCSDatagramSocket * __restrict self, SCSPointer ptr,
		SCSSocketOption option, int redundancy, SCSPacketSeqno * __restrict seqno) {
	bool tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (ptr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (length < *offset) {
//		SCS_LOG(WARN, SYSTEM, 99997,  "<<%d/%d>>", *offset, length);
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if ((length - *offset) < 1) {
//		return true;
//	}

	_SCS_LOCK(self);

	tmp_retval = _SCSDatagramSocketEnqueue(self, ptr, option, redundancy, seqno);

	_SCS_UNLOCK(self);

	return tmp_retval;
}

static bool _SCSDatagramSocketDequeue(SCSDatagramSocket * __restrict self,
		SCSPointer * __restrict ptr, SCSSocketOption option, SCSPacketSeqno * __restrict seqno) {
	SCSPacket * tmp_packet;
	struct {
		void * ptr;
		size_t length;
	} tmp_payload;

	if ((tmp_packet = SCSPacketQueueDequeue(&self->receive.queue)) == NULL) {
		return false;
	}

	if (SCSPacketHasPayload(tmp_packet) == true) {
		tmp_payload.ptr = SCSPacketGetPayloadPointer(tmp_packet);
		tmp_payload.length = SCSPacketGetPayloadSize(tmp_packet);
	}
	else {
		SCSPacketDestroy(tmp_packet);
		SCSPacketFree(tmp_packet);
		return false;
	}

	if (!SCSPointerCanWrite(*ptr, tmp_payload.length)) {
		//TODO Too short buffer
		abort();
	}

	SCSPointerWrite(*ptr, tmp_payload.ptr, tmp_payload.length);

	if (seqno != NULL) {
		*seqno = tmp_packet->seqno;
	}

	SCSPacketDestroy(tmp_packet);
	SCSPacketFree(tmp_packet);

	return true;
}
bool SCSDatagramSocketDequeue(SCSDatagramSocket * __restrict self, SCSPointer * __restrict ptr,
		SCSSocketOption option, SCSPacketSeqno * __restrict seqno) {
	bool tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (ptr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (SCS_PACKET_MAXSIZE < length) {
//		SCS_LOG(WARN, SOCKET, 99998,  "<<%d>>", length);
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (length < *offset) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if ((length - *offset) < 1) {
//		return true;
//	}

	_SCS_LOCK(self);

	while ((tmp_result = _SCSDatagramSocketDequeue(self, ptr, option, seqno)) == false) {
		if ((option & SCS_SKTOPTN_NOBLOCK)) {
			tmp_result = false;
			break;
		}

		if (_SCSDatagramSocketWaitToReceive(self) == false) {
			tmp_result = false;
			break;
		}
	}

	_SCS_UNLOCK(self);

	return tmp_result;
}

bool SCSDatagramSocketWiatToSendable(SCSDatagramSocket * self) {
	bool tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

	_SCS_LOCK(self);

	tmp_result = _SCSDatagramSocketWaitToSendable(self);

	_SCS_UNLOCK(self);

	return tmp_result;
}


static inline bool _SCSDatagramSocketSend(SCSDatagramSocket * __restrict self,
		SCSNetworkTrafficCounter * __restrict statistics) {
#ifdef _SCS_SEND
#error
#endif

#define _SCS_SEND(xxx_self, xxx_ptr, xxx_timeout) \
	(SCSPacketIsConnected(tmp_packet) ? _SCSDatagramSocketSendInternally(xxx_self, xxx_ptr, xxx_timeout) : _SCSDatagramSocketSendto(xxx_self, xxx_ptr, xxx_timeout))

	SCSNetworkTrafficCounter * tmp_statistics;
	uint8_t tmp_buffer[SCS_PACKET_MAXSIZE];
	char * tmp_ptr;
	size_t tmp_length;
	SCSPointer tmp_wp;
	SCSPointer tmp_rp;
	SCSPacket * tmp_packet;

	tmp_statistics = &self->traffic;
	SCSPointerInitialize(tmp_wp, tmp_buffer, sizeof(tmp_buffer), 0);

	if ((tmp_packet = SCSPacketQueueDequeue(&self->send.queue.control)) == NULL) {
		if ((tmp_packet = SCSPacketQueueDequeue(&self->send.queue.retransmit)) == NULL) {
			if ((tmp_packet = SCSPacketQueueDequeue(&self->send.queue.transmit)) == NULL) {
				return true;
			}
		}
	}

	/* Control sending interval of a packets */

	if (SCSPacketCanSend(tmp_packet) == false) {
		if (SCSPacketIsCancel(tmp_packet)) {
			//SCS_LOG(WARN, SOCKET, 99999,  "<<%s>>", SCSChannelGetProfile(&self->channel));

			SCSNetworkTrafficCounterIncreaseCancelPacketInSending(statistics, tmp_packet, 0);
			SCSNetworkTrafficCounterIncreaseCancelPacketInSending(tmp_statistics, tmp_packet, 0);

			SCSPacketDestroy(tmp_packet);
			SCSPacketFree(tmp_packet);

			return true;
		}

		if (SCSPacketQueueEnqueue(&self->send.queue.retransmit, tmp_packet) == false) {
			SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));

			SCSNetworkTrafficCounterIncreaseDroppedPacketInSending(statistics, tmp_packet, 0);
			SCSNetworkTrafficCounterIncreaseDroppedPacketInSending(tmp_statistics, tmp_packet, 0);

			SCSPacketDestroy(tmp_packet);
			SCSPacketFree(tmp_packet);

			return true;
		}

		//_SCSDatagramSocketEnqueueInternally(self, self->settings.send.timeout);

		SCSPacketFree(tmp_packet);

		return false;
	}

	/* Create a packets */

	tmp_packet->mode = self->packet.mode;

	if (self->settings.verification.enabled || SCSPacketCheckOption(tmp_packet, SCS_SKTOPTN_VERIFY)) {
		SCSPacketVerificationMethod tmp_method;

		tmp_method = self->settings.verification.method;

		if (SCSPacketVerificationMethodValidate(tmp_method)) {
			if (SCSPacketSetVerification(tmp_packet, tmp_method) == false) {
				SCS_LOG(WARN, SOCKET, 99999, "<<%d>>", tmp_method);
			}
		}
		else {
			SCS_LOG(WARN, SOCKET, 99999, "<<%d>>", tmp_method);
		}
	}

	SCSTimespecSetCurrentTime(tmp_packet->timestamp.self, CLOCK_MONOTONIC);

	if (SCSBuildPacket(tmp_packet, &tmp_wp) == true) {
		tmp_ptr = SCSPointerGetPointer(tmp_wp);
		tmp_length = SCSPointerGetOffset(tmp_wp);
		SCSPointerInitialize(tmp_rp, tmp_ptr, tmp_length, 0);
	}
	else {
		//SCS_LOG(WARN, SOCKET, 99999,  "<<%s>>", SCSChannelGetProfile(&self->channel));

		SCSNetworkTrafficCounterIncreaseDroppedPacketInSending(statistics, tmp_packet, 0);
		SCSNetworkTrafficCounterIncreaseDroppedPacketInSending(tmp_statistics, tmp_packet, 0);

		SCSPacketDestroy(tmp_packet);
		SCSPacketFree(tmp_packet);

		return true;
	}

	/* Send a packets */

	if (_SCS_SEND(self, &tmp_rp, self->settings.send.timeout) == true) {
		SCSNetworkTrafficCounterIncreaseSentPacket(statistics, tmp_packet, tmp_length);
		SCSNetworkTrafficCounterIncreaseSentPacket(tmp_statistics, tmp_packet, tmp_length);
		SCSChannelSend(&self->channel);
	}
	else {
		//SCS_LOG(WARN, SOCKET, 99999,  "<<%s>>", SCSChannelGetProfile(&self->channel));

		SCSNetworkTrafficCounterIncreaseDroppedPacketInSending( //
				statistics, tmp_packet, tmp_length);
		SCSNetworkTrafficCounterIncreaseDroppedPacketInSending( //
				tmp_statistics, tmp_packet, tmp_length);

		SCSPacketDestroy(tmp_packet);
		SCSPacketFree(tmp_packet);

		return true;
	}

	/* Redundancy */

	if (SCSPacketBecomeRedundancy(tmp_packet) == true) {
		if (SCSPacketHasRedundancyCallbackFunction(tmp_packet)) {
			/* Standard */
			if (SCSPacketQueueEnqueue(&self->send.queue.retransmit, tmp_packet) == false) {
				SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));

				SCSNetworkTrafficCounterIncreaseDroppedPacketInSending( //
						statistics, tmp_packet, tmp_length);
				SCSNetworkTrafficCounterIncreaseDroppedPacketInSending( //
						tmp_statistics, tmp_packet, tmp_length);

				SCSPacketDestroy(tmp_packet);
				SCSPacketFree(tmp_packet);

				return true;
			}

			//_SCSDatagramSocketEnqueueInternally(self, self->settings.send.timeout);

			SCSPacketFree(tmp_packet);

			return false;
		}
		else {
			/* Scatter */
			//TODO PROVE: Rebuild packet
			SCSPointerInitialize(tmp_wp, tmp_buffer, sizeof(tmp_buffer), 0);

			if (SCSBuildPacket(tmp_packet, &tmp_wp) == true) {
				tmp_ptr = SCSPointerGetPointer(tmp_wp);
				tmp_length = SCSPointerGetOffset(tmp_wp);
				SCSPointerInitialize(tmp_rp, tmp_ptr, tmp_length, 0);
			}
			else {
				//SCS_LOG(WARN, SOCKET, 99999,  "<<%s>>", SCSChannelGetProfile(&self->channel));

				SCSNetworkTrafficCounterIncreaseDroppedPacketInSending(statistics, tmp_packet, 0);
				SCSNetworkTrafficCounterIncreaseDroppedPacketInSending(tmp_statistics, tmp_packet,
						0);

				SCSPacketDestroy(tmp_packet);
				SCSPacketFree(tmp_packet);

				return true;
			}

			do {
				if (_SCS_SEND(self, &tmp_rp, self->settings.send.timeout) == true) {
					SCSNetworkTrafficCounterIncreaseSentPacket( //
							statistics, tmp_packet, tmp_length);
					SCSNetworkTrafficCounterIncreaseSentPacket( //
							tmp_statistics, tmp_packet, tmp_length);
					SCSChannelSend(&self->channel);
				}
				else {
					//SCS_LOG(WARN, SOCKET, 99999,  "<<%s>>", SCSChannelGetProfile(&self->channel));

					SCSNetworkTrafficCounterIncreaseDroppedPacketInSending( //
							statistics, tmp_packet, tmp_length);
					SCSNetworkTrafficCounterIncreaseDroppedPacketInSending( //
							tmp_statistics, tmp_packet, tmp_length);

					break;
				}
			} while (SCSPacketBecomeRedundancy(tmp_packet));

			SCSPacketDestroy(tmp_packet);
			SCSPacketFree(tmp_packet);

			return true;
		}
	}

	SCSPacketDestroy(tmp_packet);
	SCSPacketFree(tmp_packet);

	return true;

#undef _SCS_SEND
}
bool SCSDatagramSocketSend(SCSDatagramSocket * __restrict self,
		SCSNetworkTrafficCounter * __restrict statistics) {
	bool tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (packets == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (ptr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (statistics == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

	_SCS_LOCK(self);

	tmp_result = _SCSDatagramSocketSend(self, statistics);

	_SCS_UNLOCK(self);

	return tmp_result;
}

static inline bool _SCSDatagramSocketReceive(SCSDatagramSocket * __restrict self, int sd,
		SCSNetworkTrafficCounter * __restrict statistics) {
	SCSNetworkTrafficCounter * tmp_statistics;
	scs_time tmp_timeout;
	char * tmp_ptr;
	size_t tmp_length;
	SCSPacket * tmp_packet;
	uint8_t tmp_buffer[SCS_PACKET_MAXSIZE];
	scs_timespec tmp_timestamp;
	SCSPointer tmp_wp;
	SCSPointer tmp_rp;
	SCSPacketSequencerResult tmp_result;

	tmp_statistics = &self->traffic;
	tmp_timeout = self->settings.receive.timeout;
	tmp_length = 0;

	if ((tmp_packet = SCSPacketCreate()) != NULL) {
		SCSPacketHold(tmp_packet);
	}
	else {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));

		//SCSNetworkTrafficCounterIncreaseDroppedPacketInReceiving(statistics, NULL, 0);
		//SCSNetworkTrafficCounterIncreaseDroppedPacketInReceiving(tmp_statistics, NULL, 0);

		return false;
	}

	if (SCSChannelCheckSocketDescripotr(&self->channel, sd) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s,%d>>", SCSChannelGetProfile(&self->channel), sd);

		//SCSNetworkTrafficCounterIncreaseDroppedPacketInReceiving(statistics, NULL, 0);
		//SCSNetworkTrafficCounterIncreaseDroppedPacketInReceiving(tmp_statistics, NULL, 0);

		SCSPacketDestroy(tmp_packet);
		SCSPacketFree(tmp_packet);

		return false;
	}

	/* Receive */

	SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);
	SCSPointerInitialize(tmp_wp, tmp_buffer, sizeof(tmp_buffer), 0);

	//SCS_PRINT_DEBUG("recvfrom::");

	if (_SCSDatagramSocketRecvInternally(self, &tmp_wp, tmp_timeout) == true) {
		SCSChannelReceive(&self->channel);
	}
	else {
		SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));

		//SCSNetworkTrafficCounterIncreaseDroppedPacketInReceiving(statistics, NULL, 0);
		//SCSNetworkTrafficCounterIncreaseDroppedPacketInReceiving(tmp_statistics, NULL, 0);

		SCSPacketDestroy(tmp_packet);
		SCSPacketFree(tmp_packet);

		return false;
	}

	/* Parse */

	tmp_ptr = SCSPointerGetPointer(tmp_wp);
	tmp_length = SCSPointerGetOffset(tmp_wp);
	SCSPointerInitialize(tmp_rp, tmp_ptr, tmp_length, 0);

	if (SCSParserPacket(&tmp_rp, tmp_packet) == false) {
		//SCS_LOG(WARN, SOCKET, 99999,  "<<%s>>", SCSChannelGetProfile(&self->channel));

		SCSNetworkTrafficCounterIncreaseDroppedPacketInReceiving( //
				statistics, tmp_packet, tmp_length);
		SCSNetworkTrafficCounterIncreaseDroppedPacketInReceiving( //
				tmp_statistics, tmp_packet, tmp_length);

		SCSPacketDestroy(tmp_packet);
		SCSPacketFree(tmp_packet);

		return false;
	}

	if (SCSPacketCheckMode(tmp_packet, self->packet.mode, SCS_PKTMODE_MASKFULL) == false) {
		//SCS_LOG(WARN, SOCKET, 99999,  "<<%s>>", SCSChannelGetProfile(&self->channel));

		SCSNetworkTrafficCounterIncreaseInvalidPacketInReceiving( //
				statistics, tmp_packet, tmp_length);
		SCSNetworkTrafficCounterIncreaseInvalidPacketInReceiving( //
				tmp_statistics, tmp_packet, tmp_length);

		SCSPacketDestroy(tmp_packet);
		SCSPacketFree(tmp_packet);

		return false;
	}

	if (SCSPacketCheckConnectionId(tmp_packet, self->connid) == false) {
		//SCS_LOG(WARN, SOCKET, 99999,  "<<%s>>", SCSChannelGetProfile(&self->channel));

		SCSNetworkTrafficCounterIncreaseInvalidPacketInReceiving( //
				statistics, tmp_packet, tmp_length);
		SCSNetworkTrafficCounterIncreaseInvalidPacketInReceiving( //
				tmp_statistics, tmp_packet, tmp_length);

		SCSPacketDestroy(tmp_packet);
		SCSPacketFree(tmp_packet);

		return false;
	}

	if (SCSPacketVerify(tmp_packet) == false) {
		//SCS_LOG(WARN, SOCKET, 99999,  "<<%s>>", SCSChannelGetProfile(&self->channel));

		SCSNetworkTrafficCounterIncreaseInvalidPacketInReceiving( //
				statistics, tmp_packet, tmp_length);
		SCSNetworkTrafficCounterIncreaseInvalidPacketInReceiving( //
				tmp_statistics, tmp_packet, tmp_length);

		SCSPacketDestroy(tmp_packet);
		SCSPacketFree(tmp_packet);

		return false;
	}

	if (SCSPacketCheckFlags(tmp_packet, SCS_PKTFLAG_RED) == false) {
		//SCS_LOG(NOTICE, DEBUG, 00000, "%"PRIu64", %"PRIu64, //
		//		self->packet.seqno.next, SCSPacketGetSeqno(tmp_packet));

		if (self->packet.seqno.next != SCSPacketGetSeqno(tmp_packet)) {
			bool tmp_positive;

			if (self->packet.seqno.next < SCSPacketGetSeqno(tmp_packet)) {
				tmp_positive = true;
			}
			else {
				SCSPacketSeqno tmp_diff;

				tmp_diff = self->packet.seqno.next - SCSPacketGetSeqno(tmp_packet);
				if (tmp_diff < (self->packet.seqno.maximum >> 1)) {
					tmp_positive = true;
				}
				else {
					tmp_positive = false;
				}
			}

			if (tmp_positive) {
				SCSNetworkTrafficCounterIncreaseUnexpectedPacketInReceiving( //
						statistics, tmp_packet, tmp_length);
				SCSNetworkTrafficCounterIncreaseUnexpectedPacketInReceiving( //
						tmp_statistics, tmp_packet, tmp_length);
			}
			else {
				SCSNetworkTrafficCounterIncreaseReorderingPacketInReceiving( //
						statistics, tmp_packet, tmp_length);
				SCSNetworkTrafficCounterIncreaseReorderingPacketInReceiving( //
						tmp_statistics, tmp_packet, tmp_length);
			}
		}

		self->packet.seqno.next = SCSPacketGetSeqno(tmp_packet) + 1;
		if (self->packet.seqno.maximum < self->packet.seqno.next) {
			self->packet.seqno.next = 0;
		}
	}

	SCSTimespecSetCurrentTime(self->receive.timestamp, CLOCK_MONOTONIC);

	tmp_result = SCSPacketSequencerUpdate(&self->receive.sequencer, SCSPacketGetSeqno(tmp_packet));
	if (tmp_result != SCS_PKTSEQRES_OK) {
		//SCS_LOG(WARN, SOCKET, 99999,  "<<%s>>", SCSChannelGetProfile(&self->channel));

		switch (tmp_result) {
			case SCS_PKTSEQRES_DUPLICATE: {
				SCSNetworkTrafficCounterIncreaseDuplicatedPacketInReceiving( //
						statistics, tmp_packet, tmp_length);
				SCSNetworkTrafficCounterIncreaseDuplicatedPacketInReceiving( //
						tmp_statistics, tmp_packet, tmp_length);
				break;
			}
			case SCS_PKTSEQRES_OUTOFRANGE: {
				SCSNetworkTrafficCounterIncreaseOutOfRangePacketInReceiving( //
						statistics, tmp_packet, tmp_length);
				SCSNetworkTrafficCounterIncreaseOutOfRangePacketInReceiving( //
						tmp_statistics, tmp_packet, tmp_length);
				break;
			}
			case SCS_PKTSEQRES_NG:
			default: {
				SCSNetworkTrafficCounterIncreaseDroppedPacketInReceiving( //
						statistics, tmp_packet, tmp_length);
				SCSNetworkTrafficCounterIncreaseDroppedPacketInReceiving( //
						tmp_statistics, tmp_packet, tmp_length);
				break;
			}
		}

		SCSPacketDestroy(tmp_packet);
		SCSPacketFree(tmp_packet);

		return false;
	}

	if (SCSPacketCheckFlags(tmp_packet, SCS_PKTFLAG_RTT)) {
		_SCSDatagramSocketMeasureRTT(self, tmp_packet, tmp_timestamp);
	}

	if (SCSPMTUMeasurerUpdate(&self->pmtu.measurer, tmp_length) == true) {
		SCS_LOG(NOTICE, SOCKET, 10043, "<<%zu>>", SCSPMTUMeasurerGet(&self->pmtu.measurer));
	}

	if (SCSPacketIsFin(tmp_packet)) {
		if (SCSPacketIsFinAck(tmp_packet)) {
			SCS_LOG(NOTICE, SOCKET, 10030, "<<%s>>", SCSChannelGetProfile(&self->channel));
			SCSTimespecSetCurrentTime(self->close.timeout, CLOCK_MONOTONIC);
		}
		else {
			SCSChannelShutdown(&self->channel, SCS_CHNLSTATE_RECEIVCLOSE);
			SCS_LOG(NOTICE, SOCKET, 10028, "<<%s>>", SCSChannelGetProfile(&self->channel));
			_SCSDatagramSocketSendFinAckPacket(self);
		}
	}

	if (SCSPacketHasPayload(tmp_packet)) {
		if (SCSPacketCheckFlags(tmp_packet, SCS_PKTFLAG_NTY) == false) {
			if (SCSPacketQueueEnqueue(&self->receive.queue, tmp_packet) == true) {
				SCSNetworkTrafficCounterIncreaseReceivedPacket( //
						statistics, tmp_packet, tmp_length);
				SCSNetworkTrafficCounterIncreaseReceivedPacket( //
						tmp_statistics, tmp_packet, tmp_length);
			}
			else {
				//SCS_LOG(WARN, SOCKET, 99999,  "<<%s>>", SCSChannelGetProfile(&self->channel));

				SCSNetworkTrafficCounterIncreaseOverrunPacketInReceiving( //
						statistics, tmp_packet, tmp_length);
				SCSNetworkTrafficCounterIncreaseOverrunPacketInReceiving( //
						tmp_statistics, tmp_packet, tmp_length);

				SCSPacketDestroy(tmp_packet);
				SCSPacketFree(tmp_packet);

				return false;
			}
		}
		else {
			if (SCSFeedbackCallbackConfigValidate(&self->settings.notification.callback)) {
				uint8_t * tmp_data_ptr;
				size_t tmp_data_size;
				uint8_t * tmp_buffer_ptr;
				size_t tmp_buffer_size;

				tmp_data_ptr = SCSPacketGetPayloadPointer(tmp_packet);
				tmp_data_size = SCSPacketGetPayloadSize(tmp_packet);

				if ((tmp_buffer_ptr = self->notification.receive.feedback.extension.ptr) != NULL) {
					tmp_buffer_size = self->notification.receive.feedback.extension.size;

					if (tmp_buffer_size < tmp_data_size) {
						free(tmp_buffer_ptr);

						if ((tmp_buffer_ptr = malloc(tmp_data_size)) != NULL) {
							tmp_buffer_size = tmp_data_size;
						}
						else {
							SCS_LOG(WARN, SOCKET, 99999, "<<%s>>",
									SCSChannelGetProfile(&self->channel));
						}
					}
				}
				else {
					if ((tmp_buffer_ptr = malloc(tmp_data_size)) != NULL) {
						tmp_buffer_size = tmp_data_size;
					}
					else {
						SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));
					}
				}

				if (tmp_buffer_ptr != NULL) {
					memcpy(tmp_buffer_ptr, tmp_data_ptr, tmp_data_size);

					self->notification.receive.feedback.extension.ptr = tmp_buffer_ptr;
					self->notification.receive.feedback.extension.size = tmp_buffer_size;
					self->notification.receive.feedback.extension.length = tmp_data_size;
				}
				else {
					SCS_LOG(WARN, SOCKET, 99999, "<<%s>>", SCSChannelGetProfile(&self->channel));

					self->notification.receive.feedback.extension.ptr = NULL;
					self->notification.receive.feedback.extension.size = 0;
					self->notification.receive.feedback.extension.length = 0;
				}
			}

			SCSPacketDestroy(tmp_packet);
		}
	}
	else {
		SCSNetworkTrafficCounterIncreaseReceivedPacket(statistics, tmp_packet, tmp_length);
		SCSNetworkTrafficCounterIncreaseReceivedPacket(tmp_statistics, tmp_packet, tmp_length);

		SCSPacketDestroy(tmp_packet);
	}

	if (SCSPacketCheckFlags(tmp_packet, SCS_PKTFLAG_NTY)) {
		_SCSDatagramSocketReceiveNotificationPacket(self, tmp_packet, tmp_timestamp);
	}

	/* Clean up */

	SCSPacketFree(tmp_packet);

	return true;
}
bool SCSDatagramSocketReceive(SCSDatagramSocket * __restrict self, int sd,
		SCSNetworkTrafficCounter * __restrict statistics) {
	bool tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (sd == SCS_SKTDESC_INVVAL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (ptr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (length < 1) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

//	if (statistics == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

	_SCS_LOCK(self);

	tmp_result = _SCSDatagramSocketReceive(self, sd, statistics);

	_SCS_UNLOCK(self);

	return tmp_result;
}

void SCSDatagramSocketNotify(SCSDatagramSocket * self) {
	scs_timespec tmp_timestamp;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);

	_SCS_LOCK(self);

	_SCSDatagramSocketSendPathMTUDiscoveryPacket(self, tmp_timestamp);
	_SCSDatagramSocketSendNotificationPacket(self, tmp_timestamp);

	_SCS_UNLOCK(self);

}

static void _SCSDatagramSocketTimeOut(SCSDatagramSocket * self) {
	scs_timespec tmp_last;
	scs_timespec tmp_current;
	scs_timespec tmp_interval;

	if (SCS_SKTSTATE_BROKEN <= self->state) {
		return;
	}

	if (SCSChannelCheckState(&self->channel, SCS_CHNLSTATE_NONE, SCS_CHNLSTATE_CONNECTED)) {
		return;
	}

	if (!SCSChannelCheckState(&self->channel, SCS_CHNLSTATE_NONE,
			(SCS_CHNLSTATE_BROKEN | SCS_CHNLSTATE_CLOSED))) {
		if (SCSChannelCheckState(&self->channel, SCS_CHNLSTATE_BROKEN, SCS_CHNLSTATE_BROKEN)) {
			return;
		}

		if (SCSChannelCheckState(&self->channel, SCS_CHNLSTATE_CLOSED, SCS_CHNLSTATE_CLOSED)) {
			return;
		}

		/* Harf close */
	}

	if (SCSTimespecIsInfinity(self->settings.notification.interval)) {
		return;
	}

	_SCS_LOCK_NOTIFICATION(self);
	SCSTimespecCopy(tmp_last, self->receive.timestamp);
	_SCS_UNLOCK_NOTIFICATION(self);

	SCSTimespecSetCurrentTime(tmp_current, CLOCK_MONOTONIC);
	SCSTimespecSub(tmp_current, tmp_last, tmp_interval);

	if (SCSTimespecCompare(tmp_interval, self->settings.notification.timeout, <)) {
		return;
	}

	_SCSDatagramSocketBroken(self);

	SCS_LOG(NOTICE, SOCKET, 10018, "<<%s>>", SCSChannelGetProfile(&self->channel));

}
void SCSDatagramSocketTimeOut(SCSDatagramSocket * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	_SCS_LOCK(self);
	_SCSDatagramSocketTimeOut(self);
	_SCS_UNLOCK(self);

}

void SCSDatagramSocketClear(SCSDatagramSocket * self, SCSSocketDirection direction) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return;
//	}

	_SCS_LOCK(self);

	switch (direction) {
		case SCS_SKTDRCTN_BOTH: {
			//SCSPacketQueueCleanUp(&self->send.queue.control);
			SCSPacketQueueCleanUp(&self->send.queue.retransmit);
			SCSPacketQueueCleanUp(&self->send.queue.transmit);
			SCSPacketQueueCleanUp(&self->receive.queue);
			break;
		}
		case SCS_SKTDRCTN_SEND: {
			//SCSPacketQueueCleanUp(&self->send.queue.control);
			SCSPacketQueueCleanUp(&self->send.queue.retransmit);
			SCSPacketQueueCleanUp(&self->send.queue.transmit);
			break;
		}
		case SCS_SKTDRCTN_RECV: {
			SCSPacketQueueCleanUp(&self->receive.queue);
			break;
		}
		default: {
			SCS_LOG(WARN, SYSTEM, 99998, "");
			break;
		}
	}

	_SCS_UNLOCK(self);

}

void SCSDatagramSocketShutdown(SCSDatagramSocket * self, SCSSocketDirection direction) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return;
//	}

	_SCS_LOCK(self);

	switch (direction) {
		case SCS_SKTDRCTN_BOTH: {
			self->state = SCS_SKTSTATE_CLOSED;
			break;
		}
		case SCS_SKTDRCTN_SEND: {
			self->state = SCS_SKTSTATE_SENDCLOSED;
			break;
		}
		case SCS_SKTDRCTN_RECV: {
			self->state = SCS_SKTSTATE_RECVCLOSED;
			break;
		}
		default: {
			SCS_LOG(WARN, SYSTEM, 99998, "");
			break;
		}
	}

	_SCS_UNLOCK(self);

}

void SCSDatagramSocketBroken(SCSDatagramSocket * self) {
	bool tmp_retval;
	SCSPacket * tmp_packet;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return;
//	}

	_SCS_LOCK(self);

	_SCSDatagramSocketBroken(self);

	_SCS_UNLOCK(self);

}

bool SCSDatagramSocketClose(SCSDatagramSocket * self) {
	bool tmp_retval;
	SCSPacket * tmp_packet;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return;
//	}

	_SCS_LOCK(self);

	if ((tmp_packet = self->close.packet) == NULL) {
		if ((tmp_retval = _SCSDatagramSocketSendFinPacket(self)) == true) {
			self->close.done = true;
		}
	}
	else {
		tmp_retval = true;
	}

	_SCS_UNLOCK(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

bool SCSDatagramSocketEnablePathMTUDiscovery(SCSDatagramSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	_SCS_LOCK(self);

	self->settings.pmtudisc.enabled = true;

	_SCS_UNLOCK(self);

	return true;
}

bool SCSDatagramSocketDisablePathMTUDiscovery(SCSDatagramSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	_SCS_LOCK(self);

	self->settings.pmtudisc.enabled = false;

	_SCS_UNLOCK(self);

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSDatagramSocketSetId(SCSDatagramSocket * self, SCSSocketId id) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	_SCS_LOCK(self);

	self->id = id;

	_SCS_UNLOCK(self);

}

SCSSocketId SCSDatagramSocketGetId(SCSDatagramSocket * self) {
	SCSSocketId tmp_retval;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return SCS_SKTID_INVVAL;
	}

	_SCS_LOCK(self);

	tmp_retval = self->id;

	_SCS_UNLOCK(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

scs_socket_desc SCSDatagramSocketGetSocketDescriptor(SCSDatagramSocket * self) {
	scs_socket_desc tmp_retval;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return SCS_SKTID_INVVAL;
	}

	_SCS_LOCK(self);

	tmp_retval = SCSChannelGetSocketDescriptor(&self->channel);

	_SCS_UNLOCK(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

bool SCSDatagramSocketGetNetworkTrafficInfo(SCSDatagramSocket * __restrict self,
		SCSNetworkTrafficInfo * __restrict out) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (out == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);

	SCSNetworkTrafficCounterGet(&self->traffic, out);

	_SCS_UNLOCK(self);

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

bool SCSDatagramSocketGetFeedbackInfo(SCSDatagramSocket * __restrict self,
		SCSFeedbackInfo * __restrict out) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (out == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);

	SCSFeedbackInfoCopy(&self->notification.receive.feedback.basic, out);

	_SCS_UNLOCK(self);

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

static bool _SCSDatagramSocketGetExtendedFeedbackInfo(SCSDatagramSocket * __restrict self,
		uint8_t ** __restrict out_ptr, size_t * out_size) {

	if (self->notification.receive.feedback.extension.ptr != NULL) {
		uint8_t * tmp_ptr;
		size_t tmp_size;

		if ((tmp_size = self->notification.receive.feedback.extension.size) < 1) {
			return false;
		}

		if ((tmp_ptr = malloc(tmp_size)) != NULL) {
			memcpy(tmp_ptr, self->notification.receive.feedback.extension.ptr, tmp_size);
		}
		else {
			SCS_LOG(NOTICE, SYSTEM, 00002, "<<%"PRIuS">>", tmp_size);
			return false;
		}

		*out_ptr = tmp_ptr;
		*out_size = tmp_size;
	}
	else {
		*out_ptr = NULL;
		*out_size = 0;
	}

	return true;
}
bool SCSDatagramSocketGetExtendedFeedbackInfo(SCSDatagramSocket * __restrict self,
		uint8_t ** __restrict out_ptr, size_t * __restrict out_size) {
	bool tmp_retval;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (out_ptr == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (out_size == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);

	tmp_retval = _SCSDatagramSocketGetExtendedFeedbackInfo(self, out_ptr, out_size);

	_SCS_UNLOCK(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

uint32_t SCSDatagramSocketGetPLR(SCSDatagramSocket * self) {
	uint32_t tmp_retval;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return 0;
	}

	_SCS_LOCK(self);

	tmp_retval = SCSPLRMeasurerGetLatest(&self->plr.measurer);

	_SCS_UNLOCK(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

scs_timespec SCSDatagramSocketGetRTT(SCSDatagramSocket * self) {
	scs_timespec tmp_retval;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		SCSTimespecInitialize(tmp_retval);
		return tmp_retval;
	}

	_SCS_LOCK(self);

	tmp_retval = SCSRTTMeasurerGetLatest(&self->rtt.measurer);

	_SCS_UNLOCK(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

int SCSDatagramSocketGetMTU(SCSDatagramSocket * self) {
	int tmp_retval;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return -1;
	}

	_SCS_LOCK(self);

	tmp_retval = SCSChannelGetMTU(&self->channel);

	_SCS_UNLOCK(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

int SCSDatagramSocketGetPathMTU(SCSDatagramSocket * self) {
	int tmp_retval;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return -1;
	}

	_SCS_LOCK(self);

	tmp_retval = SCSPMTUMeasurerGet(&self->pmtu.measurer);

	_SCS_UNLOCK(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

bool SCSDatagramSocketSetFeedbackInterval(SCSDatagramSocket * __restrict self,
		scs_timespec * __restrict interval) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (interval == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);
	SCSTimespecCopy(self->settings.notification.interval, *interval);
	_SCS_UNLOCK(self);

	return true;
}

scs_timespec SCSDatagramSocketGetFeedbackInterval(SCSDatagramSocket * self) {
	scs_timespec tmp_retval;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		SCSTimespecSetZero(tmp_retval);
		return tmp_retval;
	}

	_SCS_LOCK(self);
	SCSTimespecCopy(tmp_retval, self->settings.notification.interval);
	_SCS_UNLOCK(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

bool SCSDatagramSocketSetBandwidth(SCSDatagramSocket * __restrict self, uint64_t * __restrict bps) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (bps == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);
	self->settings.bandwidth = *bps;
	_SCS_UNLOCK(self);

	return true;
}

uint64_t SCSDatagramSocketGetBandwidth(SCSDatagramSocket * self) {
	uint64_t tmp_retval;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return ((uint64_t) -1);
	}

	_SCS_LOCK(self);
	tmp_retval = self->settings.bandwidth;
	_SCS_UNLOCK(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

bool SCSDatagramSocketGetBufferStatus(SCSDatagramSocket * __restrict self,
		SCSSocketBufferStatus * __restrict out) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (out == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	_SCS_LOCK(self);

	SCSPacketQueueGetStatus(&self->send.queue.control, &out->send.control);
	SCSPacketQueueGetStatus(&self->send.queue.retransmit, &out->send.retransmit);
	SCSPacketQueueGetStatus(&self->send.queue.transmit, &out->send.transmit);
	SCSPacketQueueGetStatus(&self->receive.queue, &out->receive);

	_SCS_UNLOCK(self);

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

bool SCSDatagramSocketGetInfo(SCSDatagramSocket * __restrict self, SCSSocketInfo * __restrict out) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (out == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	_SCS_LOCK(self);

	SCSChannelGetIfname(&self->channel, out->ifname, SCS_IFNAMESIZE_MAX);
	SCSChannelGetSelfAddr(&self->channel, &out->addr.self);
	SCSChannelGetPeerAddr(&self->channel, &out->addr.peer);
	out->mtu = SCSChannelGetMTU(&self->channel);
	out->connid = self->connid;

	_SCS_UNLOCK(self);

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

static bool _SCSDatagramSocketSetHastyData(SCSDatagramSocket * __restrict self,
		uint8_t * __restrict ptr, size_t size) {

	if (0 < size && ptr != NULL) {
		uint8_t * tmp_ptr;

		_SCS_MEMDUP(ptr, size, tmp_ptr);

		if (self->connect.hasty.ptr != NULL) {
			free(self->connect.hasty.ptr);
		}

		self->connect.hasty.ptr = tmp_ptr;
		self->connect.hasty.size = size;
	}
	else {
		if (self->connect.hasty.ptr != NULL) {
			free(self->connect.hasty.ptr);
		}

		self->connect.hasty.ptr = NULL;
		self->connect.hasty.size = 0;
	}

	return true;
}
bool SCSDatagramSocketSetHastyData(SCSDatagramSocket * __restrict self, uint8_t * __restrict ptr,
		size_t size) {
	bool tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (ptr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (SCS_PACKET_MAXPAYLOADSIZE < size) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	_SCS_LOCK(self);

	tmp_retval = _SCSDatagramSocketSetHastyData(self, ptr, size);

	_SCS_UNLOCK(self);

	return tmp_retval;
}

static bool _SCSDatagramSocketGetHastyData(SCSDatagramSocket * __restrict self,
		uint8_t * __restrict ptr, size_t size) {
	SCSPacket * tmp_packet;
	size_t tmp_size;

	if (SCSChannelIsServer(&self->channel)) {
		tmp_packet = self->connect.packet.syn;
	}
	else {
		tmp_packet = self->connect.packet.synack;
	}

	if (tmp_packet == NULL) {
		return false;
	}

	if (size < (tmp_size = SCSPacketGetPayloadSize(tmp_packet))) {
		tmp_size = size;
	}

	memcpy(ptr, SCSPacketGetPayloadPointer(tmp_packet), tmp_size);

	return true;
}
bool SCSDatagramSocketGetHastyData(SCSDatagramSocket * __restrict self, uint8_t * __restrict ptr,
		size_t size) {
	bool tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	_SCS_LOCK(self);

	tmp_retval = _SCSDatagramSocketGetHastyData(self, ptr, size);

	_SCS_UNLOCK(self);

	return tmp_retval;
}

size_t SCSDatagramSocketGetHastyDataSize(SCSDatagramSocket * self) {
	size_t tmp_retval;
	SCSPacket * tmp_packet;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	_SCS_LOCK(self);

	if (SCSChannelIsServer(&self->channel)) {
		tmp_packet = self->connect.packet.syn;
	}
	else {
		tmp_packet = self->connect.packet.synack;
	}

	if (tmp_packet != NULL) {
		tmp_retval = SCSPacketGetPayloadSize(tmp_packet);
	}
	else {
		tmp_retval = 0;
	}

	_SCS_UNLOCK(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

bool SCSDatagramSocketSetConnectionCallback(SCSDatagramSocket * __restrict self,
		SCSRedundancyCallbackConfig * __restrict config) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	if (SCSRedundancyCallbackConfigValidate(config) == false) {
		return false;
	}

	_SCS_LOCK(self);

	SCSRedundancyCallbackConfigCopy(config, &self->settings.connect.redundancy);

	_SCS_UNLOCK(self);

	return true;
}

bool SCSDatagramSocketSetRedundancyCallback(SCSDatagramSocket * __restrict self,
		SCSRedundancyCallbackConfig * __restrict config) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	if (SCSRedundancyCallbackConfigValidate(config) == false) {
		return false;
	}

	_SCS_LOCK(self);

	SCSRedundancyCallbackConfigCopy(config, &self->settings.send.redundancy);

	_SCS_UNLOCK(self);

	return true;
}

bool SCSDatagramSocketSetFeedbackCallback(SCSDatagramSocket * __restrict self,
		SCSFeedbackCallbackConfig * __restrict config) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	if (SCSFeedbackCallbackConfigValidate(config) == false) {
		return false;
	}

	_SCS_LOCK(self);

	SCSFeedbackCallbackConfigCopy(config, &self->settings.notification.callback);

	_SCS_UNLOCK(self);

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

bool SCSDatagramSocketIsClosing(SCSDatagramSocket * self) {
	bool tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

	_SCS_LOCK(self);

	tmp_retval = self->close.done;

	_SCS_UNLOCK(self);

	return tmp_retval;
}

bool SCSDatagramSocketIsClosed(SCSDatagramSocket * self) {
	bool tmp_retval;
	scs_timespec tmp_current;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

	SCSTimespecSetCurrentTime(tmp_current, CLOCK_MONOTONIC);

	_SCS_LOCK(self);

	if (SCSTimespecCompare(tmp_current, self->close.timeout, <)) {
		tmp_retval = false;
	}
	else {
		tmp_retval = true;
	}

	_SCS_UNLOCK(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

bool SCSDatagramSocketCheckType(SCSDatagramSocket * self, SCSSocketType type) {
	bool tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

	_SCS_LOCK(self);

	tmp_result = (self->type == type ? true : false);

	_SCS_UNLOCK(self);

	return tmp_result;
}

bool SCSDatagramSocketCheckSendable(SCSDatagramSocket * self) {
	bool tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

	_SCS_LOCK(self);

	tmp_retval = _SCSDatagramSocketCheckSendable(self);

	_SCS_UNLOCK(self);

	return tmp_retval;
}

bool SCSDatagramSocketCheckReceivable(SCSDatagramSocket * self) {
	bool tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		SCSSetLastError(SCS_ERRNO_EINVALARGS);
//		return false;
//	}

	_SCS_LOCK(self);

	tmp_retval = _SCSDatagramSocketCheckReceivable(self);

	_SCS_UNLOCK(self);

	return tmp_retval;
}

bool SCSDatagramSocketCheckSelfAddr(SCSDatagramSocket * __restrict self,
		scs_sockaddr * __restrict addr) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);

	tmp_result = SCSChannelCheckSelfAddr(&self->channel, addr);

	_SCS_UNLOCK(self);

	return tmp_result;
}

bool SCSDatagramSocketCheckPeerAddr(SCSDatagramSocket * __restrict self,
		scs_sockaddr * __restrict addr) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);

	tmp_result = SCSChannelCheckPeerAddr(&self->channel, addr);

	_SCS_UNLOCK(self);

	return tmp_result;
}

bool SCSDatagramSocketCheckPayloadLength(SCSDatagramSocket * self, size_t length) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);

	tmp_result = SCSChannelCheckPayloadLength(&self->channel, length);

	_SCS_UNLOCK(self);

	return tmp_result;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSDatagramSocketLoggingStatistics(SCSDatagramSocket * self, SCSLogType type) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	_SCS_LOCK(self);

	SCSNetworkTrafficCounterLogging(&self->traffic, type);
	SCSPacketQueueLogging(&self->send.queue.control, type, "send.control");
	SCSPacketQueueLogging(&self->send.queue.retransmit, type, "send.retransmit");
	SCSPacketQueueLogging(&self->send.queue.transmit, type, "send.transmit");
	SCSPacketQueueLogging(&self->receive.queue, type, "received");

	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSDatagramSocketMonitor(void) {

	SCSObjectCounterMonitor("DatagramSocket");

}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

#undef _SCS_LOCK_NOTIFICATION
#undef _SCS_UNLOCK_NOTIFICATION

#undef _SCS_MEMDUP

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
