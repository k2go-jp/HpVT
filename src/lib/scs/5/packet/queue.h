#ifndef SCS_5_PACKET_QUEUE_H_
#define SCS_5_PACKET_QUEUE_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/2/lock.h"
#include "scs/2/logger.h"
#include "scs/5/packet/packet.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPacketQueue {
	scs_mutex mutex;
	struct {
		scs_cond enqueue;
		scs_cond dequeue;
	} cond;
	SCSPacket ** entries;
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
	uint64_t bytes;
} SCSPacketQueue;

extern void SCSPacketQueueInitialize(SCSPacketQueue * self);
extern void SCSPacketQueueFinalize(SCSPacketQueue * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSPacketQueueStandBy(SCSPacketQueue * self, size_t capacity);

extern bool SCSPacketQueueEnqueue(SCSPacketQueue * __restrict self, SCSPacket * __restrict packet);
extern SCSPacket * SCSPacketQueueDequeue(SCSPacketQueue * self);

/* ---------------------------------------------------------------------------------------------- */

extern size_t SCSPacketQueueGetCount(SCSPacketQueue * self);

extern uint64_t SCSPacketQueueGetBytes(SCSPacketQueue * self);

extern void SCSPacketQueueGetStatus(SCSPacketQueue * __restrict self,
		SCSPacketQueueState * __restrict out);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSPacketQueueWaitToEnqueue(SCSPacketQueue * self, scs_time timeout_ms);
extern void SCSPacketQueueWaitToDequeue(SCSPacketQueue * self, scs_time timeout_ms);

/* ---------------------------------------------------------------------------------------------- */

#define SCSPacketQueueIsFull(xxx_self) \
		((xxx_self)->capacity <= (xxx_self)->count ? true : false)

/* ---------------------------------------------------------------------------------------------- */

extern void SCSPacketQueueCleanUp(SCSPacketQueue * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSPacketQueueLogging(SCSPacketQueue * __restrict self, 							//
		SCSLogType type, 																		//
		const char * __restrict prefix);

/* ============================================================================================== */

#endif /* SCS_5_PACKET_QUEUE_H_ */
