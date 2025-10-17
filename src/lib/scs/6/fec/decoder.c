#define SCS_SOURCECODE_FILEID	"6FECDEC"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/comatibility.h"
#include "scs/1/inttypes.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/2/logger.h"
#include "scs/6/fec/decoder.h"
#include "scs/6/fec/fec.h"
#include "scs/6/fec/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_NULLCHECK(xxx_value, xxx_retval) \
		if (xxx_value == NULL) {\
			return xxx_retval;\
		}

/* ---------------------------------------------------------------------------------------------- */

static void _FECDecoderEntryInitialize(FECDecoderEntry * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	memset(self, 0, sizeof(FECDecoderEntry));

	//self->hdr;
	//self->data = NULL;
	//self->child.next = NULL;
	//self->child.count = 0;
	//self->next = NULL;

}
static void _FECDecoderEntryFinalize(FECDecoderEntry * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	//self->hdr;
	//self->data = NULL;
	//self->child.next = NULL;
	//self->child.count = 0;
	//self->next = NULL;

	memset(self, 0, sizeof(FECDecoderEntry));

}

static void _FECDecoderEntryReuse(FECDecoderEntry * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	//self->sid = 0;
	//self->flags = 0;
	//self->data = NULL;
	//self->child.next = NULL;
	//self->child.count = 0;
	//self->next = NULL;

	memset(self, 0, sizeof(FECDecoderEntry));

}

static void _FECDecoderEntryPrint(FECDecoderEntry * self) {
	FECDecoderEntry * tmp_entry;
	bool tmp_has_parity;
	bool tmp_can_recovery;
	int tmp_cnt;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	tmp_has_parity = false;
	tmp_cnt = 0;

	for (tmp_entry = self->child.next; tmp_entry != NULL; tmp_entry = tmp_entry->child.next) {
		if (tmp_entry->hdr.flags & FEC_FLAGS_PARITY) {
			tmp_has_parity = true;
		}
		tmp_cnt++;
	}

	if (tmp_has_parity) {
		if (self->hdr.src_cnt == tmp_cnt) {
			tmp_can_recovery = true;
		}
		else if ((self->hdr.src_cnt - 1) == tmp_cnt) {
			tmp_can_recovery = true;
		}
		else {
			tmp_can_recovery = false;
		}
	}
	else {
		tmp_can_recovery = false;
	}

	SCS_PRINT_DEBUG("%-20s : %"PRIu16, "+ Group ID", self->hdr.gid);
	SCS_PRINT_DEBUG("%-20s : %"PRIu16, "+ Set ID", self->hdr.sid);
	SCS_PRINT_DEBUG("%-20s : %"PRIuS, "+ Parity Length", self->hdr.parity_len);
	SCS_PRINT_DEBUG("%-20s : %d/%d", "+ Entries", tmp_cnt, self->hdr.src_cnt);
	SCS_PRINT_DEBUG("%-20s : %s", "+ Has Parity Data", (tmp_has_parity ? "Yes" : "No"));
	SCS_PRINT_DEBUG("%-20s : %s", "+ Can Recovery", (tmp_can_recovery ? "Yes" : "No"));

}

/* ---------------------------------------------------------------------------------------------- */

static inline void _FECDecoderEnqueueEntry(FECDecoder * self, FECDecoderEntry * entry) {

	_FECDecoderEntryReuse(entry);
	entry->next = self->queue.head;

	self->queue.head = entry;
	self->queue.count++;

}
static FECDecoderEntry * _FECDecoderDequeueEntry(FECDecoder * self) {
	FECDecoderEntry * tmp_entry;

	if (self->queue.count < 1) {
		return NULL;
	}

	tmp_entry = self->queue.head;

	self->queue.head = tmp_entry->next;
	self->queue.count--;

	//FECDecoderEntryReuse(tmp_entry);
	tmp_entry->next = NULL;

	return tmp_entry;
}

/* ---------------------------------------------------------------------------------------------- */

void FECDecoderInitialize(FECDecoder * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	memset(self, 0, sizeof(FECDecoder));

	self->gid = FEC_GROUPID_INVVAL;
	//self->entries = NULL;
	//self->capacity = 0;
	//self->queue.head = NULL;
	//self->queue.count = 0;
	//self->head = NULL;
	//self->count = 0;

}
void FECDecoderFinalize(FECDecoder * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	if (0 < self->capacity && self->entries != NULL) {
		size_t i;

		for (i = 0; i < self->capacity; i++) {
			_FECDecoderEntryFinalize(&self->entries[i]);
		}

		free(self->entries);

		self->entries = NULL;
		self->capacity = 0;
	}

	//self->gid = 0;
	//self->queue.head = NULL;
	//self->queue.count = 0;
	//self->head = NULL;
	//self->count = 0;

	memset(self, 0, sizeof(FECDecoder));

}

/* ---------------------------------------------------------------------------------------------- */

bool FECDecoderStandBy(FECDecoder * self, size_t capacity) {
	FECDecoderEntry * tmp_entries;
	size_t i;

	_SCS_NULLCHECK(self, false);

	if ((tmp_entries = calloc(sizeof(FECDecoderEntry), capacity)) == NULL) {
		return false;
	}

	for (i = 0; i < capacity; i++) {
		_FECDecoderEntryInitialize(&tmp_entries[i]);
		_FECDecoderEnqueueEntry(self, &tmp_entries[i]);
	}

	self->entries = tmp_entries;
	self->capacity = capacity;
	self->head = NULL;
	self->count = 0;

	return true;
}

static bool ___FECDecoderAdd(FECDecoder * __restrict self, FECDecoderEntry * entry, FECDecoderEntry * parent) {
	FECDecoderEntry * tmp_entry;
	FECDecoderEntry * tmp_prev;

	tmp_prev = parent;

	for (tmp_entry = parent->child.next; tmp_entry != NULL; tmp_entry = tmp_entry->child.next) {
		tmp_prev = tmp_entry;
	}

	tmp_prev->child.next = entry;

	return true;
}
static bool __FECDecoderAdd(FECDecoder * __restrict self, FECDecoderEntry * entry) {
	FECDecoderEntry * tmp_entry;
	FECDecoderEntry * tmp_prev;

	tmp_prev = NULL;

	for (tmp_entry = self->head; tmp_entry != NULL; tmp_entry = tmp_entry->next) {
		tmp_prev = tmp_entry;

		if (tmp_entry->hdr.gid != entry->hdr.gid) {
			continue;
		}

		if (tmp_entry->hdr.sid != entry->hdr.sid) {
			continue;
		}

		return ___FECDecoderAdd(self, entry, tmp_entry);
	}

	if (tmp_prev != NULL) {
		tmp_prev->next = entry;
	}
	else {
		self->head = entry;
	}

	return true;
}
static bool _FECDecoderAdd(FECDecoder * __restrict self, uint8_t * __restrict ptr, size_t size, FECHeader * __restrict hdr) {
	FECDecoderEntry * tmp_entry;

	if ((tmp_entry = _FECDecoderDequeueEntry(self)) != NULL) {
		memcpy(&tmp_entry->hdr, hdr, sizeof(FECHeader));
		tmp_entry->data.ptr = ptr;
		tmp_entry->data.size = size;
	}
	else {
		return false;
	}

	if (__FECDecoderAdd(self, tmp_entry)) {
		self->count++;
	}
	else {
		_FECDecoderEnqueueEntry(self, tmp_entry);
		return false;
	}

	return true;
}
bool FECDecoderAdd(FECDecoder * __restrict self, uint8_t * __restrict ptr, size_t size) {
	FECHeader tmp_hdr;

	if (FECGetHeader(ptr, size, &tmp_hdr) == false) {
		return false;
	}

	if (self->gid == FEC_GROUPID_INVVAL) {
		self->gid = tmp_hdr.gid;
	}
	else {
		if (self->gid != tmp_hdr.gid) {
			return false;
		}
	}

	if (_FECDecoderAdd(self, ptr, size, &tmp_hdr) == false) {
		return false;
	}

	return true;
}

static void _FECfree(FECData * ptrs, size_t cnt) {
	size_t i;

	for (i = 0; i < cnt; i++) {
		free(ptrs[i].ptr);
		ptrs[i].size = 0;
	}

}
static bool _FECDecoderDecode(FECDecoder * self, FECDecoderEntry * __restrict entry, FECData * __restrict out_data, size_t * __restrict out_cnt) {
	FECDecoderEntry * tmp_entry;
	FECData tmp_entries[FEC_DATANUM_MAX];
	size_t tmp_entry_cnt;
	size_t tmp_cnt;
	FECGroupId tmp_gid;
	FECDecodeResult tmp_result;

	memset(tmp_entries, 0, sizeof(tmp_entries));
	tmp_entry_cnt = 0;
	memset(&tmp_result,0, sizeof(tmp_result));

	for (tmp_entry = entry; tmp_entry != NULL; tmp_entry = tmp_entry->child.next) {
		if (FEC_DATANUM_MAX <= tmp_entry_cnt) {
			return false;
		}

		tmp_entries[tmp_entry_cnt++] = tmp_entry->data;
	}

	tmp_cnt = *out_cnt;
	tmp_gid = FEC_GROUPID_INVVAL;

	if (FECDecode(tmp_entries, tmp_entry_cnt, out_data, &tmp_cnt, &tmp_gid, &tmp_result) == false) {
		return false;
	}

	if (tmp_gid != self->gid) {
		return false;
	}

	*out_cnt = tmp_cnt;

	self->statistics.parity += tmp_result.parity_cnt;
	self->statistics.recovery_success += tmp_result.recovery_success_cnt;
	self->statistics.recovery_failure += tmp_result.recovery_failure_cnt;

	return true;
}
bool FECDecoderDecode(FECDecoder * __restrict self, FECData * __restrict out_data, size_t * __restrict out_cnt) {
	FECDecoderEntry * tmp_entry;
	size_t tmp_out_cnt;
	size_t tmp_cnt;
	size_t tmp_total_cnt;

	_SCS_NULLCHECK(self, false);
	_SCS_NULLCHECK(out_data, false);
	_SCS_NULLCHECK(out_cnt, false);

	if ((tmp_out_cnt = *out_cnt) < 1) {
		return false;
	}

	if (self->head == NULL) {
		return false;
	}

	tmp_total_cnt = 0;

	for (tmp_entry = self->head; tmp_entry != NULL; tmp_entry = tmp_entry->next) {
		if (tmp_total_cnt < tmp_out_cnt) {
			tmp_cnt = tmp_out_cnt - tmp_total_cnt;
		}
		else {
			_FECfree(out_data, tmp_total_cnt);
			return false;
		}

		if (_FECDecoderDecode(self, tmp_entry, &out_data[tmp_total_cnt], &tmp_cnt)) {
			tmp_total_cnt += tmp_cnt;
		}
		else {
			_FECfree(out_data, tmp_total_cnt);
			return false;
		}
	}

	*out_cnt = tmp_total_cnt;

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

static void __FECDecoderCleanUp(FECDecoder * self, FECDecoderEntry * entry) {
	FECDecoderEntry * tmp_entry;
	FECDecoderEntry * tmp_next;

	for (tmp_entry = entry->child.next; tmp_entry != NULL; tmp_entry = tmp_next) {
		tmp_next = tmp_entry->child.next;

		_FECDecoderEnqueueEntry(self, tmp_entry);

		if (0 < self->count) {
			self->count--;
		}
		else {
			abort();
		}
	}

}
static void _FECDecoderCleanUp(FECDecoder * self) {
	FECDecoderEntry * tmp_entry;
	FECDecoderEntry * tmp_next;

	for (tmp_entry = self->head; tmp_entry != NULL; tmp_entry = tmp_next) {
		tmp_next = tmp_entry->next;

		__FECDecoderCleanUp(self, tmp_entry);
		_FECDecoderEnqueueEntry(self, tmp_entry);

		if (0 < self->count) {
			self->count--;
		}
		else {
			abort();
		}
	}

}
void FECDecoderCleanUp(FECDecoder * self) {

	_FECDecoderCleanUp(self);

	if (self->count != 0) {
		abort();
	}

	if (self->queue.count != self->capacity) {
		abort();
	}

	self->gid = FEC_GROUPID_INVVAL;
	self->head = NULL;
	self->count = 0;

	self->statistics.parity = 0;
	self->statistics.recovery_success = 0;
	self->statistics.recovery_failure = 0;
}

/* ---------------------------------------------------------------------------------------------- */

EXTERN void FECDecoderPrint(FECDecoder * self) {
	FECDecoderEntry * tmp_entry;

	SCS_PRINT_DEBUG("%-20s : %"PRIu16, "Group ID", self->gid);
	SCS_PRINT_DEBUG("%-20s : %"PRIuS"/%"PRIuS, "Entries", self->count, self->capacity);

	for (tmp_entry = self->head; tmp_entry != NULL; tmp_entry = tmp_entry->next) {
		_FECDecoderEntryPrint(tmp_entry);
	}

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
