#define SCS_SOURCECODE_FILEID	"0INT"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/2/lock.h"
#include "scs/2/logger.h"
#include "scs/5/context/context.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_LOCK
#error
#endif
#ifdef _SCS_UNLOCK
#error
#endif

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_LOCK() \
		if (SCSMutexLock(_context.mutex) != 0) {\
			SCS_LOG(ALERT, SYSTEM, 00000,  "%s. <<%d>>", strerror(errno), errno);\
			abort();\
		}
#define _SCS_UNLOCK() \
		if (SCSMutexUnlock(_context.mutex) != 0) {\
			SCS_LOG(ALERT, SYSTEM, 00000,  "%s. <<%d>>", strerror(errno), errno);\
			abort();\
		}

/* ---------------------------------------------------------------------------------------------- */

static struct {
	scs_mutex mutex;
	SCSContext * ptr;
} _context = { SCSMutexInitializer, NULL };

/* ---------------------------------------------------------------------------------------------- */

static bool __SCSStartUp(SCSSettings * settings) {
	SCSSettings tmp_settings;
	SCSContext * tmp_context;

	if (settings != NULL) {
		/* Custom settings */
		SCSSettingsCopy(&tmp_settings, settings);
	}
	else {
		/* Default settings */
		SCSSettingsInitialize(&tmp_settings);
		SCSSettingsAdapt(&tmp_settings);
	}

	SCSSettingsDump(&tmp_settings, "");

	if (SCSSettingsValidate(&tmp_settings) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EINVALSETTINGS);
		return false;
	}

	if ((tmp_context = SCSContextCreate()) == NULL) {
		SCS_LOG(ERROR, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return false;
	}

	if (SCSContextStandBy(tmp_context, &tmp_settings) == true) {
		SCSContextWaitReady(tmp_context);
	}
	else {
		SCS_LOG(ERROR, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSContextDestroy(tmp_context);
		return false;
	}

	if (SCSContextHold(tmp_context) == false) {
		SCS_LOG(ERROR, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSContextDestroy(tmp_context);
		return false;
	}

	_context.ptr = tmp_context;

	return true;
}
bool _SCSStartUp(SCSSettings * settings) {
	bool tmp_result;

	_SCS_LOCK();

	if (_context.ptr == NULL) {
		tmp_result = __SCSStartUp(settings);
	}
	else {
		tmp_result = true;
	}

	_SCS_UNLOCK();

	return tmp_result;
}
void _SCSCleanUp(void) {

	_SCS_LOCK();

	if (_context.ptr != NULL) {
		SCSContextDestroy(_context.ptr);
		SCSContextFree(_context.ptr);
		_context.ptr = NULL;
	}

	_SCS_UNLOCK();

}

static inline SCSContext * __SCSGetContext(void) {
	SCSContext * tmp_context;

	if ((tmp_context = _context.ptr) == NULL) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_ENOTSTART);
		return NULL;
	}

	if (SCSContextHold(tmp_context) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return NULL;
	}

	return tmp_context;
}
SCSContext * _SCSGetContext(void) {
	SCSContext * tmp_context;

	_SCS_LOCK();

	tmp_context = __SCSGetContext();

	_SCS_UNLOCK();

	return tmp_context;
}

/* ---------------------------------------------------------------------------------------------- */

#undef _SCS_LOCK
#undef _SCS_UNLOCK

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
