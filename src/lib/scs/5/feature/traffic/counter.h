#ifndef SCS_5_FEATURE_TRRAFIC_COUNTER_H_
#define SCS_5_FEATURE_TRRAFIC_COUNTER_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"
#include "scs/5/packet/packet.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSNetworkTrafficCounter {
	struct {
		struct {
			scs_atomic_uint64 packets;
			scs_atomic_uint64 bytes;
		} control;
		struct {
			scs_atomic_uint64 packets;
			scs_atomic_uint64 bytes;
		} payload;
		struct {
			scs_atomic_uint64 packets;
			scs_atomic_uint64 bytes;
		} redundancy;
		struct {
			scs_atomic_uint64 packets;
			scs_atomic_uint64 bytes;
		} dropped;
		struct {
			scs_atomic_uint64 packets;
			scs_atomic_uint64 bytes;
		} cancel;
	} sent;
	struct {
		struct {
			scs_atomic_uint64 packets;
			scs_atomic_uint64 bytes;
		} control;
		struct {
			scs_atomic_uint64 packets;
			scs_atomic_uint64 bytes;
		} payload;
		struct {
			scs_atomic_uint64 packets;
			scs_atomic_uint64 bytes;
		} redundancy;
		struct {
			scs_atomic_uint64 packets;
		} unexpected;
		struct {
			scs_atomic_uint64 packets;
		} reordering;
		struct {
			struct {
				scs_atomic_uint64 packets;
				scs_atomic_uint64 bytes;
			} total;
			struct {
				scs_atomic_uint64 packets;
				scs_atomic_uint64 bytes;
			} duplicate;
			struct {
				scs_atomic_uint64 packets;
				scs_atomic_uint64 bytes;
			} overrun;
			struct {
				scs_atomic_uint64 packets;
				scs_atomic_uint64 bytes;
			} outofrange;
			struct {
				scs_atomic_uint64 packets;
				scs_atomic_uint64 bytes;
			} invalid;
			struct {
				scs_atomic_uint64 packets;
				scs_atomic_uint64 bytes;
			} broken;
		} dropped;
	} received;
} SCSNetworkTrafficCounter;

extern void SCSNetworkTrafficCounterInitialize(SCSNetworkTrafficCounter * self);
extern void SCSNetworkTrafficCounterFinalize(SCSNetworkTrafficCounter * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSNetworkTrafficCounterIncreaseSentPacket(											//
		SCSNetworkTrafficCounter * __restrict self,												//
		SCSPacket * __restrict packet, 															//
		size_t bytes);

extern void SCSNetworkTrafficCounterIncreaseDroppedPacketInSending(
		SCSNetworkTrafficCounter * __restrict self, 											//
		SCSPacket * __restrict packet, 															//
		size_t bytes);

extern void SCSNetworkTrafficCounterIncreaseCancelPacketInSending(								//
		SCSNetworkTrafficCounter * __restrict self, 											//
		SCSPacket * __restrict packet, 															//
		size_t bytes);

extern void SCSNetworkTrafficCounterIncreaseReceivedPacket(										//
		SCSNetworkTrafficCounter * __restrict self, 											//
		SCSPacket * __restrict packet, 															//
		size_t bytes);

extern void SCSNetworkTrafficCounterIncreaseUnexpectedPacketInReceiving(						//
		SCSNetworkTrafficCounter * __restrict self, 											//
		SCSPacket * __restrict packet, 															//
		size_t bytes);

extern void SCSNetworkTrafficCounterIncreaseReorderingPacketInReceiving(						//
		SCSNetworkTrafficCounter * __restrict self, 											//
		SCSPacket * __restrict packet, 															//
		size_t bytes);

extern void SCSNetworkTrafficCounterIncreaseDroppedPacketInReceiving(							//
		SCSNetworkTrafficCounter * __restrict self, 											//
		SCSPacket * __restrict packet, 															//
		size_t bytes);

extern void SCSNetworkTrafficCounterIncreaseDuplicatedPacketInReceiving(						//
		SCSNetworkTrafficCounter * __restrict self, 											//
		SCSPacket * __restrict packet, 															//
		size_t bytes);

extern void SCSNetworkTrafficCounterIncreaseOverrunPacketInReceiving(							//
		SCSNetworkTrafficCounter * __restrict self, 											//
		SCSPacket * __restrict packet, 															//
		size_t bytes);

extern void SCSNetworkTrafficCounterIncreaseOutOfRangePacketInReceiving(						//
		SCSNetworkTrafficCounter * __restrict self, 											//
		SCSPacket * __restrict packet, 															//
		size_t bytes);

extern void SCSNetworkTrafficCounterIncreaseInvalidPacketInReceiving(							//
		SCSNetworkTrafficCounter * __restrict self, 											//
		SCSPacket * __restrict packet, 															//
		size_t bytes);

extern void SCSNetworkTrafficCounterIncreaseBrokenPacketInReceiving(							//
		SCSNetworkTrafficCounter * __restrict self, 											//
		SCSPacket * __restrict packet, 															//
		size_t bytes);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSNetworkTrafficCounterGet(														//
		SCSNetworkTrafficCounter * __restrict self, 											//
		SCSNetworkTrafficInfo * __restrict out);

#define SCSNetworkTrafficGetTotalReceivedPacket(xxx_self) \
		(SCSAtomicGet((xxx_self)->received.control.packets) + \
		 SCSAtomicGet((xxx_self)->received.payload.packets))

#define SCSNetworkTrafficGetReceivedDataPacket(xxx_self) \
		SCSAtomicGet((xxx_self)->received.payload.packets)

#define SCSNetworkTrafficGetReceivedControlPacket(xxx_self) \
		SCSAtomicGet((xxx_self)->received.control.packets)

/* ---------------------------------------------------------------------------------------------- */

extern void SCSNetworkTrafficCounterLogging(SCSNetworkTrafficCounter * self, SCSLogType type);

/* ============================================================================================== */

#endif /* SCS_5_FEATURE_TRRAFIC_COUNTER_H_ */
