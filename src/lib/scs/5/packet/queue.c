#define SCS_SOURCECODE_FILEID	"5PKTQUE"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/inttypes.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/1/time.h"
#include "scs/1/unistd.h"
#include "scs/2/logger.h"
#include "scs/5/defines.h"
#include "scs/5/packet/queue.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_LOCK
#error
#endif
#ifdef _SCS_UNLOCK
#error
#endif

#ifdef _SCS_WAIT
#error
#endif
#ifdef _SCS_SIGNAL
#error
#endif

#ifdef _SCS_WAIT_ENQUEUE
#error
#endif
#ifdef _SCS_SIGNAL_ENQUEUE
#error
#endif

#ifdef _SCS_WAIT_ENQUEUE
#error
#endif
#ifdef _SCS_SIGNAL_ENQUEUE
#error
#endif

#ifdef _SCS_LOG
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

#define _SCS_WAIT(xxx_cond, xxx_mutex, xxx_timeout, xxx_retval) \
		if ((xxx_retval = SCSCondTimedwait(xxx_cond, xxx_mutex, xxx_timeout)) != 0) {\
			switch (xxx_retval) { \
				case ETIMEDOUT: \
				case EINTR: \
					break;\
				default: {\
					SCS_LOG(ALERT, SYSTEM, 99999,  "<<%d>>", xxx_retval);\
					abort();\
				}\
			}\
		}
#define _SCS_SIGNAL(xxx_cond) \
		if (SCSCondSignal(xxx_cond) != 0) {\
			SCS_LOG(ALERT, SYSTEM, 99999, "");\
			abort();\
		}

#define _SCS_WAIT_ENQUEUE(xxx_self, xxx_timeout, xxx_retval) \
		_SCS_WAIT((xxx_self)->cond.enqueue, (xxx_self)->mutex, xxx_timeout, xxx_retval)
#define _SCS_SIGNAL_ENQUEUE(xxx_self) \
		_SCS_SIGNAL((xxx_self)->cond.enqueue)

#define _SCS_WAIT_DEQUEUE(xxx_self, xxx_timeout, xxx_retval) \
		_SCS_WAIT((xxx_self)->cond.dequeue, (xxx_self)->mutex, xxx_timeout, xxx_retval)
#define _SCS_SIGNAL_DEQUEUE(xxx_self) \
		_SCS_SIGNAL((xxx_self)->cond.dequeue)

#define _SCS_LOG(xxx_type, xxx_format, xxx_args...) \
		switch(type) {\
			case SCS_LOGTYPE_DEBUG: {\
				SCS_LOG(NOTICE, DEBUG, 00000, xxx_format, xxx_args);\
				break;\
			}\
			case SCS_LOGTYPE_MEMORY: {\
				SCS_LOG(NOTICE, MEMORY, 00000, xxx_format, xxx_args);\
				break;\
			}\
			case SCS_LOGTYPE_PACKET: {\
				SCS_LOG(NOTICE, PACKET, 00000, xxx_format, xxx_args);\
				break;\
			}\
			case SCS_LOGTYPE_SOCKET: {\
				SCS_LOG(NOTICE, SOCKET, 00000, xxx_format, xxx_args);\
				break;\
			}\
			case SCS_LOGTYPE_SYSTEM: {\
				SCS_LOG(NOTICE, SYSTEM, 00000, xxx_format, xxx_args);\
				break;\
			}\
		}

/* ---------------------------------------------------------------------------------------------- */

static inline void _SCSPacketQueueCleanUp(SCSPacketQueue * self) {
	size_t i;

	for (i = 0; i < self->capacity; i++) {
		SCSPacket * tmp_packet;

		if ((tmp_packet = self->entries[i]) == NULL) {
			continue;
		}

		SCSPacketDestroy(tmp_packet);
		SCSPacketFree(tmp_packet);
	}

}

/* ---------------------------------------------------------------------------------------------- */

void SCSPacketQueueInitialize(SCSPacketQueue * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	memset(self, 0, sizeof(SCSPacketQueue));

	SCSMutexInitialize(self->mutex);
	SCSCondInitialize(self->cond.enqueue);
	SCSCondInitialize(self->cond.dequeue);
	//self->entries = NULL;
	//self->capacity = 0;
	//self->count = 0;
	//self->index.head = 0;
	//self->index.tail = 0;
	//self->index.last = 0;
	SCSTimespecInitialize(self->timestamp.next);
	//self->bytes = 0;

}
void SCSPacketQueueFinalize(SCSPacketQueue * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	SCSMutexFinalize(self->mutex);
	SCSCondFinalize(self->cond.enqueue);
	SCSCondFinalize(self->cond.dequeue);

	if (self->entries != NULL) {
		_SCSPacketQueueCleanUp(self);
		free(self->entries);
		self->entries = NULL;
	}
	//self->capacity
	//self->count
	//self->index.head
	//self->index.tail
	//self->index.last
	SCSTimespecFinalize(self->timestamp.next);
	//self->bytes

	memset(self, 0, sizeof(SCSPacketQueue));

}

/* ---------------------------------------------------------------------------------------------- */

static inline bool _SCSPacketQueueStandBy(SCSPacketQueue * self, size_t capacity) {
	SCSPacket ** tmp_ptr;

	if (capacity < SCS_PKTQUEUE_MINSIZE || SCS_PKTQUEUE_MAXSIZE < capacity) {
		SCS_LOG(ERROR, SYSTEM, 99997, "<<%zu, %zu, %zu>>", //
				SCS_PKTQUEUE_MINSIZE, capacity, SCS_PKTQUEUE_MAXSIZE);
		return false;
	}

	if (self->capacity != 0) {
		return false;
	}

	if ((tmp_ptr = (SCSPacket **) calloc(capacity, sizeof(SCSPacket *))) == NULL) {
		SCS_LOG(ERROR, SYSTEM, 00002, "<<%zu>>", (capacity * sizeof(SCSPacket *)));
		return false;
	}

	self->index.head = 0;
	self->index.tail = 0;
	self->index.last = capacity - 1;
	self->entries = tmp_ptr;
	self->capacity = capacity;
	self->count = 0;

	return true;
}
inline bool SCSPacketQueueStandBy(SCSPacketQueue * self, size_t capacity) {
	bool tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return 0;
//	}

	_SCS_LOCK(self);
	tmp_result = _SCSPacketQueueStandBy(self, capacity);
	_SCS_UNLOCK(self);

	return tmp_result;
}

static inline bool _SCSPacketQueueEnqueue(SCSPacketQueue * __restrict self,
		SCSPacket * __restrict packet) {
	size_t tmp_next;

	if (self->capacity < self->count) {
		SCS_LOG(WARN, SYSTEM, 79994, "<<%zu/%zu>>", self->count, self->capacity);
		return false;
	}

	if (self->index.tail == self->index.last) {
		tmp_next = 0;
	}
	else {
		tmp_next = self->index.tail + 1;
	}

	if (self->index.head == tmp_next) {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%zu/%zu,%zu/%zu>>", //
				self->count, self->capacity, self->index.head, self->index.tail);
		return false;
	}

	if (SCSPacketHold(packet) == false) {
		return false;
	}

	self->entries[self->index.tail] = packet;
	self->index.tail = tmp_next;
	self->count++;

	self->bytes += SCSPacketGetPayloadSize(packet);
	self->bytes += SCSPacketGetPadLength(packet);

	if ((self->capacity >> 1) < self->count) {
		scs_timespec tmp_timestamp;

		SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);

		if (SCSTimespecCompare(self->timestamp.next, tmp_timestamp, <)) {
			SCS_LOG(WARN, SYSTEM, 99999, "<<%zu/%zu>>", self->count, self->capacity);
			SCSTimespecIncreaseSec(tmp_timestamp, 3);
			SCSTimespecCopy(self->timestamp.next, tmp_timestamp);
		}
	}

	_SCS_SIGNAL_ENQUEUE(self);

	return true;
}
inline bool SCSPacketQueueEnqueue(SCSPacketQueue * __restrict self, SCSPacket * __restrict packet) {
	bool tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return 0;
//	}

//	if (packet == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	_SCS_LOCK(self);

	tmp_result = _SCSPacketQueueEnqueue(self, packet);

	_SCS_UNLOCK(self);

	return tmp_result;
}
static inline SCSPacket * _SCSPacketQueueDequeue(SCSPacketQueue * self) {
	SCSPacket * tmp_packet;

	if (self->count < 1) {
		return NULL;
	}

	if ((tmp_packet = self->entries[self->index.head]) == NULL) {
		return NULL;
	}

	self->entries[self->index.head] = NULL;

	if (self->index.head < self->index.last) {
		self->index.head++;
	}
	else {
		self->index.head = 0;
	}

	self->count--;

	self->bytes -= SCSPacketGetPayloadSize(tmp_packet);
	self->bytes -= SCSPacketGetPadLength(tmp_packet);

	//SCSPacketFree(tmp_packet);

	_SCS_SIGNAL_DEQUEUE(self);

	return tmp_packet;
}
inline SCSPacket * SCSPacketQueueDequeue(SCSPacketQueue * self) {
	SCSPacket * tmp_result;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return 0;
//	}

	_SCS_LOCK(self);

	tmp_result = _SCSPacketQueueDequeue(self);

	_SCS_UNLOCK(self);

	return tmp_result;
}

/* ---------------------------------------------------------------------------------------------- */

size_t SCSPacketQueueGetCount(SCSPacketQueue * self) {
	size_t tmp_reuslt;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return 0;
//	}

	_SCS_LOCK(self);

	tmp_reuslt = self->count;

	_SCS_UNLOCK(self);

	return tmp_reuslt;
}

uint64_t SCSPacketQueueGetBytes(SCSPacketQueue * self) {
	uint64_t tmp_reuslt;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return 0;
//	}

	_SCS_LOCK(self);

	tmp_reuslt = self->bytes;

	_SCS_UNLOCK(self);

	return tmp_reuslt;
}

inline void SCSPacketQueueGetStatus(SCSPacketQueue * __restrict self,
		SCSPacketQueueState * __restrict out) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return 0;
//	}

//	if (out == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return 0;
//	}

	_SCS_LOCK(self);

	out->bytes = self->bytes;
	out->packets.capacity = self->capacity;
	out->packets.count = self->count;

	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSPacketQueueWaitToEnqueue(SCSPacketQueue * self, scs_time timeout_ms) {
	int tmp_result;
	scs_timespec tmp_timeout;

	_SCS_LOCK(self);

	if (self->count == 0) {
		SCSTimespecSetCurrentTime(tmp_timeout, CLOCK_REALTIME);
		SCSTimespecIncreaseMsec(tmp_timeout, timeout_ms);
		_SCS_WAIT_ENQUEUE(self, tmp_timeout, tmp_result);
	}

	_SCS_UNLOCK(self);

}
inline void SCSPacketQueueWaitToDequeue(SCSPacketQueue * self, scs_time timeout_ms) {
	int tmp_result;
	scs_timespec tmp_timeout;

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return 0;
//	}

	_SCS_LOCK(self);

	if (self->count == self->capacity) {
		SCSTimespecSetCurrentTime(tmp_timeout, CLOCK_REALTIME);
		SCSTimespecIncreaseMsec(tmp_timeout, timeout_ms);
		_SCS_WAIT_DEQUEUE(self, tmp_timeout, tmp_result);
	}

	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSPacketQueueCleanUp(SCSPacketQueue * self) {

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return 0;
//	}

	_SCS_LOCK(self);

	_SCSPacketQueueCleanUp(self);

	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSPacketQueueLogging(SCSPacketQueue * __restrict self, SCSLogType type,
		const char * __restrict prefix) {
	char tmp_message[512];

//	if (self == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return;
//	}

	_SCS_LOCK(self);

	snprintf(tmp_message, sizeof(tmp_message), //
			"%s: packets=%"PRIuS"/%"PRIuS", bytes=%"PRIu64, //
			prefix, self->count, self->capacity, self->bytes);

	_SCS_UNLOCK(self);

	_SCS_LOG(type, "%s", tmp_message);

}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

#undef _SCS_WAIT
#undef _SCS_SIGNAL

#undef _SCS_WAIT_ENQUEUE
#undef _SCS_SIGNAL_ENQUEUE

#undef _SCS_WAIT_DEQUEUE
#undef _SCS_SIGNAL_DEQUEUE

#undef _SCS_LOG

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
