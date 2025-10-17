#define SCS_SOURCECODE_FILEID	"5FTRTCN"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/inttypes.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/1/time.h"
#include "scs/2/lock.h"
#include "scs/2/logger.h"
#include "scs/5/feature/traffic/controller.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_SHIFTBIT
#error
#endif
#ifdef _SCS_SECOND
#error
#endif

#ifdef _SCS_NULLCHECK
#error
#endif

#ifdef _SCS_BIT2BYTE
#error
#endif

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_SHIFTBIT			3
#define _SCS_SECOND				(1000 * 1000 * 1000)

#define _SCS_NULLCHECK(xxx_self) \
		if (self == NULL) {\
			SCS_LOG(ALERT, SOCKET, 99998, "");\
			abort();\
		}

#define _SCS_BIT2BYTE(xxx_bit, xxx_byte) \
		xxx_byte = xxx_bit >> 3

#define _SCS_DEBUG(xxx_format, ...)
//#define _SCS_DEBUG(xxx_format, ...)	printf(xxx_format"\n", ##__VA_ARGS__)

/* ---------------------------------------------------------------------------------------------- */

static void _SCSTimespecDump(scs_timespec * __restrict self, __const char * __restrict prefix) {
	char tmp_buffer[UINT8_MAX];

	if (self == NULL) {
		return;
	}

	if (prefix == NULL) {
		prefix = "";
	}

	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), prefix, "", "%"PRIuS".%09"PRIuS, self->tv_sec,
			self->tv_nsec);

}

/* ---------------------------------------------------------------------------------------------- */

static inline void _SCSVideoStreamTrafficControllerReset(SCSVideoStreamTrafficController * self) {

	memset(&self->state, 0, sizeof(self->state));

}

/* ---------------------------------------------------------------------------------------------- */

inline void SCSVideoStreamTrafficControllerInitialize(SCSVideoStreamTrafficController * self) {

	memset(self, 0, sizeof(SCSVideoStreamTrafficController));

	//self->available = false;
	//self->rate.bit = 0;
	//self->rate.frame = 0;
	//self->divition = 0;
	//self->interval.frame = 0;
	//self->interval.quantity = 0;
	//self->bytes.total = 0;
	//self->bytes.frame = 0;
	//self->bytes.quantity = 0;
	//self->state.packets = 0;
	//self->state.bytes.limit = 0;
	//self->state.bytes.total = 0;
	//self->state.bytes.quantity = 0;
	//self->state.frames.remain = 0;
	//self->state.frames.ideal = 0;
	//self->state.frames.actual = 0;
	SCSTimespecInitialize(self->state.timestamp.base);
	SCSTimespecInitialize(self->state.timestamp.dead);
	SCSTimespecInitialize(self->state.timestamp.next.frame);
	SCSTimespecInitialize(self->state.timestamp.next.quantity);

}
inline void SCSVideoStreamTrafficControllerFinalize(SCSVideoStreamTrafficController * self) {

	//self->available = false;
	//self->rate.bit = 0;
	//self->rate.frame = 0;
	//self->divition = 0;
	//self->interval.frame = 0;
	//self->interval.quantity = 0;
	//self->bytes.total = 0;
	//self->bytes.frame = 0;
	//self->bytes.quantity = 0;
	//self->state.packets = 0;
	//self->state.bytes.limit = 0;
	//self->state.bytes.total = 0;
	//self->state.bytes.quantity = 0;
	//self->state.frames.remain = 0;
	//self->state.frames.ideal = 0;
	//self->state.frames.actual = 0;
	SCSTimespecFinalize(self->state.timestamp.base);
	SCSTimespecFinalize(self->state.timestamp.dead);
	SCSTimespecFinalize(self->state.timestamp.next.frame);
	SCSTimespecFinalize(self->state.timestamp.next.quantity);

	memset(self, 0, sizeof(SCSVideoStreamTrafficController));

}

/* ---------------------------------------------------------------------------------------------- */

SCS_TCRETVAL SCSVideoStreamTrafficControllerUpdate(
		SCSVideoStreamTrafficController * __restrict self, size_t size,
		scs_timespec * __restrict out) {
	scs_timespec tmp_timestamp;
	uint64_t tmp_bytes;

	_SCS_NULLCHECK(self);
	//_SCS_NULLCHECK(out);

	if (self->available == false) {
		return SCS_TCRETVAL_NG;
	}

	SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);

	if (0 < self->state.packets) {
		self->state.packets++;
		while (SCSTimespecCompare(self->state.timestamp.next.quantity, tmp_timestamp, <)) {
			self->state.bytes.limit += self->state.bytes.quantity;
			SCSTimespecIncrease(self->state.timestamp.next.quantity, self->interval.quantity);
		}
	}
	else {
		self->state.packets++;
		self->state.bytes.limit = self->bytes.quantity;
		self->state.bytes.total = 0;
		self->state.bytes.quantity = self->bytes.quantity;
		self->state.frames.remain = self->rate.frame;
		self->state.frames.ideal = 1;
		self->state.frames.actual = 1;
		SCSTimespecSetCurrentTime(self->state.timestamp.base, CLOCK_MONOTONIC);
		SCSTimespecAddSec(self->state.timestamp.base, 1, self->state.timestamp.dead);
		SCSTimespecAdd(self->state.timestamp.base, self->interval.frame,
				self->state.timestamp.next.frame);
		SCSTimespecAdd(self->state.timestamp.base, self->interval.quantity,
				self->state.timestamp.next.quantity);
	}

	tmp_bytes = self->state.bytes.total + size;

	if (self->state.bytes.limit < tmp_bytes) {
		if (out != NULL) {
			SCSTimespecSub(self->state.timestamp.next.quantity, tmp_timestamp, *out);
			_SCS_DEBUG("+++++ Sleep : %"PRIuS".%09"PRIuS, out->tv_sec, out->tv_nsec);
		}

		return SCS_TCRETVAL_OVER;
	}

	self->state.bytes.total = tmp_bytes;

	return SCS_TCRETVAL_OK;
}

SCS_TCRETVAL SCSVideoStreamTrafficControllerNextFrame(
		SCSVideoStreamTrafficController * __restrict self, scs_timespec * __restrict out) {
	SCS_TCRETVAL tmp_retval;
	scs_timespec tmp_timestamp;

	if (self->available == false) {
		return SCS_TCRETVAL_NG;
	}

	tmp_retval = SCS_TCRETVAL_OK;
	SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);

	if (0 < self->state.frames.remain) {
		uint64_t tmp_bytes;

		self->state.frames.actual++;

		while (SCSTimespecCompare(self->state.timestamp.next.frame, tmp_timestamp, <)
				&& (0 < self->state.frames.remain)) {
			self->state.frames.remain--;
			self->state.frames.ideal++;
			SCSTimespecIncrease(self->state.timestamp.next.frame, self->interval.frame);
		}

		tmp_bytes = self->bytes.frame * self->state.frames.actual;

		if (self->state.bytes.total < tmp_bytes) {
			self->state.bytes.total = tmp_bytes;
		}

		if (self->state.frames.ideal <= self->state.frames.actual) {
			self->state.bytes.quantity = self->bytes.quantity;
		}
		else {
			_SCS_DEBUG("!!!!! Delay : %d/%d", //
					self->state.frames.actual, self->state.frames.ideal);
			self->state.bytes.quantity <<= 1;
			tmp_retval = SCS_TCRETVAL_DELAY;
		}
	}
	else {
		if (SCSTimespecCompare(tmp_timestamp, self->state.timestamp.dead, <)) {
			if (out != NULL) {
				SCSTimespecSub(self->state.timestamp.next.quantity, tmp_timestamp, *out);
				_SCS_DEBUG("!!!!! Sleep : %"PRIuS".%09"PRIuS, out->tv_sec, out->tv_nsec);
			}

			tmp_retval = SCS_TCRETVAL_OVER;
		}

		_SCSVideoStreamTrafficControllerReset(self);
	}

	return tmp_retval;
}

void SCSVideoStreamTrafficControllerReset(SCSVideoStreamTrafficController * self) {

	_SCSVideoStreamTrafficControllerReset(self);

}

/* ---------------------------------------------------------------------------------------------- */

static void _SCSVideoStreamTrafficControllerUpdateConfig(SCSVideoStreamTrafficController * self) {
	uint64_t tmp_total_bytes;
	uint32_t tmp_total_frames;
	uint32_t tmp_divition; /* Must be unsigned */
	scs_time tmp_interval_ms;
	scs_timespec tmp_frame_interval;
	scs_timespec tmp_quantity_interval;
	uint64_t tmp_frame_bytes;
	uint64_t tmp_quantity_bytes;

	if (self->rate.bit < 1) {
		self->available = false;
		return;
	}

	if (self->rate.frame < 1) {
		self->available = false;
		return;
	}

	_SCS_BIT2BYTE(self->rate.bit, tmp_total_bytes);

	tmp_total_frames = self->rate.frame;
	tmp_total_frames++; /* Set margin */

	tmp_divition = self->rate.frame << _SCS_SHIFTBIT;
	if (tmp_divition < self->rate.frame) {
		tmp_divition = 1000;
	}
	if (1000 < tmp_divition) {
		tmp_divition = 1000;
	}

	tmp_interval_ms = 1000 / tmp_total_frames;
	SCSTimespecInitialize(tmp_frame_interval);
	SCSTimespecSetMsec(tmp_frame_interval, tmp_interval_ms);

	tmp_interval_ms = 1000 / tmp_divition;
	SCSTimespecInitialize(tmp_quantity_interval);
	SCSTimespecSetMsec(tmp_quantity_interval, tmp_interval_ms);

	if ((tmp_frame_bytes = tmp_total_bytes / tmp_total_frames) < 1) {
		self->available = false;
		return;
	}

	if ((tmp_quantity_bytes = tmp_total_bytes / tmp_divition) < 1) {
		self->available = false;
		return;
	}

	self->divition = tmp_divition;
	self->interval.frame = tmp_frame_interval;
	self->interval.quantity = tmp_quantity_interval;
	self->bytes.total = tmp_total_bytes;
	self->bytes.frame = tmp_frame_bytes;
	self->bytes.quantity = tmp_quantity_bytes;
	self->available = true;

	_SCSVideoStreamTrafficControllerReset(self);

}

bool SCSVideoStreamTrafficControllerSetBitRate(SCSVideoStreamTrafficController * self,
		scs_bit_rate value) {

	_SCS_NULLCHECK(self);

	self->rate.bit = value;

	_SCSVideoStreamTrafficControllerUpdateConfig(self);

	return true;
}
inline scs_bit_rate SCSVideoStreamTrafficControllerGetBitRate(
		SCSVideoStreamTrafficController * self) {

	_SCS_NULLCHECK(self);

	return self->rate.bit;
}

bool SCSVideoStreamTrafficControllerSetFrameRate(SCSVideoStreamTrafficController * self,
		scs_frame_rate value) {

	_SCS_NULLCHECK(self);

	if (SCS_FRAMERATE_MAX < value) {
		SCS_LOG(WARN, SYSTEM, 00000, "<<%d>>", value);
		value = SCS_FRAMERATE_MAX;
	}

	self->rate.frame = value;

	_SCSVideoStreamTrafficControllerUpdateConfig(self);

	return true;
}
inline scs_frame_rate SCSVideoStreamTrafficControllerGetFrameRate(
		SCSVideoStreamTrafficController * self) {

	_SCS_NULLCHECK(self);

	return self->rate.frame;
}

size_t SCSVideoStreamTrafficControllerGetFrameSize(SCSVideoStreamTrafficController * self) {
	size_t tmp_retval;

	_SCS_NULLCHECK(self);

	if (self->available) {
		tmp_retval = self->bytes.total;
	}
	else {
		tmp_retval = ((size_t) -1);
	}

	return tmp_retval;
}

size_t SCSVideoStreamTrafficControllerGetSendableFrames(SCSVideoStreamTrafficController * self) {
	size_t tmp_retval;

	_SCS_NULLCHECK(self);

	if (self->available) {
		tmp_retval = self->state.frames.remain;
	}
	else {
		tmp_retval = ((size_t) -1);
	}

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSVideoStreamTrafficControllerDump(SCSVideoStreamTrafficController * __restrict self,
		__const char * __restrict prefix) {
	const char tmp_caption[] = "vstc.";
	char tmp_prefix[UINT8_MAX];
	char tmp_buffer[UINT8_MAX];

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	if (prefix == NULL) {
		prefix = "";
	}

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s", prefix, tmp_caption);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "available", "%d", self->available);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "rate.bit", "%"PRIu64" [bps]",
			self->rate.bit);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "rate.frame", "%d [fps]", self->rate.frame);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "divition", "%d", self->divition);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "interval.frame");
	_SCSTimespecDump(&self->interval.frame, tmp_prefix);
	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "interval.quantity");
	_SCSTimespecDump(&self->interval.quantity, tmp_prefix);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s", prefix, tmp_caption);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, //
			"bytes.total", "%"PRIu64" [bytes]", self->bytes.total);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, //
			"bytes.frame", "%"PRIu64" [bytes]", self->bytes.frame);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, //
			"bytes.quantity", "%"PRIu64" [bytes]", self->bytes.quantity);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s", prefix, tmp_caption);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, //
			"state.packets", "%"PRIu64" [packets]", self->state.packets);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, //
			"state.bytes.limit", "%"PRIu64" [bytes]", self->state.bytes.limit);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, //
			"state.bytes.total", "%"PRIu64" [bytes]", self->state.bytes.total);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, //
			"state.bytes.quantity", "%"PRIu64" [bytes]", self->state.bytes.quantity);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, //
			"state.frames.remain", "%d [frames]", self->state.frames.remain);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, //
			"state.frames.ideal", "%d", self->state.frames.ideal);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, //
			"state.frames.actual", "%d", self->state.frames.actual);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "state.timestamp.base");
	_SCSTimespecDump(&self->state.timestamp.base, tmp_prefix);
	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "state.timestamp.dead");
	_SCSTimespecDump(&self->state.timestamp.dead, tmp_prefix);
	snprintf(tmp_prefix, sizeof(tmp_prefix), //
			"%s%s%s", prefix, tmp_caption, "state.timestamp.next.frame");
	_SCSTimespecDump(&self->state.timestamp.next.frame, tmp_prefix);
	snprintf(tmp_prefix, sizeof(tmp_prefix), //
			"%s%s%s", prefix, tmp_caption, "state.timestamp.next.quantity");
	_SCSTimespecDump(&self->state.timestamp.next.quantity, tmp_prefix);

}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_SHIFTBIT
#undef _SCS_INTERVAL_SEC
#undef _SCS_INTERVAL_USEC

#undef _SCS_NULLCHECK

#undef _SCS_BIT2BYTE

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID

