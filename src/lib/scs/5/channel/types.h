#ifndef SCS_5_CHANNEL_TYPES_H_
#define SCS_5_CHANNEL_TYPES_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/socket.h"
#include "scs/1/stdbool.h"
#include "scs/1/time.h"

/* ---------------------------------------------------------------------------------------------- */

typedef int SCSChannelType;

#define SCS_CHNLTYPE_NONE		0
#define SCS_CHNLTYPE_CLIENT		1
#define SCS_CHNLTYPE_SERVER		2

/* ---------------------------------------------------------------------------------------------- */

typedef uint32_t SCSChannelConfig;
typedef scs_atomic_uint32 SCSChannelConfigAtomic;

#define SCS_CHNLCNFG_NONE			(0x00000000)	// 0000 0000 0000 0000 0000 0000 0000 0000

#define SCS_CHNLCNFG_IP				(0x00000001)	// 0000 0000 0000 0000 0000 0000 0000 0001
#define SCS_CHNLCNFG_UDP			(0x00000002)	// 0000 0000 0000 0000 0000 0000 0000 0010

#define SCS_CHNLCNFG_IPv4			(0x00000010)	// 0000 0000 0000 0000 0000 0000 0001 0000
#define SCS_CHNLCNFG_IPv6			(0x00000020)	// 0000 0000 0000 0000 0000 0000 0010 0000

#define SCS_CHNLCNFG_CLIENT			(0x00000100)	// 0000 0000 0000 0000 0000 0001 0000 0000
#define SCS_CHNLCNFG_SERVER			(0x00000200)	// 0000 0000 0000 0000 0000 0010 0000 0000

#define SCS_CHNLCNFG_LOWBAND		(0x00001000)	// 0000 0000 0000 0000 0001 0000 0000 0000
#define SCS_CHNLCNFG_STANDARD		(0x00002000)	// 0000 0000 0000 0000 0010 0000 0000 0000
#define SCS_CHNLCNFG_HIGHBAND		(0x00004000)	// 0000 0000 0000 0000 0100 0000 0000 0000

#define SCS_CHNLCNFG_MASKFULL		(0x000FFFFF)	// 0000 0000 0000 1111 1111 1111 1111 1111
#define SCS_CHNLCNFG_MASK1			(0x0000000F)	// 0000 0000 0000 0000 0000 0000 0000 1111
#define SCS_CHNLCNFG_MASK2			(0x000000F0)	// 0000 0000 0000 0000 0000 0000 1111 0000
#define SCS_CHNLCNFG_MASK3			(0x00000F00)	// 0000 0000 0000 0000 0000 1111 0000 0000
#define SCS_CHNLCNFG_MASK4			(0x0000F000)	// 0000 0000 0000 0000 1111 0000 0000 0000
#define SCS_CHNLCNFG_MASK5			(0x000F0000)	// 0000 0000 0000 1111 0000 0000 0000 0000

extern bool SCSChannelConfigValidate(SCSChannelConfig value);

/* ---------------------------------------------------------------------------------------------- */

typedef uint32_t SCSChannelState;
typedef scs_atomic_int32 SCSChannelStateAtomic;

#define SCS_CHNLSTATE_NONE			(0x00000000)	// 0000 0000 0000 0000 0000 0000 0000 0000
#define SCS_CHNLSTATE_LISTEN		(0x00000001)	// 0000 0000 0000 0000 0000 0000 0000 0001
#define SCS_CHNLSTATE_BINDED		(0x00000002)	// 0000 0000 0000 0000 0000 0000 0000 0010
#define SCS_CHNLSTATE_CONNECTING	(0x00000004)	// 0000 0000 0000 0000 0000 0000 0000 0100
#define SCS_CHNLSTATE_CONNECTED		(0x00000008)	// 0000 0000 0000 0000 0000 0000 0000 1000
#define SCS_CHNLSTATE_RECEIVCLOSE	(0x00000010)	// 0000 0000 0000 0000 0000 0000 0001 0000
#define SCS_CHNLSTATE_SENDCLOSE		(0x00000020)	// 0000 0000 0000 0000 0000 0000 0010 0000
#define SCS_CHNLSTATE_BROKEN		(0x00000040)	// 0000 0000 0000 0000 0000 0000 0100 0000

#define SCS_CHNLSTATE_CLOSED		(SCS_CHNLSTATE_RECEIVCLOSE | SCS_CHNLSTATE_SENDCLOSE)
#define SCS_CHNLSTATE_ALL \
	(SCS_CHNLSTATE_LISTEN | SCS_CHNLSTATE_BINDED | \
	 SCS_CHNLSTATE_CONNECTING | SCS_CHNLSTATE_CONNECTED | \
	 SCS_CHNLSTATE_RECEIVCLOSE | SCS_CHNLSTATE_SENDCLOSE | \
	 SCS_CHNLSTATE_BROKEN)

#define SCS_CHNLSTATE_MASKFULL		(0x000000FF)	// 0000 0000 0000 0000 0000 0000 1111 1111

#define _SCSChannelStateToString(xxx_value) \
		((xxx_value & SCS_CHNLSTATE_SENDCLOSE) == SCS_CHNLSTATE_SENDCLOSE)		? "CONNECTED(RECV-ONLY)" : \
		((xxx_value & SCS_CHNLSTATE_RECEIVCLOSE) == SCS_CHNLSTATE_RECEIVCLOSE)	? "CONNECTED(SEND-ONLY)" : \
		"CONNECTED"
#define SCSChannelStateToString(xxx_value) \
		(xxx_value == SCS_CHNLSTATE_NONE) 										? "INITIALIZED" : \
		((xxx_value & SCS_CHNLSTATE_CLOSED) == SCS_CHNLSTATE_CLOSED)			? "CLOSED" : \
		((xxx_value & SCS_CHNLSTATE_BROKEN) == SCS_CHNLSTATE_BROKEN) 			? "BROKEN" : \
		((xxx_value & SCS_CHNLSTATE_CONNECTED) == SCS_CHNLSTATE_CONNECTED) 		? _SCSChannelStateToString(xxx_value) : \
		((xxx_value & SCS_CHNLSTATE_CONNECTING) == SCS_CHNLSTATE_CONNECTING) 	? "CONNECTING" : \
		((xxx_value & SCS_CHNLSTATE_BINDED) == SCS_CHNLSTATE_BINDED) 			? "BINDED" : \
		((xxx_value & SCS_CHNLSTATE_LISTEN) == SCS_CHNLSTATE_LISTEN) 			? "LISTEN" : \
		"ERROR"

extern bool SCSChannelStateValidate(SCSChannelState value);

/* ---------------------------------------------------------------------------------------------- */

typedef uint32_t SCSConnectionId;

#define SCS_CONNID_INVVAL		(0x00000000)
#define SCS_CONNID_MINVAL		(0x00000001)
#define SCS_CONNID_MAXVAL		(0xFFFFFFFF)

extern bool SCSConnectionIdValidate(SCSConnectionId value);

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSChannel {
	char profile[128];
	SCSChannelConfigAtomic config;
	SCSChannelStateAtomic state;
	scs_socket_desc sd;
	scs_sockaddr self;
	scs_sockaddr peer;
	struct {
		scs_timespec send;
		scs_timespec receive;
	} timestamp;
	char * ifname;
	int mtu;
} SCSChannel;

/* ============================================================================================== */

#endif /* SCS_5_CHANNEL_TYPES_H_ */
