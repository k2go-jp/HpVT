#ifndef LOGGER_LOGGER_H_
#define LOGGER_LOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "logger_s.h"

/* ---------------------------------------------------------------------------------------------------- */
extern void HPVT_Logger_setup(const char * ident, LOG_FACILITY facility, LOG_LEVEL level);
extern void HPVT_Logger_teardown(void);
extern void HPVT_Logger_setLevel(LOG_LEVEL level);
extern LOG_LEVEL HPVT_Logger_getLevel(void);

/* ---------------------------------------------------------------------------------------------------- */
extern LOG_FACILITY global_log_facility;
extern LOG_LEVEL global_log_level;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LOGGER_LOGGER_H_ */
