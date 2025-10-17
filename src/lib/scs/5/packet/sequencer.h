#ifndef SCS_5_PACKET_SEQUENCER_H_
#define SCS_5_PACKET_SEQUENCER_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/2/lock.h"
#include "scs/5/packet/types.h"

/* ---------------------------------------------------------------------------------------------- */

typedef int SCSPacketSequencerResult;

#define SCS_PKTSEQRES_NG				(-1)
#define SCS_PKTSEQRES_OK				(0)
#define SCS_PKTSEQRES_DUPLICATE			(1)
#define SCS_PKTSEQRES_OUTOFRANGE		(2)

/* ---------------------------------------------------------------------------------------------- */

#define SCS_PKTSEQUENCER_MAXBLOCKES		(16)

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPacketSeqnoBlock {
	SCSPacketSeqno head;
	SCSPacketSeqno tail;
} SCSPacketSeqnoBlock;

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPacketSequencerEntry SCSPacketSequencerEntry;

struct SCSPacketSequencerEntry {
	uint64_t head;
	uint64_t tail;

	SCSPacketSequencerEntry * next;
};

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPacketSequencer {
	scs_mutex mutex;
	SCSPacketSequencerEntry * entries;
	size_t capacity;
	SCSPacketSequencerEntry * head;
	struct {
		SCSPacketSequencerEntry * head;
		size_t count;
	} queue;
	struct {
		SCSPacketSeqno initial;
		SCSPacketSeqno maximum;
	} seqno;
	struct {
		SCSPacketSeqnoBlock blocks[SCS_PKTSEQUENCER_MAXBLOCKES];
		struct {
			int head;
			int tail;
		} index;
	} boundaries;
} SCSPacketSequencer;

extern void SCSPacketSequencerInitialize(SCSPacketSequencer * self);
extern void SCSPacketSequencerFinalize(SCSPacketSequencer * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSPacketSequencerStandBy( 															//
		SCSPacketSequencer * self, 																//
		SCSPacketMode mode, 																	//
		SCSPacketSeqno initial);

extern SCSPacketSequencerResult SCSPacketSequencerUpdate( 										//
		SCSPacketSequencer * self, 																//
		SCSPacketSeqno seqno);

/* ============================================================================================== */

#endif /* SCS_5_PACKET_SEQUENCER_H_ */
