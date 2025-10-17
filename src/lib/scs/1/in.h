#ifndef SCS_1_IN_H_
#define SCS_1_IN_H_ 1

/* ============================================================================================== */

#ifdef WIN32
#include <Winsock2.h>
#else /* WIN32 */
#include <endian.h>
#include <netinet/in.h>
#endif /* WIN32 */

/* ---------------------------------------------------------------------------------------------- */

#ifdef WIN32
#define htonll(value) 	__byteswap_uint64(value)
#define ntohll(value) 	__byteswap_uint64(value)
#else
#define htonll(value) 	htobe64(value)
#define ntohll(value) 	be64toh(value)
#endif /* WIN32 */

/* ============================================================================================== */

#endif /* SCS_1_IN_H_ */
