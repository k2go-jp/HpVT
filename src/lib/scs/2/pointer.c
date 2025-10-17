#define SCS_SOURCECODE_FILEID	"2PTR"

/* ============================================================================================== */

#include <stdint.h>
#include <string.h>

#include "scs/1/stdbool.h"
#include "scs/2/pointer.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

inline bool _SCSPointerMove(SCSPointer * self, size_t length) {

	if (!SCSPointerCanMove(*self, length)) {
		return false;
	}

	self->offset += length;

	return true;
}

inline bool _SCSPointerWrite(SCSPointer * __restrict self, void * __restrict ptr, size_t length) {

	if (!SCSPointerCanWrite(*self, length)) {
		return false;
	}

	memcpy((self->ptr + self->offset), ptr, length);
	self->offset += length;

	return true;
}
inline bool _SCSPointerWritePad(SCSPointer * self, size_t length, uint8_t value) {

	if (!SCSPointerCanWrite(*self, length)) {
		return false;
	}

	memset((self->ptr + self->offset), value, length);
	self->offset += length;

	return true;
}
inline bool _SCSPointerWriteUint8(SCSPointer * self, uint8_t value) {

	if (!SCSPointerCanWrite(*self, sizeof(uint8_t))) {
		return false;
	}

	*((uint8_t *) (self->ptr + self->offset)) = value;
	self->offset += sizeof(uint8_t);

	return true;
}
inline bool _SCSPointerWriteUint16(SCSPointer * self, uint16_t value) {

	if (!SCSPointerCanWrite(*self, sizeof(uint16_t))) {
		return false;
	}

	*((uint16_t *) (self->ptr + self->offset)) = value;
	self->offset += sizeof(uint16_t);

	return true;
}
inline bool _SCSPointerWriteUint24(SCSPointer * self, uint32_t value) {
	struct {
		uint8_t * src;
		uint8_t * dst;
	} tmp_ptr;

	if (!SCSPointerCanWrite(*self, (sizeof(uint8_t) * 3))) {
		return false;
	}

	tmp_ptr.dst = (self->ptr + self->offset);
	tmp_ptr.src = (uint8_t *) &value;
	tmp_ptr.dst[0] = tmp_ptr.src[1];
	tmp_ptr.dst[1] = tmp_ptr.src[2];
	tmp_ptr.dst[2] = tmp_ptr.src[3];
	self->offset += (sizeof(uint8_t) * 3);

	return true;
}
inline bool _SCSPointerWriteUint32(SCSPointer * self, uint32_t value) {

	if (!SCSPointerCanWrite(*self, sizeof(uint32_t))) {
		return false;
	}

	*((uint32_t *) (self->ptr + self->offset)) = value;
	self->offset += sizeof(uint32_t);

	return true;
}
inline bool _SCSPointerWriteUint64(SCSPointer * self, uint64_t value) {

	if (!SCSPointerCanWrite(*self, sizeof(uint64_t))) {
		return false;
	}

	*((uint64_t *) (self->ptr + self->offset)) = value;
	self->offset += sizeof(uint64_t);

	return true;
}

inline bool _SCSPointerRead(SCSPointer * __restrict self, void * __restrict ptr, size_t length) {

	if (!SCSPointerCanRead(*self, length)) {
		return false;
	}

	memcpy(ptr, (self->ptr + self->offset), length);
	self->offset += length;

	return true;
}
inline bool _SCSPointerReadUint8(SCSPointer * __restrict self, uint8_t * __restrict value) {

	if (!SCSPointerCanRead(*self, sizeof(uint8_t))) {
		return false;
	}

	*(value) = *((uint8_t *) (self->ptr + self->offset));
	self->offset += sizeof(uint8_t);

	return true;
}
inline bool _SCSPointerReadUint16(SCSPointer * __restrict self, uint16_t * __restrict value) {

	if (!SCSPointerCanRead(*self, sizeof(uint16_t))) {
		return false;
	}

	*(value) = *((uint16_t *) (self->ptr + self->offset));
	self->offset += sizeof(uint16_t);

	return true;
}
inline bool _SCSPointerReadUint24(SCSPointer * __restrict self, uint32_t * __restrict value) {
	struct {
		uint8_t * src;
		uint8_t * dst;
	} tmp_ptr;

	if (!SCSPointerCanRead(*self, (sizeof(uint8_t) * 3))) {
		return false;
	}

	tmp_ptr.dst = (uint8_t *) &value;
	tmp_ptr.src = (self->ptr + self->offset);
	tmp_ptr.dst[0] = 0x00;
	tmp_ptr.dst[1] = tmp_ptr.src[0];
	tmp_ptr.dst[2] = tmp_ptr.src[1];
	tmp_ptr.dst[3] = tmp_ptr.src[2];
	self->offset += (sizeof(uint8_t) * 3);

	return true;
}
inline bool _SCSPointerReadUint32(SCSPointer * __restrict self, uint32_t * __restrict value) {

	if (!SCSPointerCanRead(*self, sizeof(uint32_t))) {
		return false;
	}

	*(value) = *((uint32_t *) (self->ptr + self->offset));
	self->offset += sizeof(uint32_t);

	return true;
}
inline bool _SCSPointerReadUint64(SCSPointer * __restrict self, uint64_t * __restrict value) {

	if (!SCSPointerCanRead(*self, sizeof(uint64_t))) {
		return false;
	}

	*(value) = *((uint64_t *) (self->ptr + self->offset));
	self->offset += sizeof(uint64_t);

	return true;
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
