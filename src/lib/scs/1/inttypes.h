#ifndef SCS_1_INTTYPES_H_
#define SCS_1_INTTYPES_H_ 1

/* ============================================================================================== */

#ifdef WIN32
#else /* WIN32 */
#include <inttypes.h>
#endif /* WIN32 */

/* ---------------------------------------------------------------------------------------------- */

#ifdef WIN32
#define PRId8		"d"
#define RRIu8		"u"
#define PRIx8		"x"
#define PRId16		"hd"
#define RRIu16		"hu"
#define PRIx16		"hx"
#define PRId32		"I32d"
#define RRIu32		"I32u"
#define PRIx32		"I32x"
#define PRId64		"I64d"
#define PRIu64		"I64u"
#define PRIx64		"I64x"
#define PRIdS		"Id"
#define PRIuS		"Iu"
#else /* WIN32 */
#define PRIdS		"zd"
#define PRIuS		"zu"
#endif /* WIN32 */

/* ============================================================================================== */

#endif /* SCS_1_INTTYPES_H_ */
