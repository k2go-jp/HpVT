#ifndef SCS_6_FEC_ERRNO_H_
#define SCS_6_FEC_ERRNO_H_ 1

/* ============================================================================================== */

#include <errno.h>

/* ---------------------------------------------------------------------------------------------- */

typedef int fec_errno;

#define FEC_ERRNO_SUCCESS 					0
#define FEC_ERRNO_NULL_POINTER 				(-2)
#define FEC_ERRNO_ILLEGAL_ARGUMENTS 		(-3)
#define FEC_ERRNO_MEMORY_ALLOCATE 			(-4)

/* ============================================================================================== */

#endif /* SCS_6_FEC_ERRNO_H_ */
