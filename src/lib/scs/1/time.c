#define SCS_SOURCECODE_FILEID	"1TIM"

/* ============================================================================================== */

#include <stdint.h>

#include "scs/1/time.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_SECOND	(1000 * 1000 * 1000)

/* ---------------------------------------------------------------------------------------------- */

inline void _SCSTimespecAdd(scs_timespec * __restrict self, scs_timespec * __restrict value,
		scs_timespec * __restrict retval) {

	retval->tv_sec = self->tv_sec + value->tv_sec;
	retval->tv_nsec = self->tv_nsec + value->tv_nsec;

	if (_SCS_SECOND < retval->tv_nsec) {
		++retval->tv_sec;
		retval->tv_nsec -= _SCS_SECOND;
	}

}
inline void _SCSTimespecAddMillisec(scs_timespec * __restrict self, scs_time msec,
		scs_timespec * __restrict retval) {

	retval->tv_sec = self->tv_sec + (msec / 1000);
	retval->tv_nsec = self->tv_nsec + ((msec % 1000) * 1000 * 1000);

	if (_SCS_SECOND < retval->tv_nsec) {
		++retval->tv_sec;
		retval->tv_nsec -= _SCS_SECOND;
	}

}

inline void _SCSTimespecSub(scs_timespec * __restrict self, scs_timespec * __restrict value,
		scs_timespec * __restrict retval) {

	retval->tv_sec = self->tv_sec - value->tv_sec;
	retval->tv_nsec = self->tv_nsec - value->tv_nsec;

	if (retval->tv_nsec < 0) {
		--retval->tv_sec;
		retval->tv_nsec += _SCS_SECOND;
	}

}
inline void _SCSTimespecSubMillisec(scs_timespec * __restrict self, scs_time msec,
		scs_timespec * __restrict retval) {

	retval->tv_sec = self->tv_sec - (msec / 1000);
	retval->tv_nsec = self->tv_nsec - ((msec % 1000) * 1000 * 1000);

	if (retval->tv_nsec < 0) {
		--retval->tv_sec;
		retval->tv_nsec += _SCS_SECOND;
	}

}

inline void _SCSTimespecIncrease(scs_timespec * __restrict self, scs_timespec * __restrict value) {

	self->tv_sec += value->tv_sec;
	self->tv_nsec += value->tv_nsec;

	if (_SCS_SECOND < self->tv_nsec) {
		++self->tv_sec;
		self->tv_nsec -= _SCS_SECOND;
	}

}
inline void _SCSTimespecIncreaseMillisec(scs_timespec * __restrict self, scs_time msec) {

	self->tv_sec += (msec / 1000);
	self->tv_nsec += ((msec % 1000) * 1000 * 1000);

	if (_SCS_SECOND < self->tv_nsec) {
		++self->tv_sec;
		self->tv_nsec -= _SCS_SECOND;
	}

}

inline void _SCSTimespecDecrease(scs_timespec * __restrict self, scs_timespec * __restrict value) {

	self->tv_sec -= value->tv_sec;
	self->tv_nsec -= value->tv_nsec;

	if (self->tv_nsec < 0) {
		--self->tv_sec;
		self->tv_nsec += _SCS_SECOND;
	}

}
inline void _SCSTimespecDecreaseMillisec(scs_timespec * __restrict self, scs_time msec) {

	self->tv_sec -= (msec / 1000);
	self->tv_nsec -= ((msec % 1000) * 1000 * 1000);

	if (self->tv_nsec < 0) {
		--self->tv_sec;
		self->tv_nsec += _SCS_SECOND;
	}

}

/* ---------------------------------------------------------------------------------------------- */

uint32_t _SCSTimespecConvertToMillisec32(scs_timespec * self) {
	const static uint32_t tmp_maximum = (UINT32_MAX / 1000);

	uint32_t tmp_value;

	if (tmp_maximum <= self->tv_sec) {
		return UINT32_MAX;
	}

	tmp_value = self->tv_sec * 1000;
	tmp_value = tmp_value + (self->tv_nsec / (1000 * 1000));

	return tmp_value;
}

uint64_t _SCSTimespecConvertToMillisec64(scs_timespec * self) {
	const static uint64_t tmp_maximum = (UINT64_MAX / 1000);

	uint64_t tmp_value;

	if (tmp_maximum <= self->tv_sec) {
		return UINT64_MAX;
	}

	tmp_value = self->tv_sec * 1000;
	tmp_value = tmp_value + (self->tv_nsec / (1000 * 1000));

	return tmp_value;
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
