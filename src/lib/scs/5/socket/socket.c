#define SCS_SOURCECODE_FILEID	"5SOCKET"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/1/time.h"
#include "scs/2/logger.h"
#include "scs/5/socket/socket.h"
#include "scs/5/socket/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

inline SCSSocket * SCSSocketCreate(SCSSocketType type, SCSProtocolType protocol) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return (SCSSocket *) SCSDatagramSocketCreate(protocol);
		}
		case SCS_SKTTYPE_STREAM: {
			return NULL;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", type);
			return NULL;
		}
	}

}
inline void SCSSocketDestroy(SCSSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			SCSDatagramSocketDestroy(&self->datagram);
			return;
		}
		case SCS_SKTTYPE_STREAM: {
			return;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return;
		}
	}

}

inline bool SCSSocketHold(SCSSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketHold(&self->datagram);
		}
		case SCS_SKTTYPE_STREAM: {
			return false;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return false;
		}
	}

}
inline void SCSSocketFree(SCSSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			SCSDatagramSocketFree(&self->datagram);
			return;
		}
		case SCS_SKTTYPE_STREAM: {
			return;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return;
		}
	}

}

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSSocketStandBy(SCSSocket * __restrict self, SCSSocketSettings * __restrict settings) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketStandBy(&self->datagram, &settings->datagram);
		}
		case SCS_SKTTYPE_STREAM: {
			return false;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return false;
		}
	}

}

inline bool SCSSocketBind(SCSSocket * __restrict self, scs_sockaddr * __restrict addr) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketBind(&self->datagram, addr);
		}
		case SCS_SKTTYPE_STREAM: {
			return false;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return false;
		}
	}

}

inline bool SCSSocketConnect(SCSSocket * __restrict self, scs_sockaddr * __restrict addr) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketConnect(&self->datagram, addr);
		}
		case SCS_SKTTYPE_STREAM: {
			return false;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return false;
		}
	}

}

inline bool SCSSocketListen(SCSSocket * __restrict self, scs_sockaddr * __restrict addr) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketListen(&self->datagram, addr);
		}
		case SCS_SKTTYPE_STREAM: {
			return false;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return false;
		}
	}

}

inline bool SCSSocketAccept(SCSSocket * __restrict self, scs_sockaddr * __restrict addr) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketAccept(&self->datagram, addr);
		}
		case SCS_SKTTYPE_STREAM: {
			return false;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return false;
		}
	}

}

inline bool SCSSocketSend(SCSSocket * __restrict self,
		SCSNetworkTrafficCounter * __restrict traffic) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketSend(&self->datagram, traffic);
		}
		case SCS_SKTTYPE_STREAM: {
			return false;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return false;
		}
	}

}

inline bool SCSSocketReceive(SCSSocket * __restrict self, int sd,
		SCSNetworkTrafficCounter * __restrict traffic) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketReceive(&self->datagram, sd, traffic);
		}
		case SCS_SKTTYPE_STREAM: {
			return false;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return false;
		}
	}

}

inline void SCSSocketNotify(SCSSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			SCSDatagramSocketNotify(&self->datagram);
			return;
		}
		case SCS_SKTTYPE_STREAM: {
			return;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return;
		}
	}

}

inline void SCSSocketTimeOut(SCSSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			SCSDatagramSocketTimeOut(&self->datagram);
			return;
		}
		case SCS_SKTTYPE_STREAM: {
			return;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return;
		}
	}

}

inline void SCSSocketClear(SCSSocket * self, SCSSocketDirection direction) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketClear(&self->datagram, direction);
		}
		case SCS_SKTTYPE_STREAM: {
			return;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return;
		}
	}

}

inline void SCSSocketShutdown(SCSSocket * self, SCSSocketDirection direction) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			SCSDatagramSocketShutdown(&self->datagram, direction);
			return;
		}
		case SCS_SKTTYPE_STREAM: {
			return;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return;
		}
	}

}

inline void SCSSocketBroken(SCSSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			SCSDatagramSocketBroken(&self->datagram);
			break;
		}
		case SCS_SKTTYPE_STREAM: {
			break;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			break;
		}
	}

}

inline bool SCSSocketClose(SCSSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketClose(&self->datagram);
		}
		case SCS_SKTTYPE_STREAM: {
			return false;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return false;
		}
	}

}

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSSocketEnablePathMTUDiscovery(SCSSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketEnablePathMTUDiscovery(&self->datagram);
		}
		case SCS_SKTTYPE_STREAM: {
			return -1;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return -1;
		}
	}

}
inline bool SCSSocketDisablePathMTUDiscovery(SCSSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketDisablePathMTUDiscovery(&self->datagram);
		}
		case SCS_SKTTYPE_STREAM: {
			return -1;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return -1;
		}
	}

}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSSocketSetId(SCSSocket * self, SCSSocketId id) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketSetId(&self->datagram, id);
		}
		case SCS_SKTTYPE_STREAM: {
			return;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return;
		}
	}

}
inline SCSSocketId SCSSocketGetId(SCSSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketGetId(&self->datagram);
		}
		case SCS_SKTTYPE_STREAM: {
			return SCS_SKTDESC_INVVAL;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return SCS_SKTDESC_INVVAL;
		}
	}

}

inline scs_socket_desc SCSSocketGetSocketDescriptor(SCSSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketGetSocketDescriptor(&self->datagram);
		}
		case SCS_SKTTYPE_STREAM: {
			return SCS_SKTDESC_INVVAL;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return SCS_SKTDESC_INVVAL;
		}
	}

}

inline bool SCSSocketGetNetworkTrafficInfo(SCSSocket * __restrict self,
		SCSNetworkTrafficInfo * __restrict out) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

//	if (out == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketGetNetworkTrafficInfo(&self->datagram, out);
			break;
		}
		case SCS_SKTTYPE_STREAM: {
			return false;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return false;
		}
	}

}

inline bool SCSSocketGetFeedbackInfo(SCSSocket * __restrict self,
		SCSFeedbackInfo * __restrict out) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

//	if (out == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketGetFeedbackInfo(&self->datagram, out);
			break;
		}
		case SCS_SKTTYPE_STREAM: {
			return false;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return false;
		}
	}

}

inline bool SCSSocketGetExtendedFeedbackInfo(SCSSocket * __restrict self,
		uint8_t ** __restrict out_ptr, size_t * __restrict out_size) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

//	if (out == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			SCSDatagramSocketGetExtendedFeedbackInfo(&self->datagram, out_ptr, out_size);
			break;
		}
		case SCS_SKTTYPE_STREAM: {
			return false;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return false;
		}
	}

	return true;
}

inline uint32_t SCSSocketGetPLR(SCSSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketGetPLR(&self->datagram);
			break;
		}
		case SCS_SKTTYPE_STREAM: {
			return 0;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return 0;
		}
	}

}

inline scs_timespec SCSSocketGetRTT(SCSSocket * self) {
	scs_timespec tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			tmp_retval = SCSDatagramSocketGetRTT(&self->datagram);
			break;
		}
		case SCS_SKTTYPE_STREAM: {
			SCSTimespecInitialize(tmp_retval);
			break;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			SCSTimespecInitialize(tmp_retval);
			break;
		}
	}

	return tmp_retval;
}

inline int SCSSocketGetMTU(SCSSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketGetMTU(&self->datagram);
		}
		case SCS_SKTTYPE_STREAM: {
			return -1;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return -1;
		}
	}

}

inline int SCSSocketGetPathMTU(SCSSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketGetPathMTU(&self->datagram);
		}
		case SCS_SKTTYPE_STREAM: {
			return -1;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return -1;
		}
	}

}

inline bool SCSSocketSetFeedbackInterval(SCSSocket * __restrict self,
		scs_timespec * __restrict interval) {
	bool tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			tmp_retval = SCSDatagramSocketSetFeedbackInterval(&self->datagram, interval);
			break;
		}
		case SCS_SKTTYPE_STREAM: {
			break;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			abort();
		}
	}

	return tmp_retval;
}
inline scs_timespec SCSSocketGetFeedbackInterval(SCSSocket * self) {
	scs_timespec tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			tmp_retval = SCSDatagramSocketGetFeedbackInterval(&self->datagram);
			break;
		}
		case SCS_SKTTYPE_STREAM: {
			SCSTimespecSetZero(tmp_retval);
			break;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			abort();
		}
	}

	return tmp_retval;
}

inline bool SCSSocketSetBandwidth(SCSSocket * __restrict self, uint64_t * __restrict bps) {
	bool tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			tmp_retval = SCSDatagramSocketSetBandwidth(&self->datagram, bps);
			break;
		}
		case SCS_SKTTYPE_STREAM: {
			break;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			abort();
		}
	}

	return tmp_retval;
}
inline uint64_t SCSSocketGetBandwidth(SCSSocket * self) {
	uint64_t tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			tmp_retval = SCSDatagramSocketGetBandwidth(&self->datagram);
			break;
		}
		case SCS_SKTTYPE_STREAM: {
			tmp_retval = ((uint64_t) -1);
			break;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			abort();
		}
	}

	return tmp_retval;
}

inline bool SCSSocketGetBufferStatus(SCSSocket * __restrict self,
		SCSSocketBufferStatus * __restrict out) {
	bool tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

//	if (out == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			tmp_retval = SCSDatagramSocketGetBufferStatus(&self->datagram, out);
			break;
		}
		case SCS_SKTTYPE_STREAM: {
			tmp_retval = false;
			break;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			tmp_retval = false;
			break;
		}
	}

	return tmp_retval;
}

inline bool SCSSocketGetInfo(SCSSocket * __restrict self, SCSSocketInfo * __restrict out) {
	bool tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

//	if (out == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			tmp_retval = SCSDatagramSocketGetInfo(&self->datagram, out);
			break;
		}
		case SCS_SKTTYPE_STREAM: {
			tmp_retval = false;
			break;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			tmp_retval = false;
			break;
		}
	}

	return tmp_retval;
}

bool SCSSocketSetHastyData(SCSSocket * __restrict self, uint8_t * __restrict ptr, size_t size) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

//	if (ptr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	if (SCS_PACKET_MAXPAYLOADSIZE < size) {
		SCS_LOG(WARN, SYSTEM, 99998, "<<%"PRIuS">>", size);
		return false;
	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketSetHastyData(&self->datagram, ptr, size);
		}
		case SCS_SKTTYPE_STREAM: {
			return false;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return false;
		}
	}

}
bool SCSSocketGetHastyData(SCSSocket * __restrict self, uint8_t * __restrict ptr, size_t size) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketGetHastyData(&self->datagram, ptr, size);
		}
		case SCS_SKTTYPE_STREAM: {
			return false;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return false;
		}
	}

}
size_t SCSSocketGetHastyDataSize(SCSSocket * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			return SCSDatagramSocketGetHastyDataSize(&self->datagram);
		}
		case SCS_SKTTYPE_STREAM: {
			return false;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return false;
		}
	}

}

/* ---------------------------------------------------------------------------------------------- */

bool SCSSocketSetConnectionCallback(SCSSocket * __restrict self,
		SCSRedundancyCallbackConfig * __restrict config) {

	//	if (self == NULL) {
	//		SCS_LOG(WARN, SYSTEM, 99998, "");
	//		return true;
	//	}

		switch (self->type) {
			case SCS_SKTTYPE_DATAGRAM: {
				return SCSDatagramSocketSetConnectionCallback(&self->datagram, config);
			}
			case SCS_SKTTYPE_STREAM: {
				return false;
			}
			default: {
				SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
				return false;
			}
		}

}

bool SCSSocketSetRedundancyCallback(SCSSocket * __restrict self,
		SCSRedundancyCallbackConfig * __restrict config) {

	//	if (self == NULL) {
	//		SCS_LOG(WARN, SYSTEM, 99998, "");
	//		return true;
	//	}

		switch (self->type) {
			case SCS_SKTTYPE_DATAGRAM: {
				return SCSDatagramSocketSetRedundancyCallback(&self->datagram, config);
			}
			case SCS_SKTTYPE_STREAM: {
				return false;
			}
			default: {
				SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
				return false;
			}
		}

}

bool SCSSocketSetFeedbackCallback(SCSSocket * __restrict self,
		SCSFeedbackCallbackConfig * __restrict config) {

	//	if (self == NULL) {
	//		SCS_LOG(WARN, SYSTEM, 99998, "");
	//		return true;
	//	}

		switch (self->type) {
			case SCS_SKTTYPE_DATAGRAM: {
				return SCSDatagramSocketSetFeedbackCallback(&self->datagram, config);
			}
			case SCS_SKTTYPE_STREAM: {
				return false;
			}
			default: {
				SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
				return false;
			}
		}

}

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSSocketIsClosing(SCSSocket * __restrict self) {
	bool tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			tmp_retval = SCSDatagramSocketIsClosing(&self->datagram);
			break;
		}
		case SCS_SKTTYPE_STREAM: {
			tmp_retval = false;
			break;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			tmp_retval = false;
			break;
		}
	}

	return tmp_retval;
}
inline bool SCSSocketIsClosed(SCSSocket * __restrict self) {
	bool tmp_retval;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			tmp_retval = SCSDatagramSocketIsClosed(&self->datagram);
			break;
		}
		case SCS_SKTTYPE_STREAM: {
			tmp_retval = false;
			break;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			tmp_retval = false;
			break;
		}
	}

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSSocketLoggingStatistics(SCSSocket * self, SCSLogType type) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return true;
//	}

	switch (self->type) {
		case SCS_SKTTYPE_DATAGRAM: {
			SCSDatagramSocketLoggingStatistics(&self->datagram, type);
			return;
		}
		case SCS_SKTTYPE_STREAM: {
			return;
		}
		default: {
			SCS_LOG(ALERT, SYSTEM, 99999, "<<%d>>", self->type);
			return;
		}
	}

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
