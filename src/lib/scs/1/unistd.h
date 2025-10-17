#ifndef SCS_1_UNISTD_H_
#define SCS_1_UNISTD_H_ 1

/* ============================================================================================== */

#ifdef WIN32
#include <Windows.h>
#else /* WIN32 */
#include <unistd.h>
#endif /* WIN32 */

/* ---------------------------------------------------------------------------------------------- */

#ifdef WIN32
#define MSLEEP(msec)		Sleep(msec)
#else
//TODO convert to nanosleep
#define MSLEEP(msec)		usleep(msec * 1000)
#endif

/* ============================================================================================== */

#endif /* SCS_1_UNISTD_H_ */
