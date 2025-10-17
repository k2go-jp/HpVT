#ifndef SCS_2_THREAD_H_
#define SCS_2_THREAD_H_ 1

/* ============================================================================================== */

#ifdef WIN32
#include <windows.h>
#else /* WIN32 */
#include <pthread.h>
#endif

/* ---------------------------------------------------------------------------------------------- */

#ifdef WIN32
typedef HANDLE scs_thread_desc;
#else /* WIN32 */
typedef pthread_t scs_thread_desc;
#endif /* WIN32 */

#ifdef WIN32
#error
#else /* WIN32 */
#define SCSThreadCreate(xxx_self, func, args) \
	(pthread_create(&(xxx_self)->id, NULL, func, args) == 0 ? true : false)
#endif /* WIN32 */

/* ============================================================================================== */

#endif /* SCS_2_THREAD_H_ */
