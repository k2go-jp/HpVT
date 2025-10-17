#define SCS_SOURCECODE_FILEID	"1TYP"

/* ============================================================================================== */

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef WIN32
#else /* WIN32 */
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#include "scs/1/socket.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

bool _SCSSockAddrCompare(scs_sockaddr * __restrict self, scs_sockaddr * __restrict other) {

	if (self->ss_family != other->ss_family) {
		return false;
	}

	switch (self->ss_family) {
		case AF_INET: {

			if (memcmp(self, other, sizeof(struct sockaddr_in)) != 0) {
				return false;
			}

			break;
		}
		case AF_INET6: {

			if (memcmp(self, other, sizeof(struct sockaddr_in6)) != 0) {
				return false;
			}

			break;
		}
		default: {
			return false;
		}
	}

	return true;
}

scs_protocol_num _SCSSockAddrGetProtoclNumber(scs_sockaddr * self) {
	int tmp_protocol;

	if (self == NULL) {
		return SCS_PROTOCOLNUM_INVVAL;
	}

	switch (self->ss_family) {
		case AF_INET: {
			struct sockaddr_in * tmp_addr;

			tmp_addr = (struct sockaddr_in *) self;
			tmp_protocol = tmp_addr->sin_port;

			break;
		}
		case AF_INET6: {
			struct sockaddr_in6 * tmp_addr;

			tmp_addr = (struct sockaddr_in6 *) self;
			tmp_protocol = tmp_addr->sin6_port;

			break;
		}
		default: {
			return SCS_PROTOCOLNUM_INVVAL;
		}
	}

	if (!SCSProtocolNumValidate(tmp_protocol)) {
		return SCS_PROTOCOLNUM_INVVAL;
	}

	switch (tmp_protocol) {
		case IPPROTO_IP:
		case IPPROTO_UDP:
		case IPPROTO_IPV6: {
			return -1;
		}
	}

	return tmp_protocol;
}

void _SCSSockAddrToString(scs_sockaddr * __restrict self, char * __restrict ptr, size_t length) {

	if (self == NULL) {
		return;
	}

	if (ptr == NULL) {
		return;
	}

	if (length < 1) {
		return;
	}

	switch (self->ss_family) {
		case AF_INET: {
			char tmp_addr[16];
			int tmp_port;

			if (inet_ntop(AF_INET, &((struct sockaddr_in *) self)->sin_addr, tmp_addr,
					sizeof(struct sockaddr_in)) == NULL) {
				sprintf(tmp_addr, "*");
			}

			tmp_port = ntohs(((struct sockaddr_in *) self)->sin_port);

			snprintf(ptr, length, "%s:%d", tmp_addr, tmp_port);

			break;
		}
		case AF_INET6: {
			char tmp_addr[40];
			int tmp_port;

			if (inet_ntop(AF_INET6, &((struct sockaddr_in6 *) self)->sin6_addr, tmp_addr,
					sizeof(struct sockaddr_in6)) == NULL) {
				sprintf(tmp_addr, "*");
			}

			tmp_port = ntohs(((struct sockaddr_in6 *) self)->sin6_port);

			snprintf(ptr, length, "[%s]:%d", tmp_addr, tmp_port);

			break;
		}
		default: {
			sprintf(ptr, "*");
			return;
		}
	}

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
