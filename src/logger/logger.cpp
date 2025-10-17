#define FILE_NUMBER "L101"

#include "logger.h"

/* ---------------------------------------------------------------------------------------------------- */

LOG_FACILITY global_log_facility = LOG_FACILITY_LOCAL1;
LOG_LEVEL global_log_level = LOG_LEVEL_NOTICE;

/* ---------------------------------------------------------------------------------------------------- */


void HPVT_Logger_setup(const char * ident, LOG_FACILITY facility, LOG_LEVEL level)
{
	openlog(ident, LOG_PID, facility);

	global_log_facility = facility;
	global_log_level = level;
}
void HPVT_Logger_teardown(void)
{
	closelog();
}


void HPVT_Logger_setLevel(LOG_LEVEL level)
{
	global_log_level = level;
}
LOG_LEVEL HPVT_Logger_getLevel(void)
{
	return global_log_level;
}

