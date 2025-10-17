#ifndef SCS_1_TIME_H_
#define SCS_1_TIME_H_ 1

/* ============================================================================================== */

#include <limits.h>
#include <stdint.h>
#include <time.h>

#include "scs/1/comatibility.h"

/* ---------------------------------------------------------------------------------------------- */

typedef time_t scs_time;

#define SCS_TIME_MAX	LONG_MAX

/* ---------------------------------------------------------------------------------------------- */

typedef struct timespec scs_timespec;

#define SCSTimespecInitialize(xxx_self) \
		(xxx_self).tv_sec = (xxx_self).tv_nsec = 0;
#define SCSTimespecFinalize(xxx_self) \
		(xxx_self).tv_sec = (xxx_self).tv_nsec = 0;

/* ---------------------------------------------------------------------------------------------- */

#define SCSTimespecSet(xxx_self, xxx_sec, xxx_nsec) \
		(xxx_self).tv_sec = xxx_sec; \
		(xxx_self).tv_nsec = xxx_nsec
#define SCSTimespecSetZero(xxx_self) \
		(xxx_self).tv_sec = (xxx_self).tv_nsec = 0;
#define SCSTimespecSetInfinity(xxx_self) \
		(xxx_self).tv_sec = -1;\
		(xxx_self).tv_nsec = -1
#define SCSTimespecSetMsec(xxx_self, xxx_msec) \
		(xxx_self).tv_sec = (xxx_msec / 1000);\
		(xxx_self).tv_nsec = ((xxx_msec % 1000) * 1000 * 1000)
#define SCSTimespecSetCurrentTime(xxx_self, xxx_type) \
		clock_gettime(xxx_type, &(xxx_self))

#define SCSTimespecGetSec(xxx_self) \
		(xxx_self).tv_sec
#define SCSTimespecGetNanosec(xxx_self) \
		(xxx_self).tv_nsec

#define SCSTimespecAdd(xxx_self, xxx_value, xxx_retval) \
		_SCSTimespecAdd(&(xxx_self), &(xxx_value), &(xxx_retval))
#define SCSTimespecAddSec(xxx_self, xxx_sec, xxx_retval) \
		(xxx_retval).tv_sec = (xxx_self).tv_sec + xxx_sec;\
		(xxx_retval).tv_nsec = (xxx_self).tv_nsec
#define SCSTimespecAddMsec(xxx_self, xxx_msec, xxx_retval) \
		_SCSTimespecAddMillisec(&(xxx_self), xxx_msec, &(xxx_retval))

#define SCSTimespecSub(xxx_self, xxx_value, xxx_retval) \
		_SCSTimespecSub(&(xxx_self), &(xxx_value), &(xxx_retval))
#define SCSTimespecSubSec(xxx_self, xxx_sec, xxx_retval) \
		(xxx_retval).tv_sec = (xxx_self).tv_sec - xxx_sec;\
		(xxx_retval).tv_nsec = (xxx_self).tv_nsec
#define SCSTimespecSubMsec(xxx_self, xxx_value, xxx_retval) \
		_SCSTimespecSubMillisec(&(xxx_self), &(xxx_value), &(xxx_retval))

#define SCSTimespecIncrease(xxx_self, xxx_value) \
		_SCSTimespecIncrease(&(xxx_self), &(xxx_value))
#define SCSTimespecIncreaseSec(xxx_self, xxx_sec) \
		(xxx_self).tv_sec += xxx_sec
#define SCSTimespecIncreaseMsec(xxx_self, xxx_msec) \
		_SCSTimespecIncreaseMillisec(&(xxx_self), xxx_msec)

#define SCSTimespecDecrease(xxx_self, xxx_value) \
		_SCSTimespecDecrease(&(xxx_self), &(xxx_value))
#define SCSTimespecDecreaseSec(xxx_self, xxx_sec) \
		(xxx_self).tv_sec -= xxx_sec
#define SCSTimespecDecreaseMsec(xxx_self, xxx_msec) \
		_SCSTimespecDecreaseMillisec(&(xxx_self), xxx_msec)

#define SCSTimespecConvertToMillisec32(xxx_self) \
		_SCSTimespecConvertToMillisec32(&(xxx_self))
#define SCSTimespecConvertToMillisec64(xxx_self) \
		_SCSTimespecConvertToMillisec64(&(xxx_self))

#define SCSTimespecIsSet(xxx_self) \
		((xxx_self).tv_sec != 0 || (xxx_self).tv_nsec != 0)
#define SCSTimespecIsInfinity(xxx_self) \
		((xxx_self).tv_sec == -1 || (xxx_self).tv_nsec == -1)

/* ---------------------------------------------------------------------------------------------- */

#define SCSTimespecCompare(xxx_value1, xxx_value2, xxx_action) \
		(((xxx_value1).tv_sec == (xxx_value2).tv_sec) ? \
		((xxx_value1).tv_nsec xxx_action (xxx_value2).tv_nsec) : \
		((xxx_value1).tv_sec xxx_action (xxx_value2).tv_sec))

#define SCSTimespecCopy(xxx_dst, xxx_src) \
		(xxx_dst).tv_sec = (xxx_src).tv_sec;\
		(xxx_dst).tv_nsec = (xxx_src).tv_nsec

/* ---------------------------------------------------------------------------------------------- */

#define SCSTimespecDump(xxx_self, xxx_prefix) \
		_SCSTimespecDump(&(xxx_self), xxx_prefix)

/* ---------------------------------------------------------------------------------------------- */

EXTERN void _SCSTimespecAdd(																	//
		scs_timespec * __restrict self, 														//
		scs_timespec * __restrict value,														//
		scs_timespec * __restrict retval);
EXTERN void _SCSTimespecAddMillisec(															//
		scs_timespec * __restrict self, 														//
		scs_time msec,																			//
		scs_timespec * __restrict retval);

EXTERN void _SCSTimespecSub(																	//
		scs_timespec * __restrict self, 														//
		scs_timespec * __restrict value,														//
		scs_timespec * __restrict retval);
EXTERN void _SCSTimespecSubMillisec(															//
		scs_timespec * __restrict self, 														//
		scs_time msec,																			//
		scs_timespec * __restrict retval);

EXTERN void _SCSTimespecIncrease(scs_timespec * __restrict self, scs_timespec * __restrict value);
EXTERN void _SCSTimespecIncreaseMillisec(scs_timespec * __restrict self, scs_time msec);

EXTERN void _SCSTimespecDecrease(scs_timespec * __restrict self, scs_timespec * __restrict value);
EXTERN void _SCSTimespecDecreaseMillisec(scs_timespec * __restrict self, scs_time msec);

/* ---------------------------------------------------------------------------------------------- */

EXTERN uint32_t _SCSTimespecConvertToMillisec32(scs_timespec * self);
EXTERN uint64_t _SCSTimespecConvertToMillisec64(scs_timespec * self);

/* ============================================================================================== */

#endif /* SCS_1_TIME_H_ */
