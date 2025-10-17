#ifndef NETWORK_QUEUE_H_
#define NETWORK_QUEUE_H_

#include "../../config/config_s.h"

uint32_t HPVT_get_current_time(void);
int HPVT_compare_frame_sequence_number(HPVT_Queue_FRAME_SEQNO seqno_former, HPVT_Queue_FRAME_SEQNO seqno_latter);
void HPVT_enqueue_counter_up(void);
int HPVT_enqueue_video_frame_encoded(void *buffer, int length, boolean is_keyframe, boolean is_codec_hevc);
void HPVT_start_thread_packetizer(HPVT_Config *config);
void HPVT_start_thread_packet_transmitter(HPVT_Config *config);
int HPVT_start_thread_notification_receiver(HPVT_Config *config);

void HPVT_start_thread_internal_info_sender(void);
boolean HPVT_parse_internal_info_packet(unsigned char *buffer, int length);

void HPVT_add_feeedback_data(uint32_t plr, uint64_t recv_throughput, uint64_t send_throughput);
void HPVT_start_thread_adaptive_controller(void);
void HPVT_start_thread_packet_receiver(HPVT_Config *config);
void HPVT_start_thread_received_packet_list(HPVT_Config *config);
boolean HPVT_get_video_frame_data(unsigned char* buffer, unsigned int* length, HPVT_Queue_FRAME_TYPE* frame_type);
int HPVT_send_notification_path_mtu_discovery(uint16_t mtu_value, HPVT_Queue_FRAME_SEQNO frame_seqno);
void HPVT_update_buffering_delay_time(void);

void HPVT_initialize_transmitter_objects(void);
void HPVT_initialize_receiver_objects(void);

void HPVT_UDP_setup_server_socket(uint16_t port);
boolean HPVT_SCS_setup_server_socket(uint16_t port);
void HPVT_UDP_setup_client_socket(uint32_t addr, uint16_t port);
boolean HPVT_SCS_setup_client_socket(uint32_t addr, uint16_t port);
void HPVT_UDP_reset_connection(void);
void HPVT_SCS_reset_connection(void);

#endif /* NETWORK_QUEUE_H_ */
