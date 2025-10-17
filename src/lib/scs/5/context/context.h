#ifndef SCS_5_CONTEXT_CONTEXT_H_
#define SCS_5_CONTEXT_CONTEXT_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/2/atomic.h"
#include "scs/2/lock.h"
#include "scs/3/worker.h"
#include "scs/5/callback/system/list.h"
#include "scs/5/feature/traffic/counter.h"
#include "scs/5/settings.h"
#include "scs/5/socket/queue.h"
#include "scs/5/socket/table.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSContext {
	scs_atomic_reference reference;
	scs_mutex mutex;
	SCSSettings settings;
	SCSSocketTable sockets;
	SCSNetworkTrafficCounter traiffc;
	SCSCallbackList callbacks;
	struct {
		SCSSocketQueue * queue;
	} send;
	struct {
		scs_socket_desc sd;
	} receive;
	struct {
		SCSSocketQueue * queue;
	} close;
	struct {
		SCSWorker caller;
		SCSWorker closer;
		SCSWorker manager;
		SCSWorker receiver;
		SCSWorker sender;
	} worker;
} SCSContext;

extern void SCSContextInitialize(SCSContext * self);
extern void SCSContextFinalize(SCSContext * self);

/* ---------------------------------------------------------------------------------------------- */

extern SCSContext * SCSContextCreate(void);
extern void SCSContextDestroy(SCSContext * self);

extern bool SCSContextHold(SCSContext * self);
extern void SCSContextFree(SCSContext * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSContextStandBy(SCSContext * __restrict self, SCSSettings * __restrict settings);

extern void SCSContextWaitReady(SCSContext * self);

/* ---------------------------------------------------------------------------------------------- */

extern scs_socket_desc SCSContextGetPollingSocketDescriptor(SCSContext * self);

extern bool SCSContextAddReceivableSocketDescriptor(SCSContext * self, int sd);
extern bool SCSContextRemoveReceivableSocketDescriptor(SCSContext * self, int sd);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSContextMonitor(void);

/* ============================================================================================== */

#endif /* SCS_5_CONTEXT_CONTEXT_H_ */
