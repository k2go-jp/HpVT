#ifndef SCS_1_ERRNO_H_
#define SCS_1_ERRNO_H_ 1

/* ============================================================================================== */

#include <errno.h>
#include <string.h>

/* ---------------------------------------------------------------------------------------------- */

#ifdef WIN32
typedef DWORD scs_errno;
#else
typedef int scs_errno;
#endif /* WIN32 */

#ifdef WIN32
#define SCS_EMSGSIZE_MAX 94
#else /* WIN32 */
#define SCS_EMSGSIZE_MAX 128
#endif /* WIN32 */

#ifdef WIN32
#define SCSGetLastError(xxx_errno) \
		(xxx_errno = GetLastError())
#define SCSGetWSALastError(xxx_errno) \
		(xxx_errno = WSAGetLastError())
#define SCSGetErrorMessage(xxx_errno, xxx_ptr, xxx_size) \
		strerror_s(xxx_ptr, xxx_size, xxx_errno)
#else /* WIN32 */
#define SCSSetLastError(xxx_errno) \
		(errno = xxx_errno)
#define SCSGetLastError(xxx_errno) \
		(xxx_errno = errno)
#define SCSGetWSALastError(xxx_errno) \
		(xxx_errno = errno)
#define SCSGetErrorMessage(xxx_errno, xxx_ptr, xxx_size) \
		strerror_r(xxx_errno, xxx_ptr, xxx_size)
#endif

/* ---------------------------------------------------------------------------------------------- */

//#define SCS_ERRNO_	(0x00000000)	// 0000 0000 0000 0000 0000 0000 0000 0000

#define SCS_ERRNO_NONE				0x00000000	// 0000 0000 0000 0000 0000 0000 0000 0000
#define SCS_ERRNO_ESOCKTYPE			0x00000001	// 0000 0000 0000 0000 0000 0000 0000 0001
#define SCS_ERRNO_EINVALSOCK		0x00000002	// 0000 0000 0000 0000 0000 0000 0000 0010
#define SCS_ERRNO_EINVALARGS		0x00000004	// 0000 0000 0000 0000 0000 0000 0000 0100
#define SCS_ERRNO_EINVALSTAT		0x00000008	// 0000 0000 0000 0000 0000 0000 0000 1000
#define SCS_ERRNO_EINVALSETTINGS	0x00000010	// 0000 0000 0000 0000 0000 0000 0001 0000
#define SCS_ERRNO_EINVALFRMT		0x00000020	// 0000 0000 0000 0000 0000 0000 0010 0000
#define SCS_ERRNO_EINVALCTX			0x00000040	// 0000 0000 0000 0000 0000 0000 0100 0000
#define SCS_ERRNO_ESHORTBUF			0x00001000	// 0000 0000 0000 0000 0001 0000 0000 0000
#define SCS_ERRNO_EMEMORY			0x00002000	// 0000 0000 0000 0000 0010 0000 0000 0000

#define SCS_ERRNO_ENOTSTART			0x40000000	// 1000 0000 0000 0000 0000 0000 0000 0000
#define SCS_ERRNO_EILLEGAL			0x80000000	// 1000 0000 0000 0000 0000 0000 0000 0000

/* ============================================================================================== */

#endif /* SCS_1_ERRNO_H_ */
