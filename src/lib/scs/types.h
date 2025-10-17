#ifndef SCS_TYPES_H_
#define SCS_TYPES_H_ 1

/* ============================================================================================== */

#include "scs/1/comatibility.h"
#include "scs/1/socket.h"
#include "scs/5/channel/types.h"
#include "scs/5/defines.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSSocketInfo {
	char ifname[(SCS_IFNAMESIZE_MAX + 1)];
	struct {
		scs_sockaddr self;
		scs_sockaddr peer;
	} addr;
	int mtu;
	SCSConnectionId connid;
} SCSSocketInfo;

EXTERN void SCSSocketInfoInitialize(SCSSocketInfo * self);
EXTERN void SCSSocketInfoFinalize(SCSSocketInfo * self);

/* ============================================================================================== */

#endif /* SCS_TYPES_H_ */
