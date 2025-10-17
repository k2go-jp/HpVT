#define SCS_SOURCECODE_FILEID	"5PKTWTR"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/5/packet/watcher.h"
#include "scs/2/logger.h"
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

inline void SCSPacketWatcherSettingsInitialize(SCSPacketWatcherSettings * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSPacketWatcherSettings));

	self->capacity = 0;
	self->id.minimum = 0;
	self->id.maximum = 0;

}
inline void SCSPacketWatcherSettingsFinalize(SCSPacketWatcherSettings * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	self->capacity = 0;
	self->id.minimum = 0;
	self->id.maximum = 0;

	memset(self, 0, sizeof(SCSPacketWatcherSettings));

}

inline void SCSPacketWatcherSettingsAdapt(SCSPacketWatcherSettings * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	if (self->capacity < SCS_PKTWATCHER_MINENTRIES) {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%zu(%zu)>>", //
				SCS_PKTWATCHER_MINENTRIES, self->capacity);
		self->capacity = SCS_PKTWATCHER_MINENTRIES;
	}

	if (SCS_PKTWATCHER_MAXENTRIES < self->capacity) {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%zu(%zu)>>", //
				SCS_PKTWATCHER_MAXENTRIES, self->capacity);
		self->capacity = SCS_PKTWATCHER_MAXENTRIES;
	}

}

bool SCSPacketWatcherSettingsValidate(SCSPacketWatcherSettings * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (self->capacity < 1) {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%zu>>", self->capacity);
		return false;
	}

	//if (self->id.minimum < 1) {
	//	SCS_LOG(WARN, SYSTEM, 99999,  "<<%zu>>", self->capacity);
	//	return false;
	//}

	if (self->id.maximum < 1) {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%zu>>", self->capacity);
		return false;
	}

	if (self->id.maximum < self->id.minimum) {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%zu>>", self->capacity);
		return false;
	}

	return true;
}

void SCSPacketWatcherSettingsDump(SCSPacketWatcherSettings * __restrict self,
		__const char * __restrict prefix) {
	const char tmp_caption[] = "watcher.";
	char tmp_prefix[UINT8_MAX];
	char tmp_buffer[UINT8_MAX];

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	if (prefix == NULL) {
		prefix = "";
	}

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s", prefix, tmp_caption);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "capacity", "%zd [packets]",
			self->capacity);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "id.minimum", "%zd", self->id.minimum);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "id.maximum", "%zd", self->id.maximum);

}

/* ---------------------------------------------------------------------------------------------- */

static inline void SCSPacketWatcherEnqueueEntry( //
		SCSPacketWatcher * __restrict self, //
		SCSPacketWatcherEntry * __restrict entry) {

	memset(entry, 0, sizeof(SCSPacketWatcherEntry));
	entry->next = self->queue.head;

	self->queue.head = entry;
	self->queue.count++;

}
static inline void _SCSPacketsequencerDequeueEntry(SCSPacketWatcher * self) {
	SCSPacketWatcherEntry * tmp_current;
	SCSPacketWatcherEntry * tmp_prev;

	tmp_current = self->head;
	tmp_prev = NULL;

	while (tmp_current->next != NULL) {
		tmp_prev = tmp_current;
		tmp_current = tmp_current->next;
	}

	SCSPacketWatcherEnqueueEntry(self, tmp_current);

	if (tmp_prev != NULL) {
		tmp_prev->next = NULL;
	}

}
static SCSPacketWatcherEntry * SCSPacketWatcherDequeueEntry(SCSPacketWatcher * self) {
	SCSPacketWatcherEntry * tmp_entry;

	if (self->queue.count < 1) {
		_SCSPacketsequencerDequeueEntry(self);
	}

	tmp_entry = self->queue.head;

	self->queue.head = tmp_entry->next;
	self->queue.count--;

	memset(tmp_entry, 0, sizeof(SCSPacketWatcherEntry));

	return tmp_entry;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSPacketWatcherInitialize(SCSPacketWatcher * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSPacketWatcher));

	SCSMutexInitialize(self->mutex);
	//self->entries = NULL;
	//self->capacity = 0;
	//self->head = NULL;

	//self->queue.head = NULL;
	//self->queue.count = 0;

	//self->seqno.maximum = 0;
	//self->seqno.current = 0;

}
void SCSPacketWatcherFinalize(SCSPacketWatcher * self) {

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

	//self->seqno.maximum = 0;
	//self->seqno.current = 0;

	memset(self, 0, sizeof(SCSPacketWatcher));

}

static inline bool _SCSPacketWatcherStandBy(	//
		SCSPacketWatcher * __restrict self, //
		SCSPacketWatcherSettings * __restrict settings) {
	SCSPacketWatcherEntry * tmp_entries;
	SCSPacketWatcherId tmp_id;
	size_t i;

	if (settings->capacity < SCS_PKTWATCHER_MINENTRIES || //
			SCS_PKTWATCHER_MAXENTRIES < settings->capacity) {
		SCS_LOG(ERROR, SYSTEM, 99997, "<<%zu, %zu, %zu>>", //
				SCS_PKTWATCHER_MINENTRIES, settings->capacity, SCS_PKTWATCHER_MAXENTRIES);
		return false;
	}

	if (self->capacity != 0) {
		return false;
	}

	if ((tmp_entries = (SCSPacketWatcherEntry *) calloc(settings->capacity,
			sizeof(SCSPacketWatcherEntry))) == NULL) {
		SCS_LOG(ERROR, SYSTEM, 00002, "<<%zu>>",
				(settings->capacity * sizeof(SCSPacketWatcherEntry)));
		return false;
	}

	for (i = 0; i < settings->capacity; i++) {
		SCSPacketWatcherEnqueueEntry(self, &tmp_entries[i]);
	}

	self->entries = tmp_entries;
	self->capacity = settings->capacity;
	self->id.maximum = settings->id.minimum;
	self->id.maximum = settings->id.maximum;
	self->id.current = (
			(tmp_id = time(NULL) % settings->id.maximum) < settings->id.minimum ?
					settings->id.minimum : tmp_id);

	return true;
}
bool SCSPacketWatcherStandBy( //
		SCSPacketWatcher * __restrict self, //
		SCSPacketWatcherSettings * __restrict settings) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (settings == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (SCSPacketWatcherSettingsValidate(settings) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		return false;
	}

	_SCS_LOCK(self);
	tmp_result = _SCSPacketWatcherStandBy(self, settings);
	_SCS_UNLOCK(self);

	return tmp_result;
}

static inline bool _SCSPacketWatcherAdd( //
		SCSPacketWatcher * __restrict self, //
		SCSPacket * __restrict packet, //
		SCSPacketWatcherStatus * __restrict status) {
	SCSPacketWatcherEntry * tmp_entry;

	if ((tmp_entry = SCSPacketWatcherDequeueEntry(self)) != NULL) {
		tmp_entry->status.id = self->id.current;
		tmp_entry->status.seqno = SCSPacketGetSeqno(packet);
		SCSTimespecSetCurrentTime(tmp_entry->status.timestamp, CLOCK_MONOTONIC);
	}
	else {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		return false;
	}

	if (self->head != NULL) {
		tmp_entry->next = self->head;
		self->head = tmp_entry;
	}
	else {
		self->head = tmp_entry;
	}

	self->id.current++;

	if (self->id.current < 1 || self->id.maximum < self->id.current) {
		self->id.current = 1;
	}

	if (status != NULL) {
		SCSPacketWatcherStatusCopy(*status, tmp_entry->status);
	}

	return true;
}
bool SCSPacketWatcherAdd( //
		SCSPacketWatcher * __restrict self, //
		SCSPacket * __restrict packet, //
		SCSPacketWatcherStatus * __restrict status) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);
	tmp_result = _SCSPacketWatcherAdd(self, packet, status);
	_SCS_UNLOCK(self);

	return tmp_result;
}
static inline bool _SCSPacketWatcherUpdate(SCSPacketWatcher * self, SCSPacketWatcherId id) {
	SCSPacketWatcherEntry * tmp_entry;

	if (self->head == NULL) {
		return false;
	}

	for (tmp_entry = self->head; tmp_entry != NULL; tmp_entry = tmp_entry->next) {
		if (tmp_entry->status.id != id) {
			continue;
		}

		SCSTimespecSetCurrentTime(tmp_entry->status.timestamp, CLOCK_MONOTONIC);

		return true;
	}

	return false;
}
bool SCSPacketWatcherUpdate(SCSPacketWatcher * self, SCSPacketWatcherId id) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);
	tmp_result = _SCSPacketWatcherUpdate(self, id);
	_SCS_UNLOCK(self);

	return tmp_result;
}
static inline bool _SCSPacketWatcherRemove( //
		SCSPacketWatcher * __restrict self, //
		SCSPacketWatcherId id, //
		SCSPacketWatcherStatus * __restrict status) {
	struct {
		SCSPacketWatcherEntry * prev;
		SCSPacketWatcherEntry * current;
		SCSPacketWatcherEntry * next;
	} tmp_entry;

	if (self->head == NULL) {
		return false;
	}

	tmp_entry.prev = NULL;
	tmp_entry.current = NULL;
	tmp_entry.next = NULL;

	for (tmp_entry.current = self->head; tmp_entry.current != NULL;
			tmp_entry.current = tmp_entry.next) {
		tmp_entry.next = tmp_entry.current->next;

		if (tmp_entry.current->status.id == id) {

			if (tmp_entry.prev != NULL) {
				tmp_entry.prev->next = tmp_entry.next;
			}
			else {
				self->head = tmp_entry.next;
			}

			if (status != NULL) {
				SCSPacketWatcherStatusCopy(*status, tmp_entry.current->status);
			}

			SCSPacketWatcherEnqueueEntry(self, tmp_entry.current);

			return true;
		}

		tmp_entry.prev = tmp_entry.current;
	}

	return false;
}
bool SCSPacketWatcherRemove( //
		SCSPacketWatcher * __restrict self, //
		SCSPacketWatcherId id, //
		SCSPacketWatcherStatus * __restrict status) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	_SCS_LOCK(self);
	tmp_result = _SCSPacketWatcherRemove(self, id, status);
	_SCS_UNLOCK(self);

	return tmp_result;
}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
