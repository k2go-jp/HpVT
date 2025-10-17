#define SCS_SOURCECODE_FILEID	"5PKTMTR"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/1/unistd.h"
#include "scs/2/logger.h"
#include "scs/5/packet/monitor.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_LOCK_MISSING
#error
#endif
#ifdef _SCS_UNLOCK_MISSING
#error
#endif

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_LOCK_MISSING(xxx_self) \
	if (SCSMutexLock((xxx_self)->missing.mutex) != 0) {\
		SCS_LOG(ALERT, SYSTEM, 99999, "");\
		abort();\
	}
#define _SCS_UNLOCK_MISSING(xxx_self) \
	if (SCSMutexUnlock((xxx_self)->missing.mutex) != 0) {\
		SCS_LOG(ALERT, SYSTEM, 99999, "");\
		abort();\
	}

/* ---------------------------------------------------------------------------------------------- */

static void SCSMissingPacketSequenceInitialize(SCSMissingPacketSequence * self) {

	memset(self, 0, sizeof(SCSMissingPacketSequence));

	SCSAtomicInitialize(self->head, 0);
	SCSAtomicInitialize(self->tail, 0);
	//self->next = NULL;

}
static void SCSMissingPacketSequenceFinalize(SCSMissingPacketSequence * self) {

	SCSAtomicFinalize(self->head, 0);
	SCSAtomicFinalize(self->tail, 0);
	//self->next = NULL;

	memset(self, 0, sizeof(SCSMissingPacketSequence));

}

/* ---------------------------------------------------------------------------------------------- */

void SCSPacketSequenceMonitorInitialize(SCSPacketSequenceMonitor * self) {

	memset(self, 0, sizeof(SCSPacketSequenceMonitor));

	SCSAtomicInitialize(self->seqno.head, 0);
	SCSAtomicInitialize(self->seqno.tail, 0);
	self->seqno.last = 0;

	SCSMutexInitialize(self->missing.mutex);
	self->missing.entries = NULL;
	self->missing.count = 0;
}
void SCSPacketSequenceMonitorFinalize(SCSPacketSequenceMonitor * self) {
	SCSMissingPacketSequence * tmp_current;
	SCSMissingPacketSequence * tmp_next;

	_SCS_LOCK_MISSING(self);

	for (tmp_current = self->missing.entries; tmp_current != NULL; tmp_current = tmp_next) {
		tmp_next = tmp_current->next;
		free(tmp_current);
	}

	_SCS_UNLOCK_MISSING(self);

	SCSAtomicFinalize(self->seqno.head, 0);
	SCSAtomicFinalize(self->seqno.tail, 0);
	self->seqno.last = 0;

	SCSMutexFinalize(self->missing.mutex);
	//self->missing.entries = NULL;
	//self->missing.count = 0;

	memset(self, 0, sizeof(SCSPacketSequenceMonitor));
}

void SCSPacketSequenceMonitorUpdate(		//
		SCSPacketSequenceMonitor * self, 	//
		SCSPacketSeqno seqno,				//
		size_t length) {
	struct {
		SCSPacketSeqno head;
		SCSPacketSeqno tail;
	} tmp_seqno;

	tmp_seqno.head = SCSAtomicGet(self->seqno.head);
	tmp_seqno.tail = SCSAtomicGet(self->seqno.tail);

	//TODO Monitoring packet sequence

	if (tmp_seqno.head < tmp_seqno.tail) {

	}
	else {

	}
}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK_MISSING
#undef _SCS_UNLOCK_MISSING

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
