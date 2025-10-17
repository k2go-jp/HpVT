#define SCS_SOURCECODE_FILEID	"5CHNCHN"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef WIN32
#else
#include <sys/socket.h>
#endif /* WIN32 */

#include "scs/1/errno.h"
#include "scs/1/in.h"
#include "scs/1/socket.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/1/time.h"
#include "scs/1/unistd.h"
#include "scs/2/logger.h"
#include "scs/5/channel/channel.h"
#include "scs/5/channel/types.h"
#include "scs/5/packet/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_SET_STATE
#error
#endif
#ifdef _SCS_UNSET_STATE
#error
#endif
#ifdef _SCS_GET_STATE
#error
#endif
#ifdef _SCS_CHECK_STATE
#error
#endif

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_SET_STATE(xxx_self, xxx_value) \
		SCSAtomicOr((xxx_self)->state, xxx_value)
#define _SCS_UNSET_STATE(xxx_self, xxx_value) \
		SCSAtomicAnd((xxx_self)->state, ~xxx_value)
#define _SCS_GET_STATE(xxx_self) \
		SCSAtomicGet((xxx_self)->state)
#define _SCS_CHECK_STATE(xxx_self, xxx_value, xxx_mask) \
		((SCSAtomicGet((xxx_self)->state) & xxx_mask) == xxx_value ? true : false)

/* ---------------------------------------------------------------------------------------------- */

static inline void _SCSSetDontFragment(scs_socket_desc sd) {
	int name;

#ifdef WIN32
	char value;

	name = IP_DONTFRAGMENT;
	value = true;
#else
	int value;

	name = IP_MTU_DISCOVER;
	value = IP_PMTUDISC_DO;
#endif

	if (setsockopt(sd, IPPROTO_IP, name, &value, sizeof(value))) {
		SCS_LOG(WARN, SOCKET, 00000, "%s. <<%d>>", strerror(errno), errno);
	}

}

static inline void _SCSSetTimeout(scs_socket_desc sd, int name, scs_time timeout) {
	struct timeval tmp_value;

	if (timeout < 1) {
		return;
	}

	tmp_value.tv_sec = timeout;
	tmp_value.tv_usec = 0;

	if (setsockopt(sd, SOL_SOCKET, name, &tmp_value, sizeof(tmp_value)) != 0) {
		SCS_LOG(WARN, SOCKET, 00000, "%s. <<%d>>", strerror(errno), errno);
	}

}

#define _SCSSetSendTimeout(sd, timeout) \
		_SCSSetTimeout(sd, SO_SNDTIMEO, timeout)

#define _SCSSetReceiveTimeout(sd, timeout) \
		_SCSSetTimeout(sd, SO_RCVTIMEO, timeout)

static inline void _SCSSetReuseAddress(scs_socket_desc sd) {
	int value;

	value = true;

	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) != 0) {
		SCS_LOG(WARN, SOCKET, 00000, "%s. <<%d>>", strerror(errno), errno);
	}

}

static char * _SCSGetNetworkInterfaceName(scs_socket_desc sd) {

	//TODO _SCSGetNetworkInterfaceName

	return NULL;
}

static int _SCSGetMTU(scs_socket_desc sd) {
	/* `sd` must be connected. */

	int tmp_value;
	scs_socklen tmp_size;

	tmp_value = 0;
	tmp_size = sizeof(tmp_value);

	if (getsockopt(sd, IPPROTO_IP, IP_MTU, &tmp_value, &tmp_size) == 0) {
		SCS_LOG(NOTICE, SOCKET, 10044, "<<%d>>", tmp_value);
	}
	else {
		SCS_LOG(WARN, SOCKET, 00000, "%s. <<%d>>", strerror(errno), errno);
	}

	return tmp_value;
}

/* ---------------------------------------------------------------------------------------------- */

static inline bool _SCSChannelOpenSocket(SCSChannel * __restrict self,
		scs_sockaddr * __restrict addr) {
	struct {
		scs_sockaddr self;
		scs_socklen size;
	} tmp_addr;
	SCSChannelConfig tmp_config;
	int tmp_type;
	int tmp_protocol;
	scs_socket_desc tmp_sd;

	memcpy(&tmp_addr.self, addr, sizeof(tmp_addr.self));
	tmp_addr.size = sizeof(tmp_addr.self);

	tmp_config = SCSAtomicGet(self->config);

	switch ((tmp_config & SCS_CHNLCNFG_MASK1)) {
		case SCS_CHNLCNFG_IP: {
			tmp_type = SOCK_RAW;
			if ((tmp_protocol = SCSSockAddrGetProtoclNumber(*addr)) < 0) {
				SCS_LOG(ERROR, SOCKET, 99997, "");
				return false;
			}
			break;
		}
		case SCS_CHNLCNFG_UDP: {
			tmp_type = SOCK_DGRAM;
			tmp_protocol = 0;
			break;
		}
		default: {
			SCS_LOG(ERROR, SOCKET, 99997, "<<0x%02X>>", tmp_config);
			return false;
		}
	}

	if ((tmp_sd = socket(tmp_addr.self.ss_family, tmp_type, htons(tmp_protocol))) == -1) {
		SCS_LOG(ERROR, SOCKET, 00000, "%s. <<%d>>", strerror(errno), errno);
		return false;
	}

	_SCSSetDontFragment(tmp_sd);
	_SCSSetSendTimeout(tmp_sd, 1);
	_SCSSetReceiveTimeout(tmp_sd, 1);
	_SCSSetReuseAddress(tmp_sd);

	if (bind(tmp_sd, (struct sockaddr *) &tmp_addr.self, tmp_addr.size) == -1) {
		char tmp_info[64];
		SCSSockAddrToString(tmp_addr.self, tmp_info, sizeof(tmp_info));
		SCS_LOG(ERROR, SOCKET, 00000, "%s. <<%s,%d>>", strerror(errno), tmp_info, errno);
		close(tmp_sd);
		return false;
	}

	if (getsockname(tmp_sd, (struct sockaddr *) &tmp_addr.self, &tmp_addr.size) != 0) {
		SCS_LOG(ERROR, SOCKET, 00000, "%s. <<%d>>", strerror(errno), errno);
		close(tmp_sd);
		return false;
	}

	switch (tmp_addr.self.ss_family) {
		case AF_INET: {
			tmp_config = SCS_CHNLCNFG_IPv4;
			break;
		}
		case AF_INET6: {
			tmp_config = SCS_CHNLCNFG_IPv6;
			break;
		}
		default: {
			SCS_LOG(ERROR, SOCKET, 99999, "<<%d>>", tmp_addr.self.ss_family);
			close(tmp_sd);
			return false;
		}
	}

	SCSAtomicOr(self->config, tmp_config);
	self->sd = tmp_sd;
	self->self = tmp_addr.self;
	self->ifname = _SCSGetNetworkInterfaceName(tmp_sd);

	return tmp_sd;
}

static inline void _SCSChannelUpdateProfile(SCSChannel * __restrict self,
		__const char * __restrict message) {
	struct {
		char self[64];
		char peer[64];
	} tmp_addr;
	struct {
		SCSChannelState value;
		const char * string;
	} tmp_state;

	tmp_state.value = _SCS_GET_STATE(self);
	tmp_state.string = SCSChannelStateToString(tmp_state.value);

	SCSSockAddrToString(self->self, tmp_addr.self, sizeof(tmp_addr.self));
	SCSSockAddrToString(self->peer, tmp_addr.peer, sizeof(tmp_addr.peer));

	snprintf(self->profile, sizeof(self->profile),	//
			"%s,%s,%d,%08X,%s(%08X)",		//
			tmp_addr.self, 							//
			tmp_addr.peer,		  					//
			self->sd,  								//
			SCSAtomicGet(self->config), 			//
			tmp_state.string,						//
			SCSAtomicGet(self->state));

	SCS_LOG(NOTICE, SOCKET, 00000, "%s<<%s>>", message, self->profile);

}
static inline bool _SCSChannelTransitState(SCSChannel * __restrict self, SCSChannelState state,
		__const char * __restrict message) {

	if (SCSChannelStateValidate(state) == false) {
		SCS_LOG(FATAL, SYSTEM, 99999, "<<%d>>", state);
		return false;
	}

	_SCS_SET_STATE(self, state);
	_SCSChannelUpdateProfile(self, message);

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSChannelInitialize(SCSChannel * self, SCSChannelConfig config) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	memset(self, 0, sizeof(SCSChannel));

	//memset(self->profile, 0, sizeof(profile));
	SCSAtomicInitialize(self->config, config);
	SCSAtomicInitialize(self->state, SCS_CHNLSTATE_NONE);
	self->sd = SCS_SKTDESC_INVVAL;
	SCSSockAddrInitialize(self->self);
	SCSSockAddrInitialize(self->peer);
	SCSTimespecInitialize(self->timestamp.send);
	SCSTimespecInitialize(self->timestamp.receive);
	//self->ifname = NULL;
	//self->mtu = 0;

}
void SCSChannelFinalize(SCSChannel * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	//memset(self->profile, 0, sizeof(profile));
	SCSAtomicFinalize(self->config, SCS_CHNLCNFG_NONE);
	SCSAtomicFinalize(self->state, SCS_CHNLSTATE_NONE);

	if (self->sd != SCS_SKTDESC_INVVAL) {
		shutdown(self->sd, SHUT_RDWR);
		close(self->sd);
		self->sd = SCS_SKTDESC_INVVAL;
	}

	SCSSockAddrFinalize(self->self);
	SCSSockAddrFinalize(self->peer);
	SCSTimespecFinalize(self->timestamp.send);
	SCSTimespecFinalize(self->timestamp.receive);

	if (self->ifname != NULL) {
		free(self->ifname);
		self->ifname = NULL;
	}

	self->mtu = 0;

	memset(self, 0, sizeof(SCSChannel));

}

/* ---------------------------------------------------------------------------------------------- */

bool SCSChannelOpenAsClient(SCSChannel * __restrict self, scs_sockaddr * __restrict addr) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (addr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	if (!_SCS_CHECK_STATE(self, SCS_CHNLSTATE_NONE, SCS_CHNLSTATE_MASKFULL)) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%d>>", SCSChannelGetProfile(self));
		return false;
	}

	if (_SCSChannelOpenSocket(self, addr)) {
		SCSAtomicOr(self->config, SCS_CHNLCNFG_CLIENT);
	}
	else {
		SCS_LOG(WARN, SOCKET, 99999, "<<%d>>", SCSChannelGetProfile(self));
		return false;
	}

	if (_SCSChannelTransitState(self, SCS_CHNLSTATE_BINDED, _SCS_LOGMSG_10011) == false) {
		//close(tmp_sd);
		return false;
	}

	return true;
}
bool SCSChannelOpenAsServer(SCSChannel * __restrict self, scs_sockaddr * __restrict addr) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (addr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	if (!_SCS_CHECK_STATE(self, SCS_CHNLSTATE_NONE, SCS_CHNLSTATE_MASKFULL)) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%d>>", SCSChannelGetProfile(self));
		return false;
	}

	if (_SCSChannelOpenSocket(self, addr)) {
		SCSAtomicOr(self->config, SCS_CHNLCNFG_SERVER);
	}
	else {
		SCS_LOG(WARN, SOCKET, 99999, "<<%d>>", SCSChannelGetProfile(self));
		return false;
	}

	if (_SCSChannelTransitState(self, SCS_CHNLSTATE_LISTEN, _SCS_LOGMSG_10013) == false) {
		SCS_LOG(WARN, SOCKET, 99999, "<<%d>>", SCSChannelGetProfile(self));
		return false;
	}

	return true;
}

bool SCSChannelConnect(SCSChannel * __restrict self, scs_sockaddr * __restrict addr) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (addr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	if (!_SCS_CHECK_STATE(self, SCS_CHNLSTATE_BINDED, SCS_CHNLSTATE_MASKFULL)) {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%s>>", self->profile);
		return false;
	}

	if (self->self.ss_family == addr->ss_family) {
		memcpy(&self->peer, addr, sizeof(self->peer));
	}
	else {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%d,%d>>", self->self.ss_family, addr->ss_family);
		return false;
	}

	if (_SCSChannelTransitState(self, SCS_CHNLSTATE_CONNECTING, _SCS_LOGMSG_10012) == false) {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%s>>", self->profile);
		return false;
	}

	return true;
}
bool SCSChannelConnected(SCSChannel * self) {
	scs_socklen tmp_size;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (addr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	if (!_SCS_CHECK_STATE(self, (SCS_CHNLSTATE_BINDED | SCS_CHNLSTATE_CONNECTING),
			SCS_CHNLSTATE_MASKFULL)) {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%s>>", self->profile);
		return false;
	}

	if (connect(self->sd, (struct sockaddr *) &self->peer, sizeof(self->peer)) == 0) {
		self->mtu = _SCSGetMTU(self->sd);
	}
	else {
		SCS_LOG(ERROR, SOCKET, 00000, "%s. <<%d>>", strerror(errno), errno);
		return false;
	}

	tmp_size = sizeof(self->self);

	if (getsockname(self->sd, (struct sockaddr *) &self->self, &tmp_size) != 0) {
		SCS_LOG(ERROR, SOCKET, 00000, "%s. <<%d>>", strerror(errno), errno);
		return false;
	}

	tmp_size = sizeof(self->peer);

	if (getpeername(self->sd, (struct sockaddr *) &self->peer, &tmp_size) != 0) {
		SCS_LOG(ERROR, SOCKET, 00000, "%s. <<%d>>", strerror(errno), errno);
		return false;
	}

	if (_SCSChannelTransitState(self, SCS_CHNLSTATE_CONNECTED, _SCS_LOGMSG_10015) == false) {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%s>>", self->profile);
		return false;
	}

	return true;
}

bool SCSChannelAccept1(SCSChannel * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	if (!_SCS_CHECK_STATE(self, SCS_CHNLSTATE_LISTEN, SCS_CHNLSTATE_MASKFULL)) {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%s>>", self->profile);
		return false;
	}

	memset(&self->peer, 0, sizeof(self->peer));

	if (_SCSChannelTransitState(self, SCS_CHNLSTATE_CONNECTING, _SCS_LOGMSG_10014) == false) {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%s>>", self->profile);
		return false;
	}

	return true;
}
bool SCSChannelAccept2(SCSChannel * __restrict self, scs_sockaddr * __restrict addr) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	if (!_SCS_CHECK_STATE(self, (SCS_CHNLSTATE_LISTEN | SCS_CHNLSTATE_CONNECTING),
			SCS_CHNLSTATE_MASKFULL)) {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%s>>", self->profile);
		return false;
	}

	if (self->self.ss_family == addr->ss_family) {
		memcpy(&self->peer, addr, sizeof(self->peer));
	}
	else {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%d,%d>>", self->self.ss_family, addr->ss_family);
		return false;
	}

	return true;
}
bool SCSChannelAccepted(SCSChannel * self) {
	scs_socklen tmp_size;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (addr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	if (_SCS_CHECK_STATE(self, (SCS_CHNLSTATE_LISTEN | SCS_CHNLSTATE_CONNECTING),
			SCS_CHNLSTATE_MASKFULL) == false) {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%s>>", self->profile);
		return false;
	}

	if (connect(self->sd, (struct sockaddr *) &self->peer, sizeof(self->peer)) == 0) {
		self->mtu = _SCSGetMTU(self->sd);
	}
	else {
		SCS_LOG(ERROR, SOCKET, 00000, "%s. <<%d>>", strerror(errno), errno);
		return false;
	}

	tmp_size = sizeof(self->self);

	if (getsockname(self->sd, (struct sockaddr *) &self->self, &tmp_size) != 0) {
		SCS_LOG(ERROR, SOCKET, 00000, "%s. <<%d>>", strerror(errno), errno);
		return false;
	}

	tmp_size = sizeof(self->peer);

	if (getpeername(self->sd, (struct sockaddr *) &self->peer, &tmp_size) != 0) {
		SCS_LOG(ERROR, SOCKET, 00000, "%s. <<%d>>", strerror(errno), errno);
		return false;
	}

	if (_SCSChannelTransitState(self, SCS_CHNLSTATE_CONNECTED, _SCS_LOGMSG_10015) == false) {
		SCS_LOG(ERROR, SOCKET, 99999, "<<%s>>", self->profile);
		return false;
	}

	return true;
}

inline void SCSChannelSend(SCSChannel * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	SCSTimespecSetCurrentTime(self->timestamp.send, CLOCK_MONOTONIC);

}
inline void SCSChannelReceive(SCSChannel * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	SCSTimespecSetCurrentTime(self->timestamp.receive, CLOCK_MONOTONIC);

}

inline void SCSChannelBroken(SCSChannel * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	shutdown(self->sd, SHUT_RDWR);
	_SCSChannelTransitState(self, SCS_CHNLSTATE_BROKEN, _SCS_LOGMSG_10016);
}

void SCSChannelShutdown(SCSChannel * self, SCSChannelState state) {
	int tmp_option;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	switch (state) {
		case SCS_CHNLSTATE_SENDCLOSE: {
			tmp_option = SHUT_WR;
			break;
		}
		case SCS_CHNLSTATE_RECEIVCLOSE: {
			tmp_option = SHUT_RD;
			break;
		}
		case SCS_CHNLSTATE_CLOSED: {
			tmp_option = SHUT_RDWR;
			break;
		}
		default: {
			return;
		}
	}

	if (self->sd != SCS_SKTDESC_INVVAL) {
		shutdown(self->sd, tmp_option);
	}

	_SCSChannelTransitState(self, state, _SCS_LOGMSG_10019);

}

void SCSChannelClose(SCSChannel * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	if (self->sd != SCS_SKTDESC_INVVAL) {
		shutdown(self->sd, SHUT_RDWR);
	}

	_SCSChannelTransitState(self, SCS_CHNLSTATE_CLOSED, _SCS_LOGMSG_10017);

}

/* ---------------------------------------------------------------------------------------------- */

inline const char * SCSChannelGetProfile(SCSChannel * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	return self->profile;
}

inline void SCSChannelGetIfname(SCSChannel * __restrict self, char * __restrict out_ptr,
		size_t out_size) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

//	if (out_ptr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	if (self->ifname != NULL) {
		strncpy(out_ptr, self->ifname, out_size);
	}
	else {
		memset(out_ptr, 0, out_size);
	}

}

inline void SCSChannelGetSelfAddr(SCSChannel * __restrict self, scs_sockaddr * __restrict out) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

//	if (out == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSSockAddrCopy(out, &self->self);
}

inline void SCSChannelGetPeerAddr(SCSChannel * __restrict self, scs_sockaddr * __restrict out) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

//	if (out == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSSockAddrCopy(out, &self->peer);
}

SCSPacketMode SCSChannelGetPacketMode(SCSChannel * self, SCSPacketMode initval) {
	SCSPacketMode tmp_value;
	SCSChannelConfig tmp_config;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	tmp_value = initval;
	tmp_config = SCSAtomicGet(self->config);

	switch ((tmp_config & SCS_CHNLCNFG_MASK4)) {
		case SCS_CHNLCNFG_LOWBAND: {
			tmp_value |= SCS_PKTMODE_LOWBAND;
			break;
		}
		case SCS_CHNLCNFG_STANDARD: {
			tmp_value |= SCS_PKTMODE_STANDARD;
			break;
		}
		case SCS_CHNLCNFG_HIGHBAND: {
			tmp_value |= SCS_PKTMODE_HIGHBAND;
			break;
		}
		default: {
			tmp_value |= SCS_PKTMODE_STANDARD;
			break;
		}
	}

	return tmp_value;
}

inline scs_socket_desc SCSChannelGetSocketDescriptor(SCSChannel * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	return self->sd;
}

inline int SCSChannelGetMTU(SCSChannel * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	return self->mtu;
}

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSChannelCheckSendable(SCSChannel * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	return (_SCS_CHECK_STATE(self, SCS_CHNLSTATE_CONNECTED, (SCS_CHNLSTATE_CONNECTED | SCS_CHNLSTATE_SENDCLOSE | SCS_CHNLSTATE_BROKEN)) ?
	true : false);
}

inline bool SCSChannelCheckReceivable(SCSChannel * self) {
	const SCSChannelState tmp_state = //
			(SCS_CHNLSTATE_CONNECTED | SCS_CHNLSTATE_RECEIVCLOSE | SCS_CHNLSTATE_BROKEN);

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	return (_SCS_CHECK_STATE(self, SCS_CHNLSTATE_CONNECTED, tmp_state) ? true : false);
}

inline bool SCSChannelCheckSelfAddr(SCSChannel * __restrict self, scs_sockaddr * __restrict addr) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (addr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	return SCSSockAddrCompare(self->self, *addr);
}

inline bool SCSChannelCheckPeerAddr(SCSChannel * __restrict self, scs_sockaddr * __restrict addr) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (addr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	return SCSSockAddrCompare(self->peer, *addr);
}

bool SCSChannelCheckPayloadLength(SCSChannel * self, size_t length) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	if (0 < self->mtu) {
		if (self->mtu < length) {
			return false;
		}
	}

	return true;
}

inline bool SCSChannelCheckSocketDescripotr(SCSChannel * self, scs_socket_desc sd) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	return (self->sd == sd);
}

inline bool SCSChannelCheckState(SCSChannel * self, SCSChannelState state, SCSChannelState mask) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	return _SCS_CHECK_STATE(self, state, mask);
}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_SET_STATE
#undef _SCS_UNSET_STATE
#undef _SCS_GET_STATE
#undef _SCS_CHECK_STATE

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
