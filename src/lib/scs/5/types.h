#ifndef SCS_5_TYPES_H_
#define SCS_5_TYPES_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/5/defines.h"

/* ---------------------------------------------------------------------------------------------- */

typedef int SCSProtocolType;

#define SCS_PROTOCOLTYPE_NONE		(0)
#define SCS_PROTOCOLTYPE_IP			(1)
#define SCS_PROTOCOLTYPE_UDP		(2)

extern bool SCSProtocolTypeValidate(SCSProtocolType value);

/* ---------------------------------------------------------------------------------------------- */

typedef int SCSSocketId;

#define SCS_SKTID_INVVAL			0
#define SCS_SKTID_MINVAL			1
#define SCS_SKTID_MAXVAL			SCS_SOCKET_MAXNUM

#define SCSSocketIdValidate(xxx_value) \
		(SCS_SKTID_MINVAL <= xxx_value || xxx_value <= SCS_SKTID_MAXVAL)

/* ---------------------------------------------------------------------------------------------- */

typedef uint32_t SCSSocketOption;

#define SCS_SKTOPTN_NONE			0x00000000	// 0000 0000 0000 0000 0000 0000 0000 0000
#define SCS_SKTOPTN_NOBLOCK			0x00000001	// 0000 0000 0000 0000 0000 0000 0000 0001
#define SCS_SKTOPTN_RTTM			0x00000100	// 0000 0000 0000 0000 0000 0001 0000 0000
#define SCS_SKTOPTN_PMTUM			0x00000200	// 0000 0000 0000 0000 0000 0010 0000 0000
#define SCS_SKTOPTN_VERIFY			0x00000400	// 0000 0000 0000 0000 0000 0100 0000 0000
#define SCS_SKTOPTN_PAD				0x00000800	// 0000 0000 0000 0000 0000 1000 0000 0000
#define SCS_SKTOPTN_INVVAL			0xFFFFFFFF	// 1111 1111 1111 1111 1111 1111 1111 1111

#define SCS_SKTOPTN_MASKFULL		0x0000FFFF	// 0000 0000 0000 0000 1111 1111 1111 1111
#define SCS_SKTOPTN_MASK1			0x000000FF	// 0000 0000 0000 0000 0000 0000 1111 1111
#define SCS_SKTOPTN_MASK2			0x0000FF00	// 0000 0000 0000 0000 1111 1111 0000 0000

extern bool SCSSocketOptionValidate(SCSSocketOption value);

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSNetworkTrafficInfo {
	struct {
		struct {
			uint64_t packets;
			uint64_t bytes;
		} control;
		struct {
			uint64_t packets;
			uint64_t bytes;
		} payload;
		struct {
			uint64_t packets;
			uint64_t bytes;
		} redundancy;
		struct {
			uint64_t packets;
			uint64_t bytes;
		} dropped;
		struct {
			uint64_t packets;
			uint64_t bytes;
		} cancel;
	} sent;
	struct {
		struct {
			uint64_t packets;
			uint64_t bytes;
		} control;
		struct {
			uint64_t packets;
			uint64_t bytes;
		} payload;
		struct {
			uint64_t packets;
			uint64_t bytes;
		} redundancy;
		struct {
			uint64_t packets;
		} unexpected;
		struct {
			uint64_t packets;
		} reordering;
		struct {
			struct {
				uint64_t packets;
				uint64_t bytes;
			} total;
			struct {
				uint64_t packets;
				uint64_t bytes;
			} duplicate;
			struct {
				uint64_t packets;
				uint64_t bytes;
			} overrun;
			struct {
				uint64_t packets;
				uint64_t bytes;
			} outofrange;
			struct {
				uint64_t packets;
				uint64_t bytes;
			} invalid;
			struct {
				uint64_t packets;
				uint64_t bytes;
			} broken;
		} dropped;
	} received;
} SCSNetworkTrafficInfo;

extern void SCSNetworkTrafficInfoInitalize(SCSNetworkTrafficInfo * self);
extern void SCSNetworkTrafficInfoFinalize(SCSNetworkTrafficInfo * self);

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPacketQueueState {
	uint64_t bytes;
	struct {
		size_t capacity;
		size_t count;
	} packets;
} SCSPacketQueueState;

extern void SCSPacketQueueStateInitalize(SCSPacketQueueState * self);
extern void SCSPacketQueueStateFinalize(SCSPacketQueueState * self);

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSSocketBufferStatus {
	struct {
		SCSPacketQueueState control;
		SCSPacketQueueState retransmit;
		SCSPacketQueueState transmit;
	} send;
	SCSPacketQueueState receive;
} SCSSocketBufferStatus;

extern void SCSSocketBufferStateInitalize(SCSSocketBufferStatus * self);
extern void SCSSocketBufferStateFinalize(SCSSocketBufferStatus * self);

/* ============================================================================================== */

#endif /* SCS_5_TYPES_H_ */
