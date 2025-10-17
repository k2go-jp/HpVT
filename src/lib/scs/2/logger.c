//#define SCS_SOURCECODE_FILEID	"2LOG"
//
/* ============================================================================================== */

#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#ifdef WIN32
#else /* WIN32 */
#include <unistd.h>
#include <sys/stat.h>
#endif

#include "scs/1/stdbool.h"
#include "scs/2/atomic.h"
#include "scs/2/lock.h"
#include "scs/2/logger.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSLogTypeValidate(SCSLogType value) {

	switch (value) {
		case SCS_LOGTYPE_ALL:
		case SCS_LOGTYPE_SYSTEM:
		case SCS_LOGTYPE_MEMORY:
		case SCS_LOGTYPE_SOCKET:
		case SCS_LOGTYPE_PACKET:
		case SCS_LOGTYPE_DEBUG:
			return true;
		default:
			return false;
	}

}

/* ---------------------------------------------------------------------------------------------- */

inline bool SCSLogLevelValidate(SCSLogLevel value) {

	switch (value) {
		case SCS_LOGLEVEL_DEBUG:
		case SCS_LOGLEVEL_INFO:
		case SCS_LOGLEVEL_NOTICE:
		case SCS_LOGLEVEL_WARN:
		case SCS_LOGLEVEL_ERROR:
		case SCS_LOGLEVEL_CRITITCAL:
		case SCS_LOGLEVEL_ALERT:
		case SCS_LOGLEVEL_EMERGEMCY:
			return true;
		default:
			return false;
	}

}

/* ---------------------------------------------------------------------------------------------- */

static inline void SCSErrorCounterInitialize(SCSErrorCounter * self) {

//	if (self == NULL) {
//		return;
//	}

//	memset(self, 0, sizeof(SCSErrorCounter));

	SCSAtomicInitialize(self->total, 0);
	SCSAtomicInitialize(self->warn, 0);
	SCSAtomicInitialize(self->error, 0);
	SCSAtomicInitialize(self->alert, 0);
	SCSAtomicInitialize(self->fatal, 0);

}
static inline void SCSErrorCounterFinalize(SCSErrorCounter * self) {

//	if (self == NULL) {
//		return;
//	}

	SCSAtomicFinalize(self->total, 0);
	SCSAtomicFinalize(self->warn, 0);
	SCSAtomicFinalize(self->error, 0);
	SCSAtomicFinalize(self->alert, 0);
	SCSAtomicFinalize(self->fatal, 0);

//	memset(self, 0, sizeof(SCSErrorCounter));

}

static inline void SCSErrorCounterGet(				//
		SCSErrorCounter * __restrict self, 			//
		SCSErrorInfo * __restrict out) {
	SCSErrorInfo tmp_value;

//	if (self == NULL) {
//		return;
//	}

//	if (out == NULL) {
//		return;
//	}

	memset(&tmp_value, 0, sizeof(SCSErrorInfo));

	tmp_value.total = SCSAtomicGet(self->total);
	tmp_value.warn = SCSAtomicGet(self->warn);
	tmp_value.error = SCSAtomicGet(self->error);
	tmp_value.alert = SCSAtomicGet(self->alert);
	tmp_value.fatal = SCSAtomicGet(self->fatal);

	memcpy(out, &tmp_value, sizeof(SCSErrorInfo));

}

/* ---------------------------------------------------------------------------------------------- */

static inline void SCSLoggerSettingsInitialize(SCSLoggerSettings * self) {

//	if (self == NULL) {
//		return;
//	}

//	memset(self, 0, sizeof(SCSLoggerSettings));

//	self->quiet = false;
	self->level = SCS_LOGLEVEL_ANY;
//	self->path = NULL;
//	self->fp = NULL;

}
static inline void SCSLoggerSettingsFinalize(SCSLoggerSettings * self) {

//	if (self == NULL) {
//		return;
//	}

//	self->quiet = false;
//	self->level = 0;
	if (self->path != NULL) {
		free(self->path);
		self->path = NULL;
	}
	if (self->fp != NULL) {
		fclose(self->fp);
		self->fp = NULL;
	}

//	memset(self, 0, sizeof(SCSLoggerSettings));

}

static inline void SCSLoggerSettingsOpenFile( 	//
		SCSLoggerSettings * __restrict self, 	//
		const char * __restrict path, 			//
		const char * __restrict filename) {
	char tmp_path[PATH_MAX];
	FILE * tmp_fp;

//	if (self == NULL) {
//		return;
//	}

	snprintf(tmp_path, sizeof(tmp_path), "%s/%s", path, filename);

	if ((tmp_fp = fopen(tmp_path, "w+")) == NULL) {
		return;
	}

	self->path = strdup(tmp_path);
	self->fp = tmp_fp;

}

static inline void SCSLoggerSettingsDump( 		//
		SCSLoggerSettings * __restrict self, 	//
		const char * __restrict prefix) {
	const char tmp_caption[] = "logger.";
	char tmp_prefix[UINT8_MAX];
	char tmp_buffer[UINT8_MAX];

//	if (self == NULL) {
//		return;
//	}

//	if (prefix == NULL) {
//		prefix = "";
//	}

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s", prefix, tmp_caption);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, //
			"quiet", "%s", (self->quiet ? "Yes" : "no"));
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, //
			"level", "(%d)", self->level);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, //
			"path", "%s(%s)", self->path, (self->fp != NULL ? "Open" : "Close"));

}

/* ---------------------------------------------------------------------------------------------- */

#ifdef _SCS_LOCK
#error
#endif
#ifdef _SCS_UNLOCK
#error
#endif

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_LOCK(xxx_self) \
	if (SCSMutexLock((xxx_self)->mutex) != 0) {\
		abort();\
	}
#define _SCS_UNLOCK(xxx_self) \
	if (SCSMutexUnlock((xxx_self)->mutex) != 0) {\
		abort();\
	}

/* ---------------------------------------------------------------------------------------------- */

void SCSLoggerInitialize(SCSLogger * self) {

	if (self == NULL) {
		return;
	}

	memset(self, 0, sizeof(SCSLogger));

	SCSMutexInitialize(self->mutex);

	SCSLoggerSettingsInitialize(&self->system);
	SCSLoggerSettingsInitialize(&self->memory);
	SCSLoggerSettingsInitialize(&self->socket);
	SCSLoggerSettingsInitialize(&self->packet);
	SCSLoggerSettingsInitialize(&self->debug);

	SCSErrorCounterInitialize(&self->counter);

}

void SCSLoggerFinalize(SCSLogger * self) {

	if (self == NULL) {
		return;
	}

	SCSMutexFinalize(self->mutex);

	if (self->path != NULL) {
		free(self->path);
		self->path = NULL;
	}

	SCSLoggerSettingsFinalize(&self->system);
	SCSLoggerSettingsFinalize(&self->memory);
	SCSLoggerSettingsFinalize(&self->socket);
	SCSLoggerSettingsFinalize(&self->packet);
	SCSLoggerSettingsFinalize(&self->debug);

	SCSErrorCounterFinalize(&self->counter);

	memset(self, 0, sizeof(SCSLogger));

}

/* ---------------------------------------------------------------------------------------------- */

void SCSLoggerSetLevel(SCSLogger * self, SCSLogType type, SCSLogLevel level) {

//	if (SCSLogLevelValidate(level) == false) {
//		SCS_LOG(WARN, SYSTEM, 80002,  "<<%d>>", level);
//		return;
//	}

	_SCS_LOCK(self);

	if (type == SCS_LOGTYPE_ALL) {
		self->system.level = level;
		self->memory.level = level;
		self->socket.level = level;
		self->packet.level = level;
		self->debug.level = level;
	}
	else {
		switch (type) {
			case SCS_LOGTYPE_SYSTEM: {
				self->system.level = level;
				break;
			}
			case SCS_LOGTYPE_MEMORY: {
				self->memory.level = level;
				break;
			}
			case SCS_LOGTYPE_SOCKET: {
				self->socket.level = level;
				break;
			}
			case SCS_LOGTYPE_PACKET: {
				self->packet.level = level;
				break;
			}
			case SCS_LOGTYPE_DEBUG: {
				self->debug.level = level;
				break;
			}
			default: {
				return;
			}
		}
	}

	_SCS_UNLOCK(self);

}

/* ---------------------------------------------------------------------------------------------- */

bool SCSLoggerOpenFiles(SCSLogger * __restrict self, __const char * __restrict path) {
	char * tmp_path;
	struct stat tmp_stat;

	if (self == NULL) {
		return false;
	}

	if (path == NULL) {
		return false;
	}

	if (access(path, F_OK) != 0) {
		if (mkdir(path, (S_IWUSR | S_IWGRP | S_IWGRP)) != 0) {
			return false;
		}
	}

	if (access(path, W_OK) != 0) {
		return false;
	}

	if (stat(path, &tmp_stat) != 0) {
		return false;
	}

	if (!S_ISDIR(tmp_stat.st_mode)) {
		return false;
	}

	if ((tmp_path = strdup(path)) == NULL) {
		return false;
	}

	//TODO Strip white-space
	//TODO Remove tail '/'

	_SCS_LOCK(self);

	self->path = tmp_path;
	SCSLoggerSettingsOpenFile(&self->system, tmp_path, "system.log");
	SCSLoggerSettingsOpenFile(&self->memory, tmp_path, "memory.log");
	SCSLoggerSettingsOpenFile(&self->socket, tmp_path, "socket.log");
	SCSLoggerSettingsOpenFile(&self->packet, tmp_path, "packet.log");
	SCSLoggerSettingsOpenFile(&self->debug, tmp_path, "debug.log");

	_SCS_UNLOCK(self);

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSLoggerWrite(SCSLogger * __restrict self, SCSLogType type, SCSLogLevel level,
		__const char * __restrict format, ...) {
	va_list tmp_args;
	SCSLogLevel tmp_level;
	FILE * tmp_fp;

	if (self == NULL) {
		return;
	}

	if (format == NULL) {
		return;
	}

	if (SCSLogTypeValidate(type) == false) {
		return;
	}

	if (SCSLogLevelValidate(level) == false) {
		return;
	}

	_SCS_LOCK(self);

	switch (type) {
		case SCS_LOGTYPE_ALL: {
			tmp_level = -1;
			tmp_fp = NULL;
			return;
		}
		case SCS_LOGTYPE_DEBUG: {
			tmp_level = self->debug.level;
			tmp_fp = self->debug.fp;
			break;
		}
		case SCS_LOGTYPE_MEMORY: {
			tmp_level = self->memory.level;
			tmp_fp = self->memory.fp;
			break;
		}
		case SCS_LOGTYPE_PACKET: {
			tmp_level = self->packet.level;
			tmp_fp = self->packet.fp;
			break;
		}
		case SCS_LOGTYPE_SOCKET: {
			tmp_level = self->socket.level;
			tmp_fp = self->socket.fp;
			break;
		}
		case SCS_LOGTYPE_SYSTEM: {
			tmp_level = self->system.level;
			tmp_fp = self->system.fp;
			break;
		}
		default: {
			tmp_level = -1;
			tmp_fp = NULL;
			return;
		}
	}

	_SCS_UNLOCK(self);

	va_start(tmp_args, format);

	if (level <= tmp_level) {
		if (tmp_fp == NULL) {
			vsyslog(level, format, tmp_args);
		}
		else {
			struct timespec tmp_timestamp;

			clock_gettime(CLOCK_MONOTONIC, &tmp_timestamp);
			fprintf(tmp_fp, "[%zd.%09ld]", tmp_timestamp.tv_sec, tmp_timestamp.tv_nsec);
			vfprintf(tmp_fp, format, tmp_args);
			fflush(tmp_fp);
		}
	}

	va_end(tmp_args);

}

/* ---------------------------------------------------------------------------------------------- */

void SCSLoggerGetErrorInfo(SCSLogger * __restrict self, SCSErrorInfo * __restrict out) {
	return SCSErrorCounterGet(&self->counter, out);
}

/* ---------------------------------------------------------------------------------------------- */

void SCSLoggerDump(SCSLogger * __restrict self, __const char * __restrict prefix) {
	const char tmp_caption[] = "logger.";
	char tmp_prefix[UINT8_MAX];
	char tmp_buffer[UINT8_MAX];

	if (self == NULL) {
		return;
	}

	if (prefix == NULL) {
		prefix = "";
	}

	_SCS_LOCK(self);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s", prefix, tmp_caption);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "level", "%s", self->path);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "system.");
	SCSLoggerSettingsDump(&self->system, tmp_prefix);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "memory.");
	SCSLoggerSettingsDump(&self->memory, tmp_prefix);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "socket.");
	SCSLoggerSettingsDump(&self->socket, tmp_prefix);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "packet.");
	SCSLoggerSettingsDump(&self->packet, tmp_prefix);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "debug.");
	SCSLoggerSettingsDump(&self->debug, tmp_prefix);

	_SCS_UNLOCK(self);

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
