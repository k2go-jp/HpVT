#define SCS_SOURCECODE_FILEID	"6FECENC"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/comatibility.h"
#include "scs/1/inttypes.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/2/logger.h"
#include "scs/6/fec/encoder.h"
#include "scs/6/fec/fec.h"
#include "scs/6/fec/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_NULLCHECK(xxx_value, xxx_retval) \
		if (xxx_value == NULL) {\
			return xxx_retval;\
		}

/* ---------------------------------------------------------------------------------------------- */

static void _FECEncoderEntryInitialize(FECEncoderEntry * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	memset(self, 0, sizeof(FECEncoderEntry));

	//self->data = NULL;
	//self->next = NULL;

}

static void _FECEncoderEntryFinalize(FECEncoderEntry * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	//self->data = NULL;
	//self->next = NULL;

	memset(self, 0, sizeof(FECEncoderEntry));

}

static void _FECEncoderEntryReuse(FECEncoderEntry * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	//self->data = NULL;
	//self->next = NULL;

	memset(self, 0, sizeof(FECEncoderEntry));

}

/* ---------------------------------------------------------------------------------------------- */

static inline void _FECEncoderEnqueueEntry(FECEncoder * self, FECEncoderEntry * entry) {

	_FECEncoderEntryReuse(entry);
	entry->next = self->queue.head;

	self->queue.head = entry;
	self->queue.count++;

}

static FECEncoderEntry * _FECEncoderDequeueEntry(FECEncoder * self) {
	FECEncoderEntry * tmp_entry;

	if (self->queue.count < 1) {
		return NULL;
	}

	tmp_entry = self->queue.head;

	self->queue.head = tmp_entry->next;
	self->queue.count--;

	//FECEncoderEntryReuse(tmp_entry);
	tmp_entry->next = NULL;

	return tmp_entry;
}

static size_t _FECEncoderGetNumberOfSources(FECEncoder * self) {
	size_t tmp_num;

	if (self->count < self->level) {
		tmp_num = self->count;
	}
	else {
		size_t tmp_div;

		tmp_div = (self->count / self->level);

		if ((self->count % self->level) != 0) {
			tmp_div++;
		}

		tmp_num = self->count / tmp_div;

		if ((self->count % tmp_div) != 0) {
			tmp_num++;
		}
	}

	if (tmp_num < FEC_DATANUM_MIN) {
		tmp_num = FEC_DATANUM_MIN;
	}

	return tmp_num;
}

/* ---------------------------------------------------------------------------------------------- */

void FECEncoderInitialize(FECEncoder * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	memset(self, 0, sizeof(FECEncoder));

	//self->entries = NULL;
	//self->capacity = 0;
	//self->queue.head = NULL;
	//self->queue.count = 0;
	//self->head = NULL;
	//self->count = 0;

}

void FECEncoderFinalize(FECEncoder * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	if (0 < self->capacity && self->entries != NULL) {
		size_t i;

		for (i = 0; i < self->capacity; i++) {
			_FECEncoderEntryFinalize(&self->entries[i]);
		}

		free(self->entries);

		self->entries = NULL;
		self->capacity = 0;
	}

	//self->queue.head = NULL;
	//self->queue.count = 0;
	//self->head = NULL;
	//self->count = 0;

	memset(self, 0, sizeof(FECEncoder));

}

/* ---------------------------------------------------------------------------------------------- */

bool FECEncoderStandBy(FECEncoder * self, size_t capacity) {
	FECEncoderEntry * tmp_entries;
	size_t i;

	_SCS_NULLCHECK(self, false);

	if ((tmp_entries = calloc(sizeof(FECEncoderEntry), capacity)) == NULL) {
		return false;
	}

	for (i = 0; i < capacity; i++) {
		_FECEncoderEntryInitialize(&tmp_entries[i]);
		_FECEncoderEnqueueEntry(self, &tmp_entries[i]);
	}

	self->entries = tmp_entries;
	self->capacity = capacity;
	self->head = NULL;
	self->count = 0;

	return true;
}

static bool __FECEncoderAdd(FECEncoder * __restrict self, FECEncoderEntry * entry) {
	FECEncoderEntry * tmp_entry;
	FECEncoderEntry * tmp_prev;

	tmp_prev = NULL;

	for (tmp_entry = self->head; tmp_entry != NULL; tmp_entry = tmp_entry->next) {
		tmp_prev = tmp_entry;
	}

	if (tmp_prev != NULL) {
		tmp_prev->next = entry;
	}
	else {
		self->head = entry;
	}

	return true;
}
static bool _FECEncoderAdd(FECEncoder * __restrict self, uint8_t * __restrict ptr, size_t size) {
	FECEncoderEntry * tmp_entry;

	if ((tmp_entry = _FECEncoderDequeueEntry(self)) != NULL) {
		tmp_entry->data.ptr = ptr;
		tmp_entry->data.size = size;
	}
	else {
		return false;
	}

	if (__FECEncoderAdd(self, tmp_entry)) {
		self->count++;
	}
	else {
		_FECEncoderEnqueueEntry(self, tmp_entry);
		return false;
	}

	return true;
}
bool FECEncoderAdd(FECEncoder * __restrict self, uint8_t * __restrict ptr, size_t size) {

	if (size < 1) {
		return false;
	}

	if (FEC_PKTSIZE_MAX < size) {
		return false;
	}

	if (_FECEncoderAdd(self, ptr, size) == false) {
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
bool FECEncoderEncode(FECEncoder * __restrict self, FECData * __restrict out_data, size_t * __restrict out_cnt) {
	size_t tmp_out_cnt;
	size_t tmp_src_num;
	size_t tmp_total_cnt;
	FECData tmp_srcs[FEC_DATANUM_MAX];
	size_t tmp_src_cnt;
	FECEncoderEntry * tmp_entry;
	size_t tmp_cnt;

	_SCS_NULLCHECK(self, false);
	_SCS_NULLCHECK(out_data, false);
	_SCS_NULLCHECK(out_cnt, false);

	if ((tmp_out_cnt = *out_cnt) < self->count) {
		return false;
	}

	if (self->head == NULL) {
		return false;
	}

	if (self->count < FEC_DATANUM_MIN) {
		return false;
	}

	if (FECGroupIdValidate(self->gid) == false) {
		return false;
	}

	if (FECEncodeLevelValidate(self->level) == false) {
		return false;
	}

	tmp_src_num = _FECEncoderGetNumberOfSources(self);

	tmp_total_cnt = 0;
	memset(tmp_srcs, 0, sizeof(tmp_srcs));
	tmp_src_cnt = 0;

	for (tmp_entry = self->head; tmp_entry != NULL; tmp_entry = tmp_entry->next) {
		if (tmp_src_cnt == tmp_src_num) {
			tmp_cnt = tmp_out_cnt - tmp_total_cnt;

			if (FECEncode(self->gid, tmp_srcs, tmp_src_cnt, &out_data[tmp_total_cnt], &tmp_cnt)) {
				tmp_total_cnt += tmp_cnt;
			}
			else {
				_FECfree(out_data, tmp_total_cnt);
				return false;
			}

			memset(tmp_srcs, 0, sizeof(tmp_srcs));
			tmp_src_cnt = 0;
		}

		tmp_srcs[tmp_src_cnt++] = tmp_entry->data;
	}

	if (0 < tmp_src_cnt) {
		if (tmp_out_cnt < (tmp_src_cnt + tmp_total_cnt)) {
			_FECfree(out_data, tmp_total_cnt);
			return false;
		}

		if (tmp_src_cnt < FEC_DATANUM_MIN) {
			int i;

			for (i = 0; i < tmp_src_cnt; i++) {
				if (FECWrapHeader(self->gid, &tmp_srcs[i], &out_data[tmp_total_cnt]) == true) {
					tmp_total_cnt++;
				}
				else {
					_FECfree(out_data, tmp_total_cnt);
					return false;
				}
			}
		}
		else {
			tmp_cnt = tmp_out_cnt - tmp_total_cnt;

			if (FECEncode(self->gid, tmp_srcs, tmp_src_cnt, &out_data[tmp_total_cnt], &tmp_cnt)) {
				tmp_total_cnt += tmp_cnt;
			}
			else {
				_FECfree(out_data, tmp_total_cnt);
				return false;
			}
		}
	}

	*out_cnt = tmp_total_cnt;

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

size_t FECEncoderGetCountWithParities(FECEncoder * self) {
	size_t tmp_cnt;
	size_t tmp_num;

	tmp_num = _FECEncoderGetNumberOfSources(self);
	tmp_cnt = self->count;
	tmp_cnt += self->count / tmp_num;

	if (FEC_DATANUM_MIN <= (self->count % tmp_num)) {
		tmp_cnt++;
	}

	return tmp_cnt;
}

bool FECEncoderSetGroupId(FECEncoder * self, FECGroupId value) {

	_SCS_NULLCHECK(self, false);

	if (FECGroupIdValidate(value)) {
		self->gid = value;
	}
	else {
		return false;
	}

	return true;
}
inline FECGroupId FECEncoderGetGroupId(FECEncoder * self) {

	_SCS_NULLCHECK(self, false);

	return self->gid;
}

bool FECEncoderSetLevel(FECEncoder * self, FECEncodeLevel value) {

	_SCS_NULLCHECK(self, false);

	if (FECEncodeLevelValidate(value)) {
		self->level = value;
	}
	else {
		return false;
	}

	return true;
}
inline FECEncodeLevel FECEncoderGetLevel(FECEncoder * self) {

	_SCS_NULLCHECK(self, false);

	return self->level;
}

/* ---------------------------------------------------------------------------------------------- */

static void _FECEncoderCleanUp(FECEncoder * self) {
	FECEncoderEntry * tmp_entry;
	FECEncoderEntry * tmp_next;

	for (tmp_entry = self->head; tmp_entry != NULL; tmp_entry = tmp_next) {
		tmp_next = tmp_entry->next;

		_FECEncoderEnqueueEntry(self, tmp_entry);

		if (0 < self->count) {
			self->count--;
		}
		else {
			abort();
		}
	}

}
void FECEncoderCleanUp(FECEncoder * self) {

	_FECEncoderCleanUp(self);

	if (self->count != 0) {
		abort();
	}

	if (self->queue.count != self->capacity) {
		abort();
	}

	self->head = NULL;
	self->count = 0;

}

/* ---------------------------------------------------------------------------------------------- */

EXTERN void FECEncoderPrint(FECEncoder * self) {
	//FECEncoderEntry * tmp_entry;

	SCS_PRINT_DEBUG("%-20s : %"PRIu16, "Group ID", self->gid);
	SCS_PRINT_DEBUG("%-20s : %d", "Level", self->level);
	SCS_PRINT_DEBUG("%-20s : %"PRIuS"/%"PRIuS, "Entries", self->count, self->capacity);

	//for (tmp_entry = self->head; tmp_entry != NULL; tmp_entry = tmp_entry->next) {
	//	_FECEncoderEntryPrint(tmp_entry);
	//}

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
