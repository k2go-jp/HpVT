#define SCS_SOURCECODE_FILEID	"0TYP"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/2/logger.h"
#include "scs/5/channel/types.h"
#include "scs/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

void SCSSocketInfoInitialize(SCSSocketInfo * self) {

	memset(self, 0, sizeof(SCSSocketInfo));

	//self->ifname;
	SCSSockAddrInitialize(self->addr.self);
	SCSSockAddrInitialize(self->addr.peer);
	self->connid = SCS_CONNID_INVVAL;
	//self->mtu = 0;

}
void SCSSocketInfoFinalize(SCSSocketInfo * self) {

	//self->ifname;
	SCSSockAddrFinalize(self->addr.self);
	SCSSockAddrFinalize(self->addr.peer);
	self->connid = SCS_CONNID_INVVAL;
	//self->mtu = 0;

	memset(self, 0, sizeof(SCSSocketInfo));

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
