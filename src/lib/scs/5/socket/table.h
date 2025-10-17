#ifndef SCS_5_SOCKET_TABLE_H_
#define SCS_5_SOCKET_TABLE_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/2/lock.h"
#include "scs/5/defines.h"
#include "scs/5/socket/types.h"

/* ---------------------------------------------------------------------------------------------- */

#define SCS_SOCKETTABLE_MAXENTRIES	(SCS_SKTID_MAXVAL + 1)
#define SCS_SOCKETTABLE_MAXINDEXS	(SCS_DESCRIPTOR_MAXVAL + 1)

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSSocketTableEntry {
	int sd;
	SCSSocket * socket;
} SCSSocketTableEntry;

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSSocketTable {
	scs_mutex mutex;
	SCSSocketTableEntry entries[SCS_SOCKETTABLE_MAXENTRIES];
	size_t capacity;
	size_t count;
	SCSSocketId indexes[SCS_SOCKETTABLE_MAXINDEXS];
} SCSSocketTable;

extern void SCSSocketTableInitialize(SCSSocketTable * self);
extern void SCSSocketTableFinalize(SCSSocketTable * self);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSSocketTableAdd(SCSSocketTable * __restrict self, SCSSocket * __restrict socket);
extern bool SCSSocketTableUpdate(SCSSocketTable * self, SCSSocketId id);
extern bool SCSSocketTableRemove(SCSSocketTable * self, SCSSocketId id);

/* ---------------------------------------------------------------------------------------------- */

extern SCSSocket * SCSSocketTableGet1(SCSSocketTable * self, SCSSocketId id);
extern SCSSocket * SCSSocketTableGet2(SCSSocketTable * self, scs_socket_desc sd);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSSocketTableNotify(SCSSocketTable * self);
extern void SCSSocketTableTimeOut(SCSSocketTable * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSSocketTableCleanUp(SCSSocketTable * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSSocketTableLoggingStatistics(SCSSocketTable * self);

/* ============================================================================================== */

#endif /* SCS_5_SOCKET_TABLE_H_ */
