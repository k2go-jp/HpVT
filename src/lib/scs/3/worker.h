#ifndef SCS_3_WORKER_H_
#define SCS_3_WORKER_H_ 1

/* ============================================================================================== */

#include "scs/2/lock.h"
#include "scs/2/thread.h"

/* ---------------------------------------------------------------------------------------------- */

typedef int SCSWorkerState;

#define SCS_WORKERSTATE_NONE		0
#define SCS_WORKERSTATE_STOPPED		0
#define SCS_WORKERSTATE_STARTING	10
#define SCS_WORKERSTATE_STARTED		20
#define SCS_WORKERSTATE_STOPPING	33

extern bool SCSWorkerStateValidate(SCSWorkerState value);

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSWorker {
	scs_mutex mutex;

	scs_thread_desc id;
	SCSWorkerState state;

	int cpu;
} SCSWorker;

extern void SCSWorkerInitialize(SCSWorker * self);
extern void SCSWorkerFinalize(SCSWorker * self);

extern bool SCSWorkerStart(				//
		SCSWorker * __restrict self, 	//
		void * (*func)(void *), 		//
		void * __restrict argument);
extern void SCSWorkerStop(SCSWorker * self);

extern void SCSWorkerStarted(SCSWorker * self);
extern void SCSWorkerStopped(SCSWorker * self);

extern bool SCSWorkerCheckState(SCSWorker * self, SCSWorkerState state);

#define SCSWorkerIsStarted(xxx_self) \
		SCSWorkerCheckState(xxx_self, SCS_WORKERSTATE_STARTED)
#define SCSWorkerIsStopped(xxx_self) \
		SCSWorkerCheckState(xxx_self, SCS_WORKERSTATE_STOPPED)

/* ============================================================================================== */

#endif /* SCS_3_WORKER_H_ */
