#define SCS_SOURCECODE_FILEID	"5SKTTBL"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/1/time.h"
#include "scs/1/unistd.h"
#include "scs/2/logger.h"
#include "scs/5/socket/socket.h"
#include "scs/5/socket/table.h"
#include "scs/5/socket/types.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_LOCK
#error
#endif
#ifdef _SCS_UNLOCK
#error
#endif

#ifdef _SCS_VALIDATE_SOCKDESC
#error
#endif

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_LOCK(xxx_self) \
		if (SCSMutexLock((xxx_self)->mutex) != 0) {\
			SCS_LOG(ALERT, SOCKET, 99999, "");\
			abort();\
		}
#define _SCS_UNLOCK(xxx_self) \
		if (SCSMutexUnlock((xxx_self)->mutex) != 0) {\
			SCS_LOG(ALERT, SOCKET, 99999, "");\
			abort();\
		}

#define _SCS_VALIDATE_SOCKDESC(xxx_value) \
		((xxx_value < 0 || SCS_SOCKETTABLE_MAXINDEXS < xxx_value) ? false : true)

/* ---------------------------------------------------------------------------------------------- */

static inline void SCSSocketTableEntryInitialize(SCSSocketTableEntry * self) {

	memset(self, 0, sizeof(SCSSocketTableEntry));
	//self->socket = NULL;

}

static inline void SCSSocketTableEntryFinalize(SCSSocketTableEntry * self) {
	SCSSocket * tmp_socket;

	self->sd = SCS_SKTDESC_INVVAL;
	if ((tmp_socket = self->socket) != NULL) {
		self->socket = NULL;
		SCSSocketDestroy(tmp_socket);
		SCSSocketFree(tmp_socket);
	}

	memset(self, 0, sizeof(SCSSocketTableEntry));

}

/* ---------------------------------------------------------------------------------------------- */

static void _SCSSocketTableCleanUp(SCSSocketTable * self) {
	size_t i;

	for (i = 0; i < SCS_SOCKETTABLE_MAXENTRIES; i++) {
		SCSSocketTableEntryFinalize(&self->entries[i]);
	}

	for (i = 0; i < SCS_SOCKETTABLE_MAXINDEXS; i++) {
		self->indexes[i] = SCS_SKTID_INVVAL;
	}

	self->count = 0;

	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

void SCSSocketTableInitialize(SCSSocketTable * self) {
	size_t i;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998,);
		return;
	}

	memset(self, 0, sizeof(SCSSocketTable));

	SCSMutexInitialize(self->mutex);

	self->capacity = SCS_SOCKETTABLE_MAXENTRIES;
	self->count = 0;

	for (i = 0; i < SCS_SOCKETTABLE_MAXENTRIES; i++) {
		SCSSocketTableEntryInitialize(&self->entries[i]);
	}

	for (i = 0; i < SCS_SOCKETTABLE_MAXINDEXS; i++) {
		self->indexes[i] = SCS_SKTID_INVVAL;
	}

}

void SCSSocketTableFinalize(SCSSocketTable * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998,);
		return;
	}

	SCSMutexFinalize(self->mutex);

	self->capacity = 0;
	self->count = 0;

	_SCSSocketTableCleanUp(self);
	//self->entries;
	//self->indexes;

	memset(self, 0, sizeof(SCSSocketTable));

}

/* ---------------------------------------------------------------------------------------------- */

static inline bool _SCSSocketTableAdd( //
		SCSSocketTable * __restrict self, //
		SCSSocket * __restrict socket) {
	SCSSocketId tmp_id;

	if (self->capacity <= self->count) {
		SCS_LOG(WARN, SYSTEM, 79994, "<<%zu/%zu>>", self->count, self->capacity);
		return false;
	}

	if (SCSSocketHold(socket) == false) {
		return false;
	}

	for (tmp_id = SCS_SKTID_MINVAL; tmp_id < SCS_SOCKETTABLE_MAXENTRIES; tmp_id++) {
		if (self->entries[tmp_id].socket == NULL) {
			break;
		}
	}

	SCSSocketSetId(socket, tmp_id);

	SCSSocketTableEntryInitialize(&self->entries[tmp_id]);
	self->entries[tmp_id].socket = socket;
	self->count++;

	SCS_LOG(NOTICE, SYSTEM, 00000, "Add a socket. <<%d,%p>>", tmp_id, socket);

	return true;
}
bool SCSSocketTableAdd(SCSSocketTable * __restrict self, SCSSocket * __restrict socket) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998,);
		return false;
	}

	if (socket == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998,);
		return false;
	}

	_SCS_LOCK(self);
	tmp_result = _SCSSocketTableAdd(self, socket);
	_SCS_UNLOCK(self);

	return tmp_result;
}

static inline bool _SCSSocketTableUpdate(SCSSocketTable * self, SCSSocketId id) {
	SCSSocketTableEntry * tmp_entry;
	scs_socket_desc tmp_sd;

	if (self->capacity <= self->count) {
		SCS_LOG(WARN, SYSTEM, 79994, "<<%zu/%zu>>", self->count, self->capacity);
		return false;
	}

	tmp_entry = &self->entries[id];

	if (tmp_entry->socket == NULL) {
		SCS_LOG(WARN, SYSTEM, 99999,);
		return false;
	}

	tmp_sd = SCSSocketGetSocketDescriptor(tmp_entry->socket);

	if (SCSSocketIdValidate(tmp_sd) == true) {
		if (self->indexes[tmp_sd] != SCS_SKTID_INVVAL) {
			SCS_LOG(WARN, SYSTEM, 99999, "<<%d,%d>>", id, self->indexes[tmp_sd]);
			return false;
		}
		self->indexes[tmp_sd] = id;
		tmp_entry->sd = tmp_sd;
	}
	else {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%d>>", tmp_sd);
		return false;
	}

	SCS_LOG(NOTICE, SYSTEM, 00000, "Update a socket. <<%d,%p,%d>>", id, tmp_entry->socket, tmp_sd);

	return true;
}
bool SCSSocketTableUpdate(SCSSocketTable * self, SCSSocketId id) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998,);
		return false;
	}

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 99997, "<<%d>>", id);
		return false;
	}

	_SCS_LOCK(self);
	tmp_result = _SCSSocketTableUpdate(self, id);
	_SCS_UNLOCK(self);

	return tmp_result;
}

static inline bool _SCSSocketTableRemove(SCSSocketTable * self, SCSSocketId id) {
	SCSSocket * tmp_socket;
	scs_socket_desc tmp_sd;

	if (self->count < 1) {
		return false;
	}

	tmp_socket = self->entries[id].socket;
	tmp_sd = SCSSocketGetSocketDescriptor(tmp_socket);

	SCSSocketTableEntryFinalize(&self->entries[id]);
	self->count--;

	if (_SCS_VALIDATE_SOCKDESC(tmp_sd) == true) {
		self->indexes[tmp_sd] = SCS_SKTID_INVVAL;
	}

	SCS_LOG(NOTICE, SYSTEM, 00000, "Remove a socket. <<%p,%d,%d>>", tmp_socket, id, tmp_sd);

	return true;
}
bool SCSSocketTableRemove(SCSSocketTable * self, SCSSocketId id) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998,);
		return false;
	}

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 99997, "<<%d>>", id);
		return false;
	}

	_SCS_LOCK(self);
	tmp_result = _SCSSocketTableRemove(self, id);
	_SCS_UNLOCK(self);

	return tmp_result;
}

/* ---------------------------------------------------------------------------------------------- */

static inline SCSSocket * _SCSSocketTableGet1(SCSSocketTable * self, SCSSocketId id) {
	SCSSocket * tmp_socket;

	if ((tmp_socket = self->entries[id].socket) == NULL) {
		SCS_LOG(WARN, SYSTEM, 00000, "<<%d>>", id);
		return NULL;
	}

	if (SCSSocketHold(tmp_socket) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%d>>", id);
		return NULL;
	}

	return tmp_socket;
}
SCSSocket * SCSSocketTableGet1(SCSSocketTable * self, SCSSocketId id) {
	SCSSocket * tmp_socket;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998,);
		return NULL;
	}

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 99997, "<<%d<%d<%d>>", SCS_SKTID_MINVAL, id, SCS_SKTID_MAXVAL);
		return NULL;
	}

	_SCS_LOCK(self);

	tmp_socket = _SCSSocketTableGet1(self, id);

	_SCS_UNLOCK(self);

	return tmp_socket;
}

static inline SCSSocket * _SCSSocketTableGet2(SCSSocketTable * self, scs_socket_desc sd) {
	SCSSocket * tmp_socket;
	SCSSocketId tmp_id;

	if (SCSSocketIdValidate((tmp_id = self->indexes[sd])) == false) {
		SCS_LOG(WARN, SYSTEM, 00000, "<<%d,%d>>", sd, tmp_id);
		return NULL;
	}

	if (sd != self->entries[tmp_id].sd) {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%d,%d,%d>>", tmp_id, sd, self->entries[tmp_id].sd);
		return NULL;
	}

	if ((tmp_socket = self->entries[tmp_id].socket) == NULL) {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%d>>", tmp_id);
		return NULL;
	}

	if (SCSSocketHold(tmp_socket) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%d>>", tmp_id);
		return NULL;
	}

	return tmp_socket;
}
SCSSocket * SCSSocketTableGet2(SCSSocketTable * self, scs_socket_desc sd) {
	SCSSocket * tmp_socket;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998,);
		return false;
	}

	if (_SCS_VALIDATE_SOCKDESC(sd) == false) {
		SCS_LOG(FATAL, SYSTEM, 99999, "<<%d<%d<%d>>", 0, sd, SCS_DESCRIPTOR_MAXVAL);
		return false;
	}

	_SCS_LOCK(self);

	tmp_socket = _SCSSocketTableGet2(self, sd);

	_SCS_UNLOCK(self);

	return tmp_socket;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSSocketTableNotify(SCSSocketTable * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998,);
		return;
	}

	_SCS_LOCK(self);

	if (0 < self->count) {
		SCSSocketId tmp_counter;
		SCSSocketId tmp_id;

		tmp_counter = 0;

		for (tmp_id = 0; tmp_id < SCS_SOCKETTABLE_MAXENTRIES; tmp_id++) {
			SCSSocket * tmp_socket;

			if ((tmp_socket = self->entries[tmp_id].socket) != NULL) {
				SCSSocketNotify(tmp_socket);

				if (self->count <= ++tmp_counter) {
					break;
				}
			}
		}
	}

	_SCS_UNLOCK(self);

}

void SCSSocketTableTimeOut(SCSSocketTable *self) {
	SCSSocketId tmp_counter;
	SCSSocketId tmp_id;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998,);
		return;
	}

	tmp_counter = 0;

	_SCS_LOCK(self);

	if (0 < self->count) {
		for (tmp_id = 0; tmp_id <= SCS_SOCKETTABLE_MAXENTRIES; tmp_id++) {
			SCSSocket * tmp_socket;

			if ((tmp_socket = self->entries[tmp_id].socket) == NULL) {
				continue;
			}

			SCSSocketTimeOut(tmp_socket);

			if (self->count <= ++tmp_counter) {
				break;
			}
		}
	}

	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

void SCSSocketTableCleanUp(SCSSocketTable * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998,);
		return;
	}

	_SCS_LOCK(self);

	_SCSSocketTableCleanUp(self);

	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

void SCSSocketTableLoggingStatistics(SCSSocketTable * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998,);
		return;
	}

	_SCS_LOCK(self);

	if (0 < self->count) {
		SCSSocketId tmp_counter;
		SCSSocketId tmp_id;

		tmp_counter = 0;

		for (tmp_id = SCS_SKTID_MINVAL; tmp_id < SCS_SOCKETTABLE_MAXENTRIES; tmp_id++) {
			SCSSocket * tmp_socket;

			if ((tmp_socket = self->entries[tmp_id].socket) != NULL) {
				SCSSocketLoggingStatistics(tmp_socket, SCS_LOGTYPE_SOCKET);

				if (self->count <= ++tmp_counter) {
					break;
				}
			}
		}
	}

	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

#undef _SCS_VALIDATE_SOCKDESC

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
