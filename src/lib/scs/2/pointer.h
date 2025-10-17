#ifndef SCS_2_POINTER_H_
#define SCS_2_POINTER_H_ 1

/* ============================================================================================== */

#include <stdint.h>

#include "scs/1/inttypes.h"
#include "scs/2/atomic.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSPointer {
	void * ptr;
	size_t length;
	size_t offset;
} SCSPointer;

#define SCSPointerInitialize(xxx_self, xxx_ptr, xxx_length, xxx_offset) \
		(xxx_self).ptr = xxx_ptr;\
		(xxx_self).length = xxx_length;\
		(xxx_self).offset = xxx_offset;
#define SCSPointerFinalize(xxx_self, xxx_ptr, xxx_length, xxx_offset)

#define SCSPointerPrint(xxx_self, xxx_result, xxx_format, xxx_args...) \
		if ((xxx_self).offset < (xxx_self).length) {\
			if ((xxx_result = snprintf(((xxx_self).ptr + (xxx_self).offset), ((xxx_self).length - (xxx_self).offset), xxx_format, ##xxx_args)) < (xxx_self).length) {\
				(xxx_self).offset += xxx_result;\
			}\
			else {\
				(xxx_self).offset = (xxx_self).length;\
			}\
		}

#define SCSPointerCanMove(xxx_self, xxx_size) \
		(((xxx_self).ptr != NULL) && \
		 (xxx_size < (xxx_self).length) && \
		 ((xxx_self).offset <= ((xxx_self).length - xxx_size)))
#define SCSPointerMove(xxx_self, xxx_length) \
		_SCSPointerMove(&(xxx_self), xxx_length)

#define SCSPointerCanWrite(xxx_self, xxx_size) \
		(((xxx_self).ptr != NULL) && \
		 (xxx_size <= (xxx_self).length) && \
		 ((xxx_self).offset <= ((xxx_self).length - xxx_size)))
#define SCSPointerWrite(xxx_self, xxx_ptr, xxx_length) \
		_SCSPointerWrite(&(xxx_self), xxx_ptr, xxx_length)
#define SCSPointerWritePad(xxx_self, xxx_length, xxx_value) \
		_SCSPointerWritePad(&(xxx_self), xxx_length, xxx_value)
#define SCSPointerWriteUint8(xxx_self, xxx_value) \
		_SCSPointerWriteUint8(&(xxx_self), xxx_value)
#define SCSPointerWriteUint16(xxx_self, xxx_value) \
		_SCSPointerWriteUint16(&(xxx_self), xxx_value)
#define SCSPointerWriteUint24(xxx_self, xxx_value) \
		_SCSPointerWriteUint24(&(xxx_self), xxx_value)
#define SCSPointerWriteUint32(xxx_self, xxx_value) \
		_SCSPointerWriteUint32(&(xxx_self), xxx_value)
#define SCSPointerWriteUint64(xxx_self, xxx_value) \
		_SCSPointerWriteUint64(&(xxx_self), xxx_value)

#define SCSPointerCanRead(xxx_self, xxx_size) \
		(((xxx_self).ptr != NULL) && \
		 (xxx_size <= (xxx_self).length) && \
		 ((xxx_self).offset <= ((xxx_self).length - xxx_size)))
#define SCSPointerRead(xxx_self, xxx_ptr, xxx_length) \
		_SCSPointerRead(&(xxx_self), xxx_ptr, xxx_length)
#define SCSPointerReadUint8(xxx_self, xxx_value) \
		_SCSPointerReadUint8(&(xxx_self), &(xxx_value))
#define SCSPointerReadUint16(xxx_self, xxx_value) \
		_SCSPointerReadUint16(&(xxx_self), &(xxx_value))
#define SCSPointerReadUint24(xxx_self, xxx_value) \
		_SCSPointerReadUint24(&(xxx_self), &(xxx_value))
#define SCSPointerReadUint32(xxx_self, xxx_value) \
		_SCSPointerReadUint32(&(xxx_self), &(xxx_value))
#define SCSPointerReadUint64(xxx_self, xxx_value) \
		_SCSPointerReadUint64(&(xxx_self), &(xxx_value))

#define SCSPointerGetPointer(xxx_self) \
		((xxx_self).ptr)
#define SCSPointerGetLength(xxx_self) \
		((xxx_self).length)
#define SCSPointerGetOffset(xxx_self) \
		((xxx_self).offset)
#define SCSPointerGetReadPointer(xxx_self) \
		((xxx_self).ptr + (xxx_self).offset)
#define SCSPointerGetReadableLength(xxx_self) \
		((xxx_self).length - (xxx_self).offset)
#define SCSPointerGetWritePointer(xxx_self) \
		((xxx_self).ptr + (xxx_self).offset)
#define SCSPointerGetWritableLength(xxx_self) \
		((xxx_self).length - (xxx_self).offset)

#define SCSPointerCheckOffset(xxx_self, xxx_value) \
		((xxx_self).offset == xxx_value)

#define SCSPointerCopy(xxx_self, xxx_target) \
		(xxx_target).ptr = (xxx_self).ptr;\
		(xxx_target).length = (xxx_self).length;\
		(xxx_target).offset = (xxx_self).offset

extern bool _SCSPointerMove(SCSPointer * self, size_t length);

extern bool _SCSPointerWrite(SCSPointer * __restrict self, void * __restrict ptr, size_t length);
extern bool _SCSPointerWritePad(SCSPointer * self, size_t length, uint8_t value);
extern bool _SCSPointerWriteUint8(SCSPointer * self, uint8_t value);
extern bool _SCSPointerWriteUint16(SCSPointer * self, uint16_t value);
extern bool _SCSPointerWriteUint24(SCSPointer * self, uint32_t value);
extern bool _SCSPointerWriteUint32(SCSPointer * self, uint32_t value);
extern bool _SCSPointerWriteUint64(SCSPointer * self, uint64_t value);

extern bool _SCSPointerRead(SCSPointer * __restrict self, void * __restrict ptr, size_t length);
extern bool _SCSPointerReadUint8(SCSPointer * __restrict self, uint8_t * __restrict value);
extern bool _SCSPointerReadUint16(SCSPointer * __restrict self, uint16_t * __restrict value);
extern bool _SCSPointerReadUint24(SCSPointer * __restrict self, uint32_t * __restrict value);
extern bool _SCSPointerReadUint32(SCSPointer * __restrict self, uint32_t * __restrict value);
extern bool _SCSPointerReadUint64(SCSPointer * __restrict self, uint64_t * __restrict value);

/* ============================================================================================== */

#endif /* SCS_2_POINTER_H_ */
