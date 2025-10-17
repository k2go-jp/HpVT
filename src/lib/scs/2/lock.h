#ifndef SCS_2_LOCK_H_
#define SCS_2_LOCK_H_ 1

/* ============================================================================================== */

#ifdef WIN32
#include <Windows.h>
#else /* WIN32 */
#include <pthread.h>
#endif /* WIN32 */

/* ---------------------------------------------------------------------------------------------- */

#ifdef WIN32
typedef CRITICAL_SECTION scs_mutex;
#else /* WIN32 */
typedef pthread_mutex_t scs_mutex;
#endif /* WIN32 */

#ifdef WIN32
#error
#else /* WIN32 */
#define SCSMutexInitializer	PTHREAD_MUTEX_INITIALIZER
#endif /* WIN32 */

#ifdef WIN32
#error
#else /* WIN32 */
#define SCSMutexInitialize(xxx_self) \
		pthread_mutex_init(&(xxx_self), NULL)
#define SCSMutexFinalize(xxx_self) \
		pthread_mutex_destroy(&(xxx_self))
#define SCSMutexLock(xxx_self) \
		pthread_mutex_lock(&(xxx_self))
#define SCSMutexTryLock(xxx_self) \
		pthread_mutex_trylock(&(xxx_self))
#define SCSMutexUnlock(xxx_self) \
		pthread_mutex_unlock(&(xxx_self))
#endif /* WIN32 */

/* ---------------------------------------------------------------------------------------------- */

#ifdef WIN32
typedef SRWLOCK scs_rwlock;
#else /* WIN32 */
typedef pthread_rwlock_t scs_rwlock;
#endif /* WIN32 */

#ifdef WIN32
#define SCSRWLockInitialize(xxx_self) \
		InitializeSRWLock(xxx_self)
#define SCSRWLockFinalize(xxx_self)
#define SCSRWLockReadLock(xxx_self) \
		AcquireSRWLockShared(xxx_self)
#define SCSRWLockReadUnlock(xxx_self) \
		ReleaseSRWLockShared(xxx_self)
#define SCSRWLockWriteLock(xxx_self) \
		AcquireSRWLockExclusive(xxx_self)
#define SCSRWLockWriteUnlock(xxx_self) \
		ReleaseSRWLockExclusive(xxx_self)
#else /* WIN32 */
#define SCSRWLockInitialize(xxx_self) \
		pthread_rwlock_init(xxx_self, NULL)
#define SCSRWLockFinalize(xxx_self) \
		pthread_rwlock_destroy(xxx_self)
#define SCSRWLockReadLock(xxx_self) \
		pthread_rwlock_rdlock(xxx_self)
#define SCSRWLockReadUnlock(xxx_self) \
		pthread_rwlock_unlock(xxx_self)
#define SCSRWLockWriteLock(xxx_self) \
		pthread_rwlock_wrlock(xxx_self)
#define SCSRWLockWriteUnlock(xxx_self) \
		pthread_rwlock_unlock(xxx_self)
#endif /* WIN32 */

/* ---------------------------------------------------------------------------------------------- */

#ifdef WIN32
typedef CONDTION_VARIABLE scs_cond;
#else /* WIN32 */
typedef pthread_cond_t scs_cond;
#endif /* WIN32 */

#ifdef WIN32
#error
#else /* WIN32 */
#define SCSCondInitialize(xxx_self) \
		pthread_cond_init(&(xxx_self), NULL);
#define SCSCondFinalize(xxx_self) \
		pthread_cond_destroy(&(xxx_self));
#define SCSCondWait(xxx_self, xxx_mutex) \
		pthread_cond_wait(&(xxx_self), &xxx_mutex)
#define SCSCondTimedwait(xxx_self, xxx_mutex, xxx_timeout) \
		pthread_cond_timedwait(&(xxx_self), &xxx_mutex, &xxx_timeout)
#define SCSCondSignal(xxx_self) \
		pthread_cond_signal(&(xxx_self))
#endif /* WIN32 */

/* ============================================================================================== */

#endif /* SCS_2_LOCK_H_ */
