#ifndef NETWORK_SOCKET_H_
#define NETWORK_SOCKET_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../common_s.h"
#include "../network/context_s.h"
#include "scs/api.h"

void HPVT_SCS_initialize(char *path);
void HPVT_SCS_destroy(void);
void HPVT_dispose_socket(SCSSocketId scssock);

SCSSocketId HPVT_SCS_create_server_socket(uint16_t port);
SCSSocketId HPVT_SCS_create_client_socket(uint32_t peer_addr, uint16_t peer_port);

int HPVT_send(SCSSocketId sock, unsigned char* buffer, unsigned int bufflen);
int HPVT_recv(SCSSocketId sock, unsigned char* buffer, unsigned int bufflen);

int HPVT_UDP_create_server_socket(uint16_t port);
int HPVT_UDP_create_client_socket(uint32_t peer_addr, uint16_t peer_port);

boolean HPVT_SCS_add_socket_callback(SCSSocketId socket_id, SCSCallbackId *callback_id);
boolean HPVT_SCS_remove_socket_callback(SCSCallbackId callback_id);
boolean HPVT_SCS_set_socket_custom_feedback_callback(SCSSocketId socket_id);

boolean HPVT_SCS_get_socket_info(SCSSocketId socket_id, char *local_addr, char *remote_addr, int *mtu, uint32_t *connection_id);
boolean HPVT_SCS_set_transmitter_hasty_data(SCSSocketId socket_id);
boolean HPVT_SCS_get_transmitter_hasty_data(SCSSocketId socket_id);

void HPVT_SCS_queue_clear(SCSSocketId socket_id);

uint64_t HPVT_calculate_send_throughout(HPVT_NETWORK_TRAFFIC_INFO *prev, HPVT_NETWORK_TRAFFIC_INFO *curr, int diff_time);
uint64_t HPVT_calculate_recieve_throughout(HPVT_NETWORK_TRAFFIC_INFO *prev, HPVT_NETWORK_TRAFFIC_INFO *curr, int diff_time);

void HPVT_change_connection_state(int new_state);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NETWORK_SOCKET_H_ */
