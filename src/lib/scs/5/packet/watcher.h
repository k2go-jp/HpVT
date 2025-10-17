#ifndef SCS_5_PACKET_WATCHER_H_
#define SCS_5_PACKET_WATCHER_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/2/lock.h"
#include "scs/5/packet/packet.h"

/* ---------------------------------------------------------------------------------------------- */

#define SCS_PKTWATCHER_MINENTRIES	(8)
#define SCS_PKTWATCHER_MAXENTRIES	UINT8_MAX

/* ---------------------------------------------------------------------------------------------- */

typedef size_t SCSPacketWatcherId;

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPacketWatcherSettings {
	size_t capacity;
	struct {
		SCSPacketWatcherId minimum;
		SCSPacketWatcherId maximum;
	} id;
} SCSPacketWatcherSettings;

extern void SCSPacketWatcherSettingsInitialize(SCSPacketWatcherSettings * self);
extern void SCSPacketWatcherSettingsFinalize(SCSPacketWatcherSettings * self);

extern void SCSPacketWatcherSettingsAdapt(SCSPacketWatcherSettings * self);

extern bool SCSPacketWatcherSettingsValidate(SCSPacketWatcherSettings * self);

extern void SCSPacketWatcherSettingsDump( //
		SCSPacketWatcherSettings * __restrict self, //
		__const char * __restrict prefix);

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPacketWatcherStatus {
	SCSPacketWatcherId id;
	SCSPacketSeqno seqno;
	scs_timespec timestamp;
} SCSPacketWatcherStatus;

#define SCSPacketWatcherStatusCopy(xxx_dst, xxx_src) \
		memcpy(&(xxx_dst), &(xxx_src), sizeof(SCSPacketWatcherStatus))

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPacketWatcherEntry SCSPacketWatcherEntry;

struct SCSPacketWatcherEntry {
	SCSPacketWatcherStatus status;
	SCSPacketWatcherEntry * next;
};

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPacketWatcher {
	scs_mutex mutex;
	SCSPacketWatcherEntry * entries;
	size_t capacity;
	SCSPacketWatcherEntry * head;
	struct {
		SCSPacketWatcherEntry * head;
		size_t count;
	} queue;
	struct {
		SCSPacketWatcherId minimum;
		SCSPacketWatcherId maximum;
		SCSPacketWatcherId current;
	} id;
} SCSPacketWatcher;

extern void SCSPacketWatcherInitialize(SCSPacketWatcher * self);
extern void SCSPacketWatcherFinalize(SCSPacketWatcher * self);

extern bool SCSPacketWatcherStandBy( //
		SCSPacketWatcher * __restrict self, //
		SCSPacketWatcherSettings * __restrict settings);

extern bool SCSPacketWatcherAdd( //
		SCSPacketWatcher * __restrict self, //
		SCSPacket * __restrict packet, //
		SCSPacketWatcherStatus * __restrict status);
extern bool SCSPacketWatcherUpdate(SCSPacketWatcher * self, SCSPacketWatcherId id);
extern bool SCSPacketWatcherRemove( //
		SCSPacketWatcher * __restrict self, //
		SCSPacketWatcherId id, //
		SCSPacketWatcherStatus * __restrict status);

/* ============================================================================================== */

#endif /* SCS_5_PACKET_WATCHER_H_ */
