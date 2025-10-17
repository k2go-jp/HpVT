#ifndef CONFIG_CONFIG_H_
#define CONFIG_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../network/context_s.h"
#include "config_s.h"

void HPVT_Config_initialize(HPVT_Config * object);
void HPVT_Config_dispose(HPVT_Config * object);

HPVT_Config * HPVT_Config_create(void);
void HPVT_Config_delete(HPVT_Config * object);

boolean HPVT_Config_load(HPVT_Config * object, const char * path, int particular);
boolean HPVT_Config_fix(HPVT_Config * object, HPVT_Context *context);
boolean HPVT_Config_verify(HPVT_Config * object, HPVT_Context *context);

boolean HPVT_Config_is_server(HPVT_Config_TYPE type, HPVT_Config_CONNECTION_MODE mode);
void HPVT_Config_logging(HPVT_Config * object);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CONFIG_CONFIG_H_ */
