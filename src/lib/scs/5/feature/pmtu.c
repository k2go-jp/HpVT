#define SCS_SOURCECODE_FILEID	"5FTRMTU"

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
#include "scs/5/defines.h"
#include "scs/5/feature/pmtu.h"
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

void SCSPMTUMeasurerInitialize(SCSPMTUMeasurer * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSPMTUMeasurer));

	SCSMutexInitialize(self->mutex);
	//self->value = 0;
	SCSTimespecInitialize(self->lifetime);
	SCSTimespecInitialize(self->timeout);
	//self->predited.maximum = 0;
	//self->predited.current = 0;
	//self->predited.diff = 0;
	self->predited.cycle.maximum = 5;
	//self->predited.cycle.counter = 0;

}
void SCSPMTUMeasurerFinalize(SCSPMTUMeasurer * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	SCSMutexInitialize(self->mutex);
	//self->value = 0;
	SCSTimespecFinalize(self->lifetime);
	SCSTimespecFinalize(self->timeout);
	//self->predited.maximum = 0;
	//self->predited.current = 0;
	//self->predited.diff = 0;
	//self->predited.cycle.maximum = 0;
	//self->predited.cycle.counter = 0;

	memset(self, 0, sizeof(SCSPMTUMeasurer));

}

/* ---------------------------------------------------------------------------------------------- */

static inline bool _SCSPMTUUpdate(SCSPMTUMeasurer * self, int value) {
	scs_timespec tmp_timestamp;

	SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);

	if (value <= self->value) {
		if (!SCSTimespecIsSet(self->timeout)) {
			return false;
		}

		if (SCSTimespecCompare(tmp_timestamp, self->timeout, <)) {
			return false;
		}
	}

	self->value = value;

	if (SCSTimespecIsSet(self->lifetime)) {
		SCSTimespecAdd(tmp_timestamp, self->lifetime, self->timeout);
	}
	else {
		SCSTimespecSetZero(self->timeout);
	}

	if (value < self->predited.maximum) {
		self->predited.current = value;
		self->predited.diff = self->predited.maximum - value;
	}

	self->predited.cycle.counter = 0;

	return true;
}
bool SCSPMTUMeasurerUpdate(SCSPMTUMeasurer * self, int value) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	if (value < 1) {
		//SCS_LOG(WARN, SYSTEM, 99998, "Invalid value. <<%d>>", value);
		return false;
	}

	_SCS_LOCK(self);

	tmp_result = _SCSPMTUUpdate(self, value);

	_SCS_UNLOCK(self);

	return tmp_result;
}

static inline size_t _SCSPMTUPredict(SCSPMTUMeasurer * self) {
	static const size_t tmp_sizes[] = {
			500, 	//
			1000, 	//
			1500, 	//
			3000, 	//
			6000, 	//
			9000, 	//
			12000, 	//
			15000, 	//
			30000,	//
			60000,	//
			0 };

	if (0 < self->predited.cycle.maximum) {
		if (self->predited.cycle.maximum < self->predited.cycle.counter) {
			return 0;
		}
	}

	if (self->value < tmp_sizes[0]) {
		return tmp_sizes[0];
	}

	if (self->value < self->predited.maximum) {
		if (1 < self->predited.diff) {
			self->predited.diff >>= 1;
		}

		self->predited.current -= self->predited.diff;

		if (self->predited.current <= self->value) {
			self->predited.current = self->predited.maximum;
			self->predited.diff = self->predited.maximum - self->value;
			self->predited.cycle.counter++;
		}
	}
	else {
		int tmp_size;
		int i;

		for (i = 0, tmp_size = tmp_sizes[i]; 0 < tmp_size; tmp_size = tmp_sizes[++i]) {
			if (self->value < tmp_size) {
				break;
			}
		}

		if (tmp_size == 0) {
			tmp_size = SCS_PACKET_MAXSIZE;
		}

		self->predited.maximum = tmp_size;
		self->predited.current = tmp_size;
		self->predited.diff = tmp_size - self->value;
		self->predited.cycle.counter = 0;
	}

	return self->predited.current;
}
size_t SCSPMTUMeasurerPredict(SCSPMTUMeasurer * self) {
	size_t tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return 0;
	}

	_SCS_LOCK(self);

	tmp_result = _SCSPMTUPredict(self);

	_SCS_UNLOCK(self);

	return tmp_result;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSPMTUMeasurerSetLifetime(SCSPMTUMeasurer * self, scs_timespec value) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	_SCS_LOCK(self);

	if (SCSTimespecIsSet(value)) {
		scs_timespec tmp_timestamp;

		SCSTimespecCopy(self->lifetime, value);

		SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);
		SCSTimespecAdd(tmp_timestamp, value, self->timeout);
	}
	else {
		SCSTimespecSetZero(self->lifetime);
		SCSTimespecSetZero(self->timeout);
	}

	_SCS_UNLOCK(self);

}

int SCSPMTUMeasurerGet(SCSPMTUMeasurer * self) {
	int tmp_retval;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return 0;
	}

	_SCS_LOCK(self);

	tmp_retval = self->value;

	_SCS_UNLOCK(self);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
