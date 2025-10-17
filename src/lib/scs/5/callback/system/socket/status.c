#define SCS_SOURCECODE_FILEID	"5CBKSYSSCKSTA"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/2/logger.h"
#include "scs/5/callback/system/socket/status.h"
#include "scs/5/callback/system/socket/types.h"
#include "scs/5/socket/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

void SCSSocketCallbackStatusInitialize(SCSSocketCallbackStatus * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	//TODO

	//self->socket.feedback.extension.ptr = NULL;
	//self->socket.feedback.extension.size = 0;

	SCSSocketBufferStateInitalize(&self->socket.buffer);

	memset(self, 0, sizeof(SCSSocketCallbackStatus));

}
void SCSSocketCallbackStatusFinalize(SCSSocketCallbackStatus * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	//TODO

	if (self->socket.feedback.extension.ptr != NULL) {
		free(self->socket.feedback.extension.ptr);
		self->socket.feedback.extension.ptr = NULL;
		self->socket.feedback.extension.size = 0;
	}

	SCSSocketBufferStateFinalize(&self->socket.buffer);

	memset(self, 0, sizeof(SCSSocketCallbackStatus));

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
