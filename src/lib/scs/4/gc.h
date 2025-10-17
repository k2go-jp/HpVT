#ifndef SCS_4_GC_H_
#define SCS_4_GC_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/2/atomic.h"
#include "scs/2/lock.h"
#include "scs/3/worker.h"

/* ---------------------------------------------------------------------------------------------- */

typedef bool (*SCSDiscardFunction)(void * self);

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSGarbage SCSGarbage;

struct SCSGarbage {
	void * ptr;
	SCSDiscardFunction func;

	struct {
		uint64_t times;
		struct {
			scs_time first;
			scs_time last;
		} timestamp;
	} debug;

	SCSGarbage * next;
};

extern SCSGarbage * SCSGarbageCreate(void * ptr, SCSDiscardFunction func);
extern void SCSGarbageDestroy(SCSGarbage * self);

extern void SCSGarbageInitialize(SCSGarbage * self);
extern void SCSGarbageFinalize(SCSGarbage * self);

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSGarbageCollector {
	scs_mutex mutex;

	struct {
		SCSGarbage * head;
		SCSGarbage * tail;
		size_t count;
	} garbages;
	time_t interval; //level

	struct {
		struct {
			uint64_t entry;
			uint64_t disposed;
		} count;
	} summary;

	struct {
		uint64_t entry;
		uint64_t disposed;
	} statistics;

	SCSWorker worker;
} SCSGarbageCollector;

extern void SCSGarbageCollectorInitialize(SCSGarbageCollector * self);
extern void SCSGarbageCollectorFinalize(SCSGarbageCollector * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSGarbageCollectorStart(SCSGarbageCollector * self);
extern void SCSGarbageCollectorStop(SCSGarbageCollector * self);

extern bool SCSGarbageCollectorPush(			//
		SCSGarbageCollector * __restrict self, 	//
		SCSGarbage * __restrict garbage);
extern SCSGarbage * SCSGarbageCollectorPop(SCSGarbageCollector * self);

extern size_t SCSGarbageCollectorCount(SCSGarbageCollector * self);

extern scs_time SCSGarbageCollectorUpdate(SCSGarbageCollector * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSGarbageCollectorMonitor(void);

/* ---------------------------------------------------------------------------------------------- */

extern SCSGarbageCollector _scs_global_gc;

/* ============================================================================================== */

#endif /* SCS_4_GC_H_ */
