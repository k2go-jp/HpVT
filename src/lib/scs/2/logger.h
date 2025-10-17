#ifndef SCS_2_LOGGER_H_
#define SCS_2_LOGGER_H_ 1

/* ============================================================================================== */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#error
#else
#include <pthread.h>
#include <syslog.h>
#endif /* WIN32 */

#include "scs/1/stdbool.h"
#include "scs/2/atomic.h"
#include "scs/2/lock.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

typedef int SCSLogType;

#define SCS_LOGTYPE_ALL				0
#define SCS_LOGTYPE_SYSTEM			11
#define SCS_LOGTYPE_MEMORY			12
#define SCS_LOGTYPE_SOCKET			13
#define SCS_LOGTYPE_PACKET			14
#define SCS_LOGTYPE_DEBUG			99

extern bool SCSLogTypeValidate(SCSLogType value);

/* ---------------------------------------------------------------------------------------------- */

typedef int SCSLogLevel;

#define SCS_LOGLEVEL_DEBUG			7
#define SCS_LOGLEVEL_INFO			6
#define SCS_LOGLEVEL_NOTICE			5
#define SCS_LOGLEVEL_WARN			4
#define SCS_LOGLEVEL_ERROR			3
#define SCS_LOGLEVEL_CRITITCAL		2
#define SCS_LOGLEVEL_ALERT			1
#define SCS_LOGLEVEL_EMERGEMCY		0

#define SCS_LOGLEVEL_FATAL			SCS_LOGLEVEL_EMERGEMCY
#define SCS_LOGLEVEL_ANY			SCS_LOGLEVEL_DEBUG

extern bool SCSLogLevelValidate(SCSLogLevel value);

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSErrorCounter {
	scs_atomic_uint64 total;
	scs_atomic_uint64 warn;
	scs_atomic_uint64 error;
	scs_atomic_uint64 alert;
	scs_atomic_uint64 fatal;
} SCSErrorCounter;

typedef struct SCSErrorCounterStatus {
	uint64_t total;
	uint64_t warn;
	uint64_t error;
	uint64_t alert;
	uint64_t fatal;
} SCSErrorInfo;

//extern void SCSErrorCounterInitialize(SCSErrorCounter * self);
//extern void SCSErrorCounterFinalize(SCSErrorCounter * self);

//extern SCSErrorCounterCopy SCSErrorCounterGetCopy(SCSErrorCounter * self);

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSLoggerSettings {
	bool quiet;
	SCSLogLevel level;
	char * path;
	FILE * fp;
} SCSLoggerSettings;

//extern void SCSLoggerSettingsInitialize(SCSLoggerSettings * self);
//extern void SCSLoggerSettingsFinalize(SCSLoggerSettings * self);

//extern void SCSLoggerSettingsOpenFile( 		//
//		SCSLoggerSettings * __restrict self, 	//
//		const char * __restrict path, 			//
//		const char * __restrict filename);

//extern void SCSLoggerSettingsDump( 			//
//		SCSLoggerSettings * __restrict self, 	//
//		const char * __restrict prefix);

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSLogger {
	scs_mutex mutex;

	char * path;
	SCSLoggerSettings system;
	SCSLoggerSettings memory;
	SCSLoggerSettings socket;
	SCSLoggerSettings packet;
	SCSLoggerSettings debug;

	SCSErrorCounter counter;
} SCSLogger;

extern void SCSLoggerInitialize(SCSLogger * self);
extern void SCSLoggerFinalize(SCSLogger * self);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSLoggerSetLevel(SCSLogger * self, SCSLogType type, SCSLogLevel level);

/* ---------------------------------------------------------------------------------------------- */

extern bool SCSLoggerOpenFiles(SCSLogger * __restrict self, const char * __restrict path);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSLoggerWrite(							//
		SCSLogger * __restrict self, 				//
		SCSLogType type, 							//
		SCSLogLevel level, 							//
		__const char * __restrict format, 			//
		...);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSLoggerGetErrorInfo(SCSLogger * __restrict self, SCSErrorInfo * __restrict out);

/* ---------------------------------------------------------------------------------------------- */

extern void SCSLoggerDump(SCSLogger * __restrict self, __const char * __restrict prefix);

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_LOGTYPENAME_SYSTEM		"SYS"
#define _SCS_LOGTYPENAME_MEMORY		"MEM"
#define _SCS_LOGTYPENAME_SOCKET		"SKT"
#define _SCS_LOGTYPENAME_PACKET		"PKT"
#define _SCS_LOGTYPENAME_DEBUG		"DBG"

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_LOGLEVELNAME_DEBUG		"DBG"
#define _SCS_LOGLEVELNAME_INFO		"INF"
#define _SCS_LOGLEVELNAME_NOTICE	"NTC"
#define _SCS_LOGLEVELNAME_WARN		"WRN"
#define _SCS_LOGLEVELNAME_ERROR		"ERR"
#define _SCS_LOGLEVELNAME_ALERT		"ALR"
#define _SCS_LOGLEVELNAME_FATAL		"FTL"

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_LOGMSG_00000	"-"

#define _SCS_LOGMSG_00001	"Memory leak."
#define _SCS_LOGMSG_00002	"Failed to allocate memory."
#define _SCS_LOGMSG_00003	"Unknown parameter."

#define _SCS_LOGMSG_10001	"Start."
#define _SCS_LOGMSG_10002	"Stop."

#define _SCS_LOGMSG_10011	"Bind."
#define _SCS_LOGMSG_10012	"Connect."
#define _SCS_LOGMSG_10013	"Listen."
#define _SCS_LOGMSG_10014	"Accept."
#define _SCS_LOGMSG_10015	"Connected."
#define _SCS_LOGMSG_10016	"Broken."
#define _SCS_LOGMSG_10017	"Close."
#define _SCS_LOGMSG_10018	"Timed out."
#define _SCS_LOGMSG_10019	"Shutdown."
#define _SCS_LOGMSG_10020	"Abort."

#define _SCS_LOGMSG_10021	"Sent syn."
#define _SCS_LOGMSG_10022	"Receive syn."
#define _SCS_LOGMSG_10023	"Sent syn-ack."
#define _SCS_LOGMSG_10024	"Receive syn-ack."
#define _SCS_LOGMSG_10025	"Sent ack."
#define _SCS_LOGMSG_10026	"Receive ack."
#define _SCS_LOGMSG_10027	"Sent fin."
#define _SCS_LOGMSG_10028	"Receive fin."
#define _SCS_LOGMSG_10029	"Sent fin-ack."
#define _SCS_LOGMSG_10030	"Receive fin-ack."

#define _SCS_LOGMSG_10041	"Update PLR."
#define _SCS_LOGMSG_10042	"Update RTT."
#define _SCS_LOGMSG_10043	"Update Path MTU."
#define _SCS_LOGMSG_10044	"Update MTU."

#define _SCS_LOGMSG_80001	"Argument is null."
#define _SCS_LOGMSG_80002	"Invalid argument."
#define _SCS_LOGMSG_80003	"Failed to create a socket."
#define _SCS_LOGMSG_80004	"Missing socket."

#define _SCS_LOGMSG_79811	"Missing socket."

#define _SCS_LOGMSG_79851	"Callback error."

#define _SCS_LOGMSG_79911	"Read error."

#define _SCS_LOGMSG_79921	"Write error."

#define _SCS_LOGMSG_79931	"Verify error."

#define _SCS_LOGMSG_79971	"Receive error."
#define _SCS_LOGMSG_79972	"Receive timeout."

#define _SCS_LOGMSG_79981	"Send error."
#define _SCS_LOGMSG_79982	"Send timeout."

#define _SCS_LOGMSG_79994	"Too many entries."
#define _SCS_LOGMSG_79995	"Adapt."
#define _SCS_LOGMSG_79996	"Invalid value."

#define _SCS_LOGMSG_99997	"Invalid argument."
#define _SCS_LOGMSG_99998	"Argument is null."
#define _SCS_LOGMSG_99999	"Illegal error."

/* ---------------------------------------------------------------------------------------------- */

#define _SCS_LOGGER_INCREASE_DEBUG
#define _SCS_LOGGER_INCREASE_INFO
#define _SCS_LOGGER_INCREASE_NOTICE
#define _SCS_LOGGER_INCREASE_WARN \
		SCSAtomicIncrease(_scs_global_logger.counter.total);\
		SCSAtomicIncrease(_scs_global_logger.counter.warn)
#define _SCS_LOGGER_INCREASE_ERROR \
		SCSAtomicIncrease(_scs_global_logger.counter.total);\
		SCSAtomicIncrease(_scs_global_logger.counter.error)
#define _SCS_LOGGER_INCREASE_ALERT \
		SCSAtomicIncrease(_scs_global_logger.counter.total);\
		SCSAtomicIncrease(_scs_global_logger.counter.alert)
#define _SCS_LOGGER_INCREASE_FATAL \
		SCSAtomicIncrease(_scs_global_logger.counter.total);\
		SCSAtomicIncrease(_scs_global_logger.counter.fatal)

/* ---------------------------------------------------------------------------------------------- */

extern SCSLogger _scs_global_logger;

#ifdef SCS_LOG
#error
#endif

#define SCS_LOG(xxx_level, xxx_type, xxx_code, xxx_format, xxx_args...) \
		SCSLoggerWrite(\
				&_scs_global_logger, \
				SCS_LOGTYPE_ ## xxx_type, \
				SCS_LOGLEVEL_ ## xxx_level, \
				"%-8s%05d %s %s %05d %s "xxx_format "\n", \
				SCS_SOURCECODE_FILEID, \
				__LINE__, \
				_SCS_LOGTYPENAME_ ## xxx_type, \
				_SCS_LOGLEVELNAME_ ## xxx_level, \
				xxx_code, \
				_SCS_LOGMSG_ ## xxx_code, \
				##xxx_args);\
		_SCS_LOGGER_INCREASE_ ## xxx_level

/* ---------------------------------------------------------------------------------------------- */

#ifdef SCS_DUMP
#error
#endif

#if SCS_ENABLE_DEBUG == 0
#define SCS_DUMP(xxx_ptr, xxx_length, xxx_prefix, xxx_caption, xxx_format, xxx_args...) \
		snprintf(xxx_ptr, xxx_length, "%s%s", xxx_prefix, xxx_caption);\
		syslog(SCS_LOGLEVEL_CRITITCAL, "+ %s = "xxx_format, xxx_ptr, ##xxx_args)
#else
#define SCS_DUMP(xxx_ptr, xxx_length, xxx_prefix, xxx_caption, xxx_format, xxx_args...) \
		snprintf(xxx_ptr, xxx_length, "%s%s", xxx_prefix, xxx_caption);\
		printf("+ %s = "xxx_format"\n", xxx_ptr, ##xxx_args)
#endif

/* ---------------------------------------------------------------------------------------------- */

#ifdef SCS_NOTIFY
#error
#endif

#define SCS_NOTIFY(xxx_format, xxx_args...) \
		syslog(SCS_LOGLEVEL_CRITITCAL, xxx_format, ##xxx_args)

/* ============================================================================================== */

#endif /* SCS_2_LOGGER_H_ */
