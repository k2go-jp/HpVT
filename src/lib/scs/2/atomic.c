#define SCS_SOURCECODE_FILEID	"2ATM"

/* ============================================================================================== */

#include "scs/1/stdbool.h"
#include "scs/2/atomic.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

inline bool _SCSAtomicReferenceIncrease(scs_atomic_reference * self) {
	scs_atomic_reference tmp_reference;

//	if (self == NULL) {
//		return;
//	}

	do {
		if (SCS_REFERENCE_INVVAL <= (tmp_reference = SCSAtomicGet(*self))) {
			return false;
		}
	} while (SCSAtomicCompSet(*self, tmp_reference, (tmp_reference + 1)) == false);

	return true;
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
