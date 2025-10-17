#ifndef SCS_1_SOCKET_H_
#define SCS_1_SOCKET_H_ 1

/* ============================================================================================== */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#else /* WIN32 */
#include <pthread.h>
#include <sys/socket.h>
#endif

#include "scs/1/stdbool.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef WIN32
typedef SOCKET scs_socket_desc;
#else /* WIN32 */
typedef int scs_socket_desc;
#endif /* WIN32 */

#define SCS_SKTDESC_INVVAL		(-1)
#define SCS_SKTDESC_MINVAL		0
#define SCS_SKTDESC_MAXVAL		INT_MAX

#define SCSSocketDescValidate(xxx_value) \
		((SCS_SKTDESC_MINVAL <= xxx_value) && (xxx_value <= SCS_SKTDESC_MAXVAL))

/* ---------------------------------------------------------------------------------------------- */

typedef int32_t scs_port_num;

#define SCS_PORTNUM_INVVAL		(-1)
#define SCS_PORTNUM_MINVAL		0
#define SCS_PORTNUM_MAXVAL		65535

#define SCSPortNumValidate(xxx_value) \
		((SCS_PORTNUM_MINVAL <= xxx_value) && (xxx_value <= SCS_PORTNUM_MAXVAL))

/* ---------------------------------------------------------------------------------------------- */

typedef int scs_protocol_num;

#define SCS_PROTOCOLNUM_INVVAL		(-1)
#define SCS_PROTOCOLNUM_MINVAL		0
#define SCS_PROTOCOLNUM_MAXVAL		255

#define SCSProtocolNumValidate(xxx_value) \
		((SCS_PROTOCOLNUM_MINVAL <= xxx_value) && (xxx_value <= SCS_PROTOCOLNUM_MAXVAL))

/* ---------------------------------------------------------------------------------------------- */

#ifdef WIN32
typedef int scs_socklen;
#else
typedef socklen_t scs_socklen;
#endif

/* ---------------------------------------------------------------------------------------------- */

typedef struct sockaddr_storage scs_sockaddr;

#define SCSSockAddrInitialize(xxx_self) \
		memset(&(xxx_self), 0, sizeof(scs_sockaddr))
#define SCSSockAddrFinalize(xxx_self) \
		memset(&(xxx_self), 0, sizeof(scs_sockaddr))

#define SCSSockAddrCompare(xxx_self, xxx_other) \
		_SCSSockAddrCompare(&(xxx_self), &(xxx_other))
#define SCSSockAddrGetProtoclNumber(xxx_self) \
		_SCSSockAddrGetProtoclNumber(&(xxx_self))
#define SCSSockAddrToString(xxx_self, xxx_ptr, xxx_length) \
		_SCSSockAddrToString(&(xxx_self), xxx_ptr, xxx_length)

#define SCSSockAddrCopy(xxx_dst, xxx_src) \
		memcpy(xxx_dst, xxx_src, sizeof(scs_sockaddr))

extern bool _SCSSockAddrCompare(scs_sockaddr * __restrict self, scs_sockaddr * __restrict other);
extern scs_protocol_num _SCSSockAddrGetProtoclNumber(scs_sockaddr * self);
extern void _SCSSockAddrToString(scs_sockaddr * __restrict self, char * __restrict ptr,
		size_t length);

/* ============================================================================================== */

#endif /* SCS_1_SOCKET_H_ */
