#define SCS_SOURCECODE_FILEID	"5FTRRTT"

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
#include "scs/5/feature/rtt.h"
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
			SCS_LOG(ALERT, SOCKET, 99999, "");\
			abort();\
		}
#define _SCS_UNLOCK(xxx_self) \
		if (SCSMutexUnlock((xxx_self)->mutex) != 0) {\
			SCS_LOG(ALERT, SOCKET, 99999, "");\
			abort();\
		}

/* ---------------------------------------------------------------------------------------------- */

void SCSRTTMeasurerInitialize(SCSRTTMeasurer * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSRTTMeasurer));

	SCSMutexInitialize(self->mutex);
	//self->values = NULL;
	//self->capacity = NULL;
	//self->index.minimum = 0;
	//self->index.maximum = 0;
	//self->index.current = 0;
	SCSTimespecInitialize(self->value.minimum);
	SCSTimespecInitialize(self->value.maximum);

}
void SCSRTTMeasurerFinalize(SCSRTTMeasurer * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	SCSMutexInitialize(self->mutex);
	if (self->values != NULL) {
		free(self->values);
		self->values = NULL;
	}
	//self->capacity = NULL;
	//self->index.minimum = 0;
	//self->index.maximum = 0;
	//self->index.current = 0;
	SCSTimespecFinalize(self->value.minimum);
	SCSTimespecFinalize(self->value.maximum);

	memset(self, 0, sizeof(SCSRTTMeasurer));

}

/* ---------------------------------------------------------------------------------------------- */

static inline bool _SCSRTTStandBy(SCSRTTMeasurer * self, size_t capacity) {
	void * tmp_ptr;

	if ((tmp_ptr = calloc(sizeof(scs_timespec), capacity)) == NULL) {
		SCS_LOG(ALERT, MEMORY, 99999, "<<%zu>>", (sizeof(scs_timespec) * capacity));
		return false;
	}

	self->values = tmp_ptr;
	self->capacity = capacity;
	self->index.minimum = 0;
	self->index.maximum = (capacity - 1);
	self->index.current = 0;
	self->index.previous = 0;
	SCSTimespecSetInfinity(self->value.minimum);
	SCSTimespecSet(self->value.maximum, 0, 0);

	return true;
}
bool SCSRTTMeasurerStandBy(SCSRTTMeasurer * self, size_t capacity) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (capacity < SCS_RTTMEASURER_MINENTRIES || SCS_RTTMEASURER_MAXENTRIES < capacity) {
		SCS_LOG(WARN, SYSTEM, 99997, "<<%d,%d,%d>>", //
				SCS_RTTMEASURER_MINENTRIES, capacity, SCS_RTTMEASURER_MAXENTRIES);
		return false;
	}

	_SCS_LOCK(self);
	tmp_result = _SCSRTTStandBy(self, capacity);
	_SCS_UNLOCK(self);

	return tmp_result;
}

void SCSRTTMeasurerUpdate(SCSRTTMeasurer * self, scs_timespec value) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	_SCS_LOCK(self);

	if (0 < self->capacity) {
		SCSTimespecCopy(self->values[self->index.current], value);

		self->index.previous = self->index.current;

		if (self->index.maximum <= ++self->index.current) {
			self->index.current = self->index.minimum;
		}

		if (SCSTimespecCompare(value, self->value.minimum, <)) {
			SCSTimespecCopy(self->value.minimum, value);
		}

		if (SCSTimespecCompare(value, self->value.maximum, >)) {
			SCSTimespecCopy(self->value.maximum, value);
		}
	}

	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

scs_timespec SCSRTTMeasurerGetLatest(SCSRTTMeasurer * self) {
	scs_timespec tmp_result;

	SCSTimespecInitialize(tmp_result);

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return tmp_result;
	}

	_SCS_LOCK(self);

	if (0 < self->capacity) {
		SCSTimespecCopy(tmp_result, self->values[self->index.previous]);
	}

	_SCS_UNLOCK(self);

	return tmp_result;
}
scs_timespec SCSRTTMeasurerGetMin(SCSRTTMeasurer * self) {
	scs_timespec tmp_result;

	SCSTimespecInitialize(tmp_result);

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return tmp_result;
	}

	_SCS_LOCK(self);

	if (0 < self->capacity) {
		SCSTimespecCopy(tmp_result, self->value.minimum);
	}

	_SCS_UNLOCK(self);

	return tmp_result;
}
scs_timespec SCSRTTMeasurerGetMax(SCSRTTMeasurer * self) {
	scs_timespec tmp_result;

	SCSTimespecInitialize(tmp_result);

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return tmp_result;
	}

	_SCS_LOCK(self);

	if (0 < self->capacity) {
		SCSTimespecCopy(tmp_result, self->value.maximum);
	}

	_SCS_UNLOCK(self);

	return tmp_result;
}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
