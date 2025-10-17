#define SCS_SOURCECODE_FILEID	"5CLR"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/unistd.h"
#include "scs/2/logger.h"
#include "scs/3/worker.h"
#include "scs/5/context/context.h"
#include "scs/5/defines.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

static inline void __SCSCallerWork(SCSContext * self) {

	while (SCSWorkerIsStarted(&self->worker.caller)) {
		/* Per 1 second */

		SCSCallbackListCall(&self->callbacks);

		MSLEEP(1000);
	}

}
void * _SCSCallerWork(void * argument) {
	SCSContext * tmp_self;

	SCS_LOG(NOTICE, SYSTEM, 10001, "<<%s>>", __func__);

	if ((tmp_self = (SCSContext *) argument) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		return NULL;
	}

	SCSWorkerStarted(&tmp_self->worker.caller);

	__SCSCallerWork(tmp_self);

	SCSWorkerStopped(&tmp_self->worker.caller);

	SCS_LOG(NOTICE, SYSTEM, 10002, "<<%s>>", __func__);

	return NULL;
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
