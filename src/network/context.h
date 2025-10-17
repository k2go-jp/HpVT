#ifndef NETWORK_CONTEXT_H_
#define NETWORK_CONTEXT_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../config/config_s.h"
#include "context_s.h"

void HPVT_Context_initialize(HPVT_Context *context);
void HPVT_Context_destroy(HPVT_Context *context);
void HPVT_Context_setup(HPVT_Context *context, HPVT_Config *config);
boolean HPVT_Context_is_transmitter(HPVT_Context *context);
boolean HPVT_Context_is_session_connected(HPVT_Context *context);
const char* HPVT_Context_get_connection_state_string(int state);
void HPVT_Context_ready_wait(HPVT_Context *context);
void HPVT_Context_activate(HPVT_Context *context);
void HPVT_Context_deactivate(HPVT_Context *context);
void HPVT_Context_update_encode_parameters(HPVT_Context *context, int width, int height, int framerate);
void HPVT_Context_update_receivable_mtu_size(HPVT_Context *context, uint16_t receivable_mtu);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NETWORK_CONTEXT_H_ */
