#ifndef SCS_5_SOCKET_QUEUE_H_
#define SCS_5_SOCKET_QUEUE_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/2/atomic.h"
#include "scs/2/lock.h"
#include "scs/5/socket/socket.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSSocketQueue {
	scs_atomic_reference reference;

	scs_mutex mutex;
	struct {
		scs_cond enqueue;
		scs_cond dequeue;
	} cond;
	SCSSocket ** entries;
	size_t capacity;
	size_t count;
	struct {
		size_t head;
		size_t tail;
		size_t last;
	} index;
	struct {
		scs_timespec next;
	} timestamp;
} SCSSocketQueue;

extern void SCSSocketQueueInitialize(SCSSocketQueue * self);
extern void SCSSocketQueueFinalize(SCSSocketQueue * self);

/* ---------------------------------------------------------------------------------------------- */

extern SCSSocketQueue * SCSSocketQueueCreate(size_t capacity);
extern void SCSSocketQueueDestroy(SCSSocketQueue * self);

extern bool SCSSocketQueueHold(SCSSocketQueue * self);
extern void SCSSocketQueueFree(SCSSocketQueue * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSSocketQueueEnqueue(SCSSocketQueue * __restrict self, SCSSocket * __restrict socket);
extern SCSSocket * SCSSocketQueueDequeue(SCSSocketQueue * self);

extern size_t SCSSocketQueueCount(SCSSocketQueue * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSSocketQueueWaitToEnqueue(SCSSocketQueue * self, scs_time timeout_ms);
extern void SCSSocketQueueWaitToDequeue(SCSSocketQueue * self, scs_time timeout_ms);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSSocketQueueCleanUp(SCSSocketQueue * self);

/* ============================================================================================== */

#endif /* SCS_5_SOCKET_QUEUE_H_ */
