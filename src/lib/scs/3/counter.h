#ifndef SCS_3_COUNTER_H_
#define SCS_3_COUNTER_H_ 1

/* ============================================================================================== */

#include <string.h>

#include "scs/1/inttypes.h"
#include "scs/2/atomic.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSObjectCounter {
	scs_atomic_uint64 create;
	struct {
		scs_atomic_uint64 request;
		scs_atomic_uint64 complete;
	} destroy;
} SCSObjectCounter;

#define SCSObjectCounterInitializer	\
		{ 0, { 0, 0 } }

#define SCSObjectCounterInitialize(xxx_self) \
		memset(&(xxx_self), 0, sizeof(xxx_self));\
		SCSAtomicInitialize((xxx_self).create, 0);\
		SCSAtomicInitialize((xxx_self).destroy.request, 0);\
		SCSAtomicInitialize((xxx_self).destroy.complete, 0)
#define SCSObjectCounterFinalize(xxx_self) \
		SCSAtomicFinalize((xxx_self).create, 0);\
		SCSAtomicFinalize((xxx_self).destroy.request, 0);\
		SCSAtomicFinalize((xxx_self).destroy.complete, 0);\
		memset(&(xxx_self), 0, sizeof(xxx_self))
#define SCSObjectCounterIncreaseCreate(xxx_self) \
		SCSAtomicIncrease((xxx_self).create)
#define SCSObjectCounterIncreaseDestroy(xxx_self) \
		SCSAtomicIncrease((xxx_self).destroy.request)
#define SCSObjectCounterIncreaseDestroyed(xxx_self) \
		SCSAtomicIncrease((xxx_self).destroy.complete)

#define SCSObjectCounterMonitor(xxx_caption) \
		SCS_LOG(NOTICE, MEMORY, 00000,  "%-16s C:%10"PRIu64",D:%10"PRIu64"/%10"PRIu64"", \
				xxx_caption,\
				SCSAtomicGet((_counter).create), \
				SCSAtomicGet((_counter).destroy.complete), \
				SCSAtomicGet((_counter).destroy.request))

/* ============================================================================================== */

#endif /* SCS_3_COUNTER_H_ */
