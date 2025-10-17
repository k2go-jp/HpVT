#ifndef SCS_5_CALLBACK_LIST_H_
#define SCS_5_CALLBACK_LIST_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/2/atomic.h"
#include "scs/2/lock.h"
#include "scs/5/callback/system/types.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSCallbackListEntry SCSCallbackListEntry;

struct SCSCallbackListEntry {
	scs_atomic_reference reference;

	scs_mutex mutex;
	SCSCallbackId id;
	SCSCallbackType type;
	struct {
		void * ptr;
		size_t size;
	} config;
	struct {
		scs_timespec last;
		scs_timespec next;
	} timestamp;

	SCSCallbackListEntry * next;
};

/* ---------------------------------------------------------------------------------------------- */

extern void SCSCallbackListEntryInitialize(SCSCallbackListEntry * self);
extern void SCSCallbackListEntryFinalize(SCSCallbackListEntry * self);

/* ---------------------------------------------------------------------------------------------- */

extern SCSCallbackListEntry * SCSCallbackListEntryCreate(void);
extern void SCSCallbackListEntryDestroy(SCSCallbackListEntry * self);

extern bool SCSCallbackListEntryHold(SCSCallbackListEntry * self);
extern void SCSCallbackListEntryFree(SCSCallbackListEntry * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSCallbackListEntryCall(SCSCallbackListEntry * self, scs_timespec timestamp);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSCallbackConfigMonitor(void);

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSCallbackList {
	scs_mutex mutex;
	SCSCallbackListEntry ** entries;
	size_t capacity;
	size_t count;
} SCSCallbackList;

extern void SCSCallbackListInitialize(SCSCallbackList * self);
extern void SCSCallbackListFinalize(SCSCallbackList * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSCallbackListStandBy(SCSCallbackList * self, size_t capacity);

extern bool SCSCallbackListAdd(																	//
		SCSCallbackList * __restrict self, 														//
		SCSCallbackType type,																	//
		void * __restrict ptr, 																	//
		size_t size,
		SCSCallbackId * out);
extern bool SCSCallbackListRemove(SCSCallbackList * self, SCSCallbackId id);

extern void SCSCallbackListCall(SCSCallbackList * self);

/* ============================================================================================== */

#endif /* SCS_5_CALLBACK_LIST_H_ */
