#define SCS_SOURCECODE_FILEID	"5TYP"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/stdbool.h"
#include "scs/5/types.h"

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSProtocolTypeValidate(SCSProtocolType value) {

	switch (value) {
		//case SCS_SKTPROTOTYPE_NONE:
		case SCS_PROTOCOLTYPE_IP:
		case SCS_PROTOCOLTYPE_UDP:
			return true;
		default:
			return false;
	}

}

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSSocketOptionValidate(SCSSocketOption value) {

	if (value != SCS_SKTOPTN_NONE) {
		value &= ~SCS_SKTOPTN_NOBLOCK;
		value &= ~SCS_SKTOPTN_RTTM;
		if (value != SCS_SKTOPTN_NONE) {
			return false;
		}
	}

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSNetworkTrafficInfoInitalize(SCSNetworkTrafficInfo * self) {

	memset(self, 0, sizeof(SCSNetworkTrafficInfo));

	//self->sent.control.packets = 0;
	//self->sent.control.bytes = 0;
	//self->sent.payload.packets = 0;
	//self->sent.payload.bytes = 0;
	//self->sent.redundancy.packets = 0;
	//self->sent.redundancy.bytes = 0;
	//self->sent.dropped.packets = 0;
	//self->sent.dropped.bytes = 0;
	//self->sent.cancel.packets = 0;
	//self->sent.cancel.bytes = 0;

	//self->received.control.packets = 0;
	//self->received.control.bytes = 0;
	//self->received.payload.packets = 0;
	//self->received.payload.bytes = 0;
	//self->received.redundancy.packets = 0;
	//self->received.redundancy.bytes = 0;
	//self->received.dropped.total.packets = 0;
	//self->received.dropped.total.bytes = 0;
	//self->received.dropped.duplicate.packets = 0;
	//self->received.dropped.duplicate.bytes = 0;
	//self->received.dropped.overrun.packets = 0;
	//self->received.dropped.overrun.bytes = 0;
	//self->received.dropped.outofrange.packets = 0;
	//self->received.dropped.outofrange.bytes = 0;
	//self->received.dropped.invalid.packets = 0;
	//self->received.dropped.invalid.bytes = 0;
	//self->received.dropped.broken.packets = 0;
	//self->received.dropped.broken.bytes = 0;

}
inline void SCSNetworkTrafficInfoFinalize(SCSNetworkTrafficInfo * self) {

	//self->sent.control.packets = 0;
	//self->sent.control.bytes = 0;
	//self->sent.payload.packets = 0;
	//self->sent.payload.bytes = 0;
	//self->sent.redundancy.packets = 0;
	//self->sent.redundancy.bytes = 0;
	//self->sent.dropped.packets = 0;
	//self->sent.dropped.bytes = 0;
	//self->sent.cancel.packets = 0;
	//self->sent.cancel.bytes = 0;

	//self->received.control.packets = 0;
	//self->received.control.bytes = 0;
	//self->received.payload.packets = 0;
	//self->received.payload.bytes = 0;
	//self->received.redundancy.packets = 0;
	//self->received.redundancy.bytes = 0;
	//self->received.dropped.total.packets = 0;
	//self->received.dropped.total.bytes = 0;
	//self->received.dropped.duplicate.packets = 0;
	//self->received.dropped.duplicate.bytes = 0;
	//self->received.dropped.overrun.packets = 0;
	//self->received.dropped.overrun.bytes = 0;
	//self->received.dropped.outofrange.packets = 0;
	//self->received.dropped.outofrange.bytes = 0;
	//self->received.dropped.invalid.packets = 0;
	//self->received.dropped.invalid.bytes = 0;
	//self->received.dropped.broken.packets = 0;
	//self->received.dropped.broken.bytes = 0;

	memset(self, 0, sizeof(SCSNetworkTrafficInfo));

}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSPacketQueueStateInitalize(SCSPacketQueueState * self) {

	memset(self, 0, sizeof(SCSPacketQueueState));

	//self->bytes = 0;
	//self->packets.capacity = 0;
	//self->packets.count = 0;

}
inline void SCSPacketQueueStateFinalize(SCSPacketQueueState * self) {

	//self->bytes = 0;
	//self->packets.capacity = 0;
	//self->packets.count = 0;

	memset(self, 0, sizeof(SCSPacketQueueState));

}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSSocketBufferStateInitalize(SCSSocketBufferStatus * self) {

	memset(self, 0, sizeof(SCSSocketBufferStatus));

	SCSPacketQueueStateInitalize(&self->send.control);
	SCSPacketQueueStateInitalize(&self->send.retransmit);
	SCSPacketQueueStateInitalize(&self->send.transmit);
	SCSPacketQueueStateInitalize(&self->receive);

}
inline void SCSSocketBufferStateFinalize(SCSSocketBufferStatus * self) {

	SCSPacketQueueStateFinalize(&self->send.control);
	SCSPacketQueueStateFinalize(&self->send.retransmit);
	SCSPacketQueueStateFinalize(&self->send.transmit);
	SCSPacketQueueStateFinalize(&self->receive);

	memset(self, 0, sizeof(SCSSocketBufferStatus));

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
