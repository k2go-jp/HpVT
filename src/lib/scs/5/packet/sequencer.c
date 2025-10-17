#define SCS_SOURCECODE_FILEID	"5PKTSCR"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/inttypes.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/1/unistd.h"
#include "scs/2/logger.h"
#include "scs/5/defines.h"
#include "scs/5/packet/sequencer.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_LOCK
#error
#endif
#ifdef _SCS_UNLOCK
#error
#endif

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_LOCK(xxx_self) \
	if (SCSMutexLock((xxx_self)->mutex) != 0) {\
		SCS_LOG(ALERT, SYSTEM, 99999, "");\
		abort();\
	}
#define _SCS_UNLOCK(xxx_self) \
	if (SCSMutexUnlock((xxx_self)->mutex) != 0) {\
		SCS_LOG(ALERT, SYSTEM, 99999, "");\
		abort();\
	}

/* ---------------------------------------------------------------------------------------------- */

#if SCS_ENABLE_DEBUG == 1
static void SCSPacketSequencerDump(SCSPacketSequencer * self) {
	SCSPacketSequencerEntry * tmp_entry;
	int tmp_num;

	tmp_num = 0;
	for (tmp_entry = self->head; tmp_entry != NULL; tmp_entry = tmp_entry->next) {
		printf("%3d: %"PRIu64"-%"PRIu64"\n", ++tmp_num, tmp_entry->head, tmp_entry->tail);
	}
	printf("\n");
}
#else
#define SCSPacketSequencerDump(xxx_self)
#endif

/* ---------------------------------------------------------------------------------------------- */

static inline void _SCSPacketSeqnoBlockInitialize(SCSPacketSeqnoBlock * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	memset(self, 0, sizeof(SCSPacketSeqnoBlock));

	//self->head = 0;
	//self->tail = 0;

}
static inline void _SCSPacketSeqnoBlockFinalize(SCSPacketSeqnoBlock * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	//self->head = 0;
	//self->tail = 0;

	memset(self, 0, sizeof(SCSPacketSeqnoBlock));

}

/* ---------------------------------------------------------------------------------------------- */

static inline void _SCSPacketSequencerEntryInitialize(SCSPacketSequencerEntry * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	memset(self, 0, sizeof(SCSPacketSequencerEntry));

	//self->head = 0;
	//self->tail = 0;
	//self->next = NULL;

}
/*
 static inline void _SCSPacketSequencerEntryFinalize(SCSPacketSequencerEntry * self) {

 //	if (self == NULL) {
 //		SCS_LOG(WARN, SYSTEM, 99998, "");
 //		return;
 //	}

 //self->head = 0;
 //self->tail = 0;
 //self->next = NULL;

 memset(self, 0, sizeof(SCSPacketSequencerEntry));

 }
 */

static inline void _SCSPacketSequencerEntryReuse(SCSPacketSequencerEntry * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	memset(self, 0, sizeof(SCSPacketSequencerEntry));

	//self->head = 0;
	//self->tail = 0;
	//self->next = NULL;

}

/* ---------------------------------------------------------------------------------------------- */

static inline void _SCSPacketSequencerEnqueueEntry(SCSPacketSequencer * __restrict self,
		SCSPacketSequencerEntry * __restrict entry) {

	_SCSPacketSequencerEntryReuse(entry);
	entry->next = self->queue.head;

	self->queue.head = entry;
	self->queue.count++;

}
static inline void __SCSPacketSequencerDequeueEntry(SCSPacketSequencer * self) {
	int tmp_skip;
	SCSPacketSequencerEntry * tmp_current;
	SCSPacketSequencerEntry * tmp_next;

	if (self->head == NULL) {
		return;
	}

	//SCS_PRINT_DEBUG("head=%p, pool.head=%p(%zu)", self->head, self->queue.head, self->queue.count);

	tmp_current = self->head;

	for (tmp_skip = (self->capacity >> 1); 0 < tmp_skip; tmp_skip--) {
		tmp_current = tmp_current->next;
	}

	while ((tmp_next = tmp_current->next) != NULL) {
		if (tmp_current->head < tmp_next->head) {
			tmp_current->head = tmp_next->head;
		}
		if (tmp_current->tail < tmp_next->tail) {
			tmp_current->tail = tmp_next->tail;
		}
		tmp_current->next = tmp_next->next;

		_SCSPacketSequencerEnqueueEntry(self, tmp_next);
	}

	//SCS_PRINT_DEBUG("head=%p, pool.head=%p(%zu)", self->head, self->queue.head, self->queue.count);
	SCSPacketSequencerDump(self);
}
static SCSPacketSequencerEntry * _SCSPacketSequencerDequeueEntry(SCSPacketSequencer * self) {
	SCSPacketSequencerEntry * tmp_entry;

	if (self->queue.count < 1) {
		__SCSPacketSequencerDequeueEntry(self);
	}

	tmp_entry = self->queue.head;

	self->queue.head = tmp_entry->next;
	self->queue.count--;

	//_SCSPacketSequencerEntryReuse(tmp_entry);
	tmp_entry->next = NULL;

	return tmp_entry;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSPacketSequencerInitialize(SCSPacketSequencer * self) {
	int i;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSPacketSequencer));

	SCSMutexInitialize(self->mutex);
	//self->entries = NULL;
	//self->capacity = 0;
	//self->head = NULL;

	//self->queue.head = NULL;
	//self->queue.count = 0;

	//self->seqno.initial = 0;
	//self->seqno.maximum = 0;

	for (i = 0; i < SCS_PKTSEQUENCER_MAXBLOCKES; i++) {
		_SCSPacketSeqnoBlockInitialize(&self->boundaries.blocks[i]);
	}

	//self->boundary.index.head = 0;
	//self->boundary.index.tail = 0;

}
void SCSPacketSequencerFinalize(SCSPacketSequencer *self) {
	int i;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	SCSMutexFinalize(self->mutex);

	if (self->entries != NULL) {
		free(self->entries);
		self->entries = NULL;
	}

	//self->capacity = 0;
	//self->head = NULL;

	//self->queue.head = NULL;
	//self->queue.count = 0;

	//self->seqno.initial = 0;
	//self->seqno.maximum = 0;

	for (i = 0; i < SCS_PKTSEQUENCER_MAXBLOCKES; i++) {
		_SCSPacketSeqnoBlockFinalize(&self->boundaries.blocks[i]);
	}

	//self->boundary.index.head = 0;
	//self->boundary.index.tail = 0;

	memset(self, 0, sizeof(SCSPacketSequencer));

}

/* ---------------------------------------------------------------------------------------------- */

static inline bool _SCSPacketSequencerStandBy(SCSPacketSequencer * self, SCSPacketMode mode,
		SCSPacketSeqno seqno) {
	struct {
		SCSPacketSeqno initial;
		SCSPacketSeqno maximum;
		SCSPacketSeqno size;
		SCSPacketSeqno value;
	} tmp_seqno;
	struct {
		SCSPacketSeqnoBlock blocks[SCS_PKTSEQUENCER_MAXBLOCKES];
		struct {
			int head;
			int tail;
		} index;
	} tmp_boundaries;
	SCSPacketSequencerEntry * tmp_entries;
	SCSPacketSequencerEntry * tmp_entry;
	int i;

	if ((tmp_seqno.maximum = SCSPacketSeqnoGetMax(mode)) == 0) {
		SCS_LOG(WARN, SYSTEM, 99997, "<<%d>>", mode);
		return false;
	}

	if (seqno <= tmp_seqno.maximum) {
		tmp_seqno.initial = seqno;
	}
	else {
		SCS_LOG(WARN, SYSTEM, 99997, //
				"<<%"PRIu64"/%"PRIu64"(%d)>>", tmp_seqno.initial, tmp_seqno.maximum, mode);
		return false;
	}

	tmp_seqno.size = tmp_seqno.maximum / SCS_PKTSEQUENCER_MAXBLOCKES;
	tmp_seqno.value = 0;

	for (i = 0; i < SCS_PKTSEQUENCER_MAXBLOCKES; i++) {
		tmp_boundaries.blocks[i].head = tmp_seqno.value;
		tmp_seqno.value += tmp_seqno.size;
		tmp_boundaries.blocks[i].tail = tmp_seqno.value;
		tmp_seqno.value++;

		//SCS_PRINT_DEBUG("%d : %zu - %zu", i, tmp_boundaries.blocks[i].head, tmp_boundaries.blocks[i].tail);

		if (tmp_boundaries.blocks[i].head < seqno && seqno < tmp_boundaries.blocks[i].tail) {
			tmp_boundaries.index.head = i;
			tmp_boundaries.index.tail = //
					(i + (SCS_PKTSEQUENCER_MAXBLOCKES >> 1)) % SCS_PKTSEQUENCER_MAXBLOCKES;
		}
	}

	tmp_entries = calloc(sizeof(SCSPacketSequencerEntry),
	SCS_PKTSEQUENCER_MAXENTRIES);
	if (tmp_entries == NULL) {
		SCS_LOG(ERROR, MEMORY, 00002, "<<%zu>>",
				(sizeof(SCSPacketSequencerEntry) * SCS_PKTSEQUENCER_MAXENTRIES));
		return false;
	}

	for (i = 0; i < SCS_PKTSEQUENCER_MAXENTRIES; i++) {
		_SCSPacketSequencerEntryInitialize(&tmp_entries[i]);
		_SCSPacketSequencerEnqueueEntry(self, &tmp_entries[i]);
	}

	tmp_entry = _SCSPacketSequencerDequeueEntry(self);
	tmp_entry->head = seqno;
	tmp_entry->tail = seqno;
	tmp_entry->next = NULL;

	self->entries = tmp_entries;
	self->capacity = SCS_PKTSEQUENCER_MAXENTRIES;
	self->seqno.initial = tmp_seqno.initial;
	self->seqno.maximum = tmp_seqno.maximum;

	for (i = 0; i < SCS_PKTSEQUENCER_MAXBLOCKES; i++) {
		self->boundaries.blocks[i] = tmp_boundaries.blocks[i];
	}

	self->boundaries.index.head = tmp_boundaries.index.head;
	self->boundaries.index.tail = tmp_boundaries.index.tail;
	self->head = tmp_entry;

	//SCS_LOG(NOTICE, DEBUG, 00000, "INIT SEQNO : %"PRIu64, tmp_seqno.initial);

	return true;
}
bool SCSPacketSequencerStandBy(SCSPacketSequencer * self, SCSPacketMode mode, SCSPacketSeqno seqno) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);

	tmp_result = _SCSPacketSequencerStandBy(self, mode, seqno);

	_SCS_UNLOCK(self);

	return tmp_result;
}

static inline SCSPacketSequencerResult __SCSPacketSequencerUpdate1(SCSPacketSequencer * self,
		SCSPacketSeqno seqno) {
	struct {
		SCSPacketSeqno prev;
		SCSPacketSeqno next;
	} tmp_seqno;
	struct {
		SCSPacketSequencerEntry * prev;
		SCSPacketSequencerEntry * ptr;
		SCSPacketSequencerEntry * next;
	} tmp_entry;

	if (0 < seqno) {
		tmp_seqno.prev = seqno - 1;
	}
	else {
		tmp_seqno.prev = 0;
	}

	if (seqno < self->seqno.maximum) {
		tmp_seqno.next = seqno + 1;
	}
	else {
		tmp_seqno.next = self->seqno.maximum;
	}

	tmp_entry.prev = NULL;
	tmp_entry.ptr = NULL;
	tmp_entry.next = NULL;

	for (tmp_entry.ptr = self->head; tmp_entry.ptr != NULL; tmp_entry.ptr = tmp_entry.next) {
		tmp_entry.next = tmp_entry.ptr->next;

		if (tmp_entry.ptr->head <= seqno && seqno <= tmp_entry.ptr->tail) {
			//SCS_PRINT_DEBUG("Duplicate.(%zu)", seqno);
			return SCS_PKTSEQRES_DUPLICATE;
		}

		if (tmp_entry.ptr->tail == tmp_seqno.prev) {
			//SCS_PRINT_DEBUG("Combine.(%zu)", seqno);

			tmp_entry.ptr->tail = seqno;

			return SCS_PKTSEQRES_OK;
		}

		tmp_entry.prev = tmp_entry.ptr;
	}

	//SCS_PRINT_DEBUG("Add.(%zu)", seqno);

	tmp_entry.ptr = _SCSPacketSequencerDequeueEntry(self);
	tmp_entry.ptr->head = seqno;
	tmp_entry.ptr->tail = seqno;

	tmp_entry.ptr->next = self->head;
	self->head = tmp_entry.ptr;

	return SCS_PKTSEQRES_OK;
}
static inline void __SCSPacketSequencerUpdate2(SCSPacketSequencer * self, SCSPacketSeqno seqno) {
	struct {
		struct {
			SCSPacketSeqnoBlock old;
			SCSPacketSeqnoBlock new;
		} head;
		struct {
			SCSPacketSeqnoBlock old;
		//SCSPacketSeqnoBlock new;
		} tail;
	} tmp_block;
	struct {
		SCSPacketSequencerEntry * prev;
		SCSPacketSequencerEntry * ptr;
		SCSPacketSequencerEntry * next;
	} tmp_entry;

	/* Initialize */

	tmp_block.head.old.head = self->boundaries.blocks[self->boundaries.index.head].head;
	tmp_block.head.old.tail = self->boundaries.blocks[self->boundaries.index.head].tail;

	tmp_block.tail.old.head = self->boundaries.blocks[self->boundaries.index.tail].head;
	tmp_block.tail.old.tail = self->boundaries.blocks[self->boundaries.index.tail].tail;

	if (seqno < tmp_block.tail.old.head || tmp_block.tail.old.tail < seqno) {
		return;
	}

	SCSPacketSequencerDump(self);

	self->boundaries.index.head = (self->boundaries.index.head + 1) % SCS_PKTSEQUENCER_MAXBLOCKES;
	self->boundaries.index.tail = (self->boundaries.index.tail + 1) % SCS_PKTSEQUENCER_MAXBLOCKES;

	tmp_block.head.new.head = self->boundaries.blocks[self->boundaries.index.head].head;
	tmp_block.head.new.tail = self->boundaries.blocks[self->boundaries.index.head].tail;

	//tmp_block.tail.new.head = self->boundaries.blocks[self->boundaries.index.tail].head;
	//tmp_block.tail.new.tail = self->boundaries.blocks[self->boundaries.index.tail].tail;

	/* Check to include seqno in tail block */

	//SCS_PRINT_DEBUG("H: %zu - %zu, %zu", tmp_block.head.old.head, tmp_block.head.old.tail, seqno);
	//SCS_PRINT_DEBUG("T: %zu - %zu, %zu", tmp_block.tail.old.head, tmp_block.tail.old.tail, seqno);
	/* Delete */

	tmp_entry.prev = NULL;
	tmp_entry.ptr = NULL;
	tmp_entry.next = NULL;

	for (tmp_entry.ptr = self->head; tmp_entry.ptr != NULL; tmp_entry.ptr = tmp_entry.next) {
		tmp_entry.next = tmp_entry.ptr->next;

		if (tmp_block.head.old.head <= tmp_entry.ptr->head
				&& tmp_entry.ptr->head <= tmp_block.head.old.tail) {
			/* Don't include in head block */

			if (tmp_block.head.old.tail < tmp_entry.ptr->tail) {
				/* Don't include `tail` in head block */

				tmp_entry.ptr->head = tmp_block.head.new.head;
			}
			else {
				/* Include in head block */

				if (tmp_entry.prev != NULL || tmp_entry.next != NULL) {
					if (tmp_entry.prev == NULL) {
						self->head = tmp_entry.next;
					}
					else {
						tmp_entry.prev->next = tmp_entry.next;
					}
					_SCSPacketSequencerEnqueueEntry(self, tmp_entry.ptr);
					tmp_entry.ptr = tmp_entry.prev;
				}
			}
		}
		else {
			/* Don't include in head block */
		}

		tmp_entry.prev = tmp_entry.ptr;
	}

	SCSPacketSequencerDump(self);

}
static inline SCSPacketSequencerResult _SCSPacketSequencerUpdate(SCSPacketSequencer * self,
		SCSPacketSeqno seqno) {
	SCSPacketSeqnoBlock tmp_block;
	SCSPacketSequencerResult tmp_result;

	tmp_block.head = self->boundaries.blocks[self->boundaries.index.head].head;
	tmp_block.tail = self->boundaries.blocks[self->boundaries.index.tail].tail;

	if (tmp_block.head < tmp_block.tail) {
		if (seqno < tmp_block.head || tmp_block.tail < seqno) {
			return SCS_PKTSEQRES_OUTOFRANGE;
		}
	}
	else {
		if (tmp_block.tail < seqno && seqno < tmp_block.head) {
			return SCS_PKTSEQRES_OUTOFRANGE;
		}
	}

	if ((tmp_result = __SCSPacketSequencerUpdate1(self, seqno)) == SCS_PKTSEQRES_OK) {
		__SCSPacketSequencerUpdate2(self, seqno);
	}

	return tmp_result;
}
SCSPacketSequencerResult SCSPacketSequencerUpdate(SCSPacketSequencer * self, SCSPacketSeqno seqno) {
	SCSPacketSequencerResult tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return SCS_PKTSEQRES_NG;
	}

	//SCS_LOG(NOTICE, DEBUG, 00000, "SEQNO : %"PRIu64, seqno);

	_SCS_LOCK(self);

	tmp_result = _SCSPacketSequencerUpdate(self, seqno);

	_SCS_UNLOCK(self);

	return tmp_result;
}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
