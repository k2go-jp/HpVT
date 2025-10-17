#ifndef SCS_5_PACKET_MONITOR_H_
#define SCS_5_PACKET_MONITOR_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/2/lock.h"
#include "scs/5/packet/types.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSMissingPacketSequence SCSMissingPacketSequence;

struct SCSMissingPacketSequence {
	SCSPacketSeqnoAtomic head;
	SCSPacketSeqnoAtomic tail;
	SCSMissingPacketSequence * next;
};

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPacketSequenceMonitor {
	struct {
		SCSPacketSeqnoAtomic head;
		SCSPacketSeqnoAtomic tail;
		SCSPacketSeqno last;
	} seqno;
	struct {
		scs_mutex mutex;
		SCSMissingPacketSequence * entries;
		size_t count;
	} missing;
} SCSPacketSequenceMonitor;

extern void SCSPacketSequenceMonitorInitialize(SCSPacketSequenceMonitor * self);
extern void SCSPacketSequenceMonitorFinalize(SCSPacketSequenceMonitor * self);

extern void SCSPacketSequenceMonitorUpdate(		//
		SCSPacketSequenceMonitor * self, 		//
		SCSPacketSeqno seqno,					//
		size_t length);

/* ============================================================================================== */

#endif /* SCS_5_PACKET_MONITOR_H_ */
