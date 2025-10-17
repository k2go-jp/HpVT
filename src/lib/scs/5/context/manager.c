#define SCS_SOURCECODE_FILEID	"5MGR"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/unistd.h"
#include "scs/2/logger.h"
#include "scs/4/gc.h"
#include "scs/5/context/context.h"
#include "scs/5/defines.h"
#include "scs/debug.h"

#define _SCS_CHECKSECOND(xxx_counter) \
		(xxx_counter == (1000 / SCS_MANAGER_INTERVAL))

/* ---------------------------------------------------------------------------------------------- */

static inline void __SCSManagerWork(SCSContext * context) {
	int tmp_num;

	tmp_num = 0;

	while (SCSWorkerIsStarted(&context->worker.manager)) {
		tmp_num++;

		SCSSocketTableNotify(&context->sockets);

		if (_SCS_CHECKSECOND(tmp_num)) {
			tmp_num = 0;

			SCSSocketTableTimeOut(&context->sockets);

			SCSNetworkTrafficCounterLogging(&context->traiffc, SCS_LOGTYPE_SYSTEM);
			SCSSocketTableLoggingStatistics(&context->sockets);

			SCSGarbageCollectorMonitor();
			SCSContextMonitor();
			SCSPacketMonitor();
			SCSDatagramSocketMonitor();
			SCSCallbackConfigMonitor();
		}

		MSLEEP(SCS_MANAGER_INTERVAL);
	}

}
void * _SCSManagerWork(void * argument) {
	SCSContext * tmp_self;

	SCS_LOG(NOTICE, SYSTEM, 10001, "<<%s>>", __func__);

	if ((tmp_self = (SCSContext *) argument) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		return NULL;
	}

	SCSWorkerStarted(&tmp_self->worker.manager);

	__SCSManagerWork(tmp_self);

	SCSWorkerStopped(&tmp_self->worker.manager);

	SCS_LOG(NOTICE, SYSTEM, 10002, "<<%s>>", __func__);

	return NULL;
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
