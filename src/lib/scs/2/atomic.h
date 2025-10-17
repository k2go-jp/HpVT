#ifndef SCS_2_ATOMIC_H_
#define SCS_2_ATOMIC_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/stdbool.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef WIN32
#error
#else
typedef volatile int32_t scs_atomic_int32;
typedef volatile uint32_t scs_atomic_uint32;
typedef volatile int64_t scs_atomic_int64;
typedef volatile uint64_t scs_atomic_uint64;
#endif /* WIN32 */

#ifdef WIN32
#error
#else /* WIN32 */
#define SCSAtomicInitialize(xxx_self, xxx_value) \
		xxx_self = xxx_value
#define SCSAtomicFinalize(xxx_self, xxx_value) \
		xxx_self = xxx_value
#define SCSAtomicGet(xxx_self) \
		xxx_self
#define SCSAtomicSet(xxx_self, xxx_value) \
		xxx_self = xxx_value
#define SCSAtomicAdd(xxx_self, xxx_value) \
		__sync_add_and_fetch(&(xxx_self), xxx_value)
#define SCSAtomicSub(xxx_self, xxx_value) \
		__sync_sub_and_fetch(&(xxx_self), xxx_value)
#define SCSAtomicIncrease(xxx_self) \
		__sync_add_and_fetch(&(xxx_self), 1)
#define SCSAtomicDecrease(xxx_self) \
		__sync_sub_and_fetch(&(xxx_self), 1)
#define SCSAtomicAnd(xxx_self, xxx_value) \
		__sync_and_and_fetch(&(xxx_self), xxx_value)
#define SCSAtomicOr(xxx_self, xxx_value) \
		__sync_or_and_fetch(&(xxx_self), xxx_value)
#define SCSAtomicXor(xxx_self, xxx_value) \
		__sync_xor_and_fetch(&(xxx_self), xxx_value)
#define SCSAtomicCompSet(xxx_self, xxx_oldval, xxx_newval) \
		__sync_bool_compare_and_swap(&(xxx_self), xxx_oldval, xxx_newval)
#endif /* WIN32 */

/* ---------------------------------------------------------------------------------------------- */

#ifdef WIN32
#error
#else
typedef volatile bool scs_atomic_lock;
#endif /* WIN32 */

#ifdef WIN32
#error
#else /* WIN32 */
#define SCSAtomicLockInitialize(xxx_self) \
		(xxx_self = false)
#define SCSAtomicLockFinalize(xxx_self) \
		(xxx_self = true)
#define SCSAtomicLockLock(xxx_self) \
		while(__sync_bool_compare_and_swap(&(xxx_self), false, true)) {}
#define SCSAtomicLockUnlock(xxx_self) \
		__sync_bool_compare_and_swap(&(xxx_self), true, false)
#endif /* WIN32 */

/* ---------------------------------------------------------------------------------------------- */

#ifdef WIN32
#error
#else
typedef volatile uint32_t scs_atomic_reference;
#endif /* WIN32 */

typedef uint32_t scs_reference;

#define SCS_REFERENCE_INVVAL	0x80000000
#define SCS_REFERENCE_MINVAL	0
#define SCS_REFERENCE_MAXVAL	0x7FFFFFFF

#define SCSAtomicReferenceInitialize(xxx_self) \
		SCSAtomicInitialize(xxx_self, 0)
#define SCSAtomicReferenceFinalize(xxx_self) \
		SCSAtomicFinalize(xxx_self, 0)
#define SCSAtomicReferenceForbid(xxx_self) \
		SCSAtomicOr(xxx_self, SCS_REFERENCE_INVVAL)
#define SCSAtomicReferenceCanDestroy(xxx_self) \
		(SCS_REFERENCE_INVVAL == SCSAtomicGet(xxx_self) ? true : false)
#define SCSAtomicReferenceIsDestroy(xxx_self) \
		(SCS_REFERENCE_INVVAL <= SCSAtomicGet(xxx_self) ? true : false)
#define SCSAtomicReferenceIncrease(xxx_self) \
		_SCSAtomicReferenceIncrease(&(xxx_self))
#define SCSAtomicReferenceDecrease(xxx_self) \
		SCSAtomicDecrease(xxx_self)

extern bool _SCSAtomicReferenceIncrease(scs_atomic_reference * self);

/* ============================================================================================== */

#endif /* SCS_2_ATOMIC_H_ */
