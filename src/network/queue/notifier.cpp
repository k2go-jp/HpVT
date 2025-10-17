#define FILE_NUMBER "Q203"

#include "../../config/config.h"
#include "../socket.h"
#include "../context.h"
#include "../network.h"
#include "queue.h"

#define HPVT_PATH_PATH_MTU_DISCOVERY_WAIT_TIME_INITIAL   60

extern HPVT_Context *g_context;
extern SCSSocketId g_scssock;
extern SCSCallbackId g_callback_id;

uint32_t HPVT_get_current_time(void) {

	struct timespec tmp_ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tmp_ts);

	return (tmp_ts.tv_sec * 1000000) + (tmp_ts.tv_nsec / 1000);
}
int HPVT_compare_frame_sequence_number(HPVT_Queue_FRAME_SEQNO seqno_former, HPVT_Queue_FRAME_SEQNO seqno_latter) {

	if (seqno_former == HPVT_Queue_FRAME_SEQNO_INVALID || seqno_latter == HPVT_Queue_FRAME_SEQNO_INVALID) {
		return 0;
	}

	int diff;

	if (seqno_former < seqno_latter) {

		if ((seqno_latter - seqno_former) < (HPVT_Queue_FRAME_SEQNO_MAXIMUM / 2)) {
			diff = seqno_latter - seqno_former;
		}
		else {
			diff = seqno_latter - seqno_former - HPVT_Queue_FRAME_SEQNO_MAXIMUM;
		}
	}
	else if (seqno_former == seqno_latter) {
		diff = 0;
	}
	else {

		if ((seqno_former - seqno_latter) < (HPVT_Queue_FRAME_SEQNO_MAXIMUM / 2)) {
			diff = seqno_latter - seqno_former;
		}
		else {
			diff = HPVT_Queue_FRAME_SEQNO_MAXIMUM - seqno_former + seqno_latter;
		}
	}

	return diff;
}
int HPVT_send_notification_path_mtu_discovery(uint16_t mtu_value, HPVT_Queue_FRAME_SEQNO frame_seqno) {

	unsigned char send_buffer[8];
	int ret;

	memset(send_buffer, 0, sizeof(send_buffer));

	send_buffer[0] = HPVT_PACKET_TYPE_PATH_MTU_DISCOVERY;
	memcpy(send_buffer + 1, &mtu_value, sizeof(mtu_value));
	memcpy(send_buffer + 3, &frame_seqno, sizeof(frame_seqno));

	ret = HPVT_send(g_scssock, (unsigned char *) send_buffer, 5);

	if (ret == -1) {
		HPVT_logging(LOG_LEVEL_WARNING, "notification packet send error");
		return -1;
	}

	HPVT_logging(LOG_LEVEL_NOTICE, "Sent notification packet(Path MTU size = %d, frame seqno = %d)", mtu_value, frame_seqno);

	return 0;
}
void HPVT_UDP_setup_server_socket(uint16_t port) {

	if ((g_scssock = HPVT_UDP_create_server_socket(port)) == -1) {
		HPVT_log_UNEXPECTED_ERROR();
	}
	HPVT_change_connection_state(HPVT_VIDEO_CONNECTION_STATE_CONNECTED);

	HPVT_logging(LOG_LEVEL_NOTICE, "Opened session");
}
boolean HPVT_SCS_setup_server_socket(uint16_t port) {

	HPVT_logging(LOG_LEVEL_NOTICE, "setup server socket");

	if ((g_scssock = HPVT_SCS_create_server_socket(port)) == SCS_SKTID_INVVAL) {
		HPVT_logging(LOG_LEVEL_ERROR, "SCS_create server socket error!");
		return false;
	}

	if (HPVT_SCS_add_socket_callback(g_scssock, &g_callback_id) == false) {
		HPVT_logging(LOG_LEVEL_ERROR, "add callback error!");
	}

	if (HPVT_SCS_set_socket_custom_feedback_callback(g_scssock) == false) {
		HPVT_logging(LOG_LEVEL_ERROR, "set callback error!");
	}

	HPVT_change_connection_state(HPVT_VIDEO_CONNECTION_STATE_CONNECTED);
	HPVT_logging(LOG_LEVEL_NOTICE, "Opened session");

	return true;
}
void HPVT_change_connection_state(int new_state) {

	if (new_state == g_context->connection.state) {
		return;
	}
	HPVT_logging(LOG_LEVEL_NOTICE, "Connection state=%s > %s", //
			HPVT_Context_get_connection_state_string(g_context->connection.state), HPVT_Context_get_connection_state_string(new_state));

	g_context->connection.state = new_state;
}
void HPVT_UDP_reset_connection(void) {

	HPVT_logging(LOG_LEVEL_NOTICE, "Reset connection");

	HPVT_Lock(g_context->connection.status_locked);

	if (g_context->connection.reconnection_attempt_on == true) {
		HPVT_change_connection_state(HPVT_VIDEO_CONNECTION_STATE_DISCONNECTED);
	}
	else {
		HPVT_change_connection_state(HPVT_VIDEO_CONNECTION_STATE_CLOSED);
	}

	g_context->connection.timeout_count = 0;
	g_context->connection.no_callback_count = 0;
	g_context->connection.flag_timeout = false;
	g_context->connection.self.receive_throughput = 0;

	HPVT_dispose_socket(g_scssock);
	g_scssock = SCS_SKTID_INVVAL;
	HPVT_logging(LOG_LEVEL_NOTICE, "Closed session");

	HPVT_Unlock(g_context->connection.status_locked);
}
void HPVT_SCS_reset_connection(void) {

	HPVT_logging(LOG_LEVEL_NOTICE, "Reset connection");

	HPVT_Lock(g_context->connection.status_locked);

	if (HPVT_SCS_remove_socket_callback(g_callback_id) == false) {
		HPVT_logging(LOG_LEVEL_ERROR, "remove callback error!");
	}

	g_callback_id == SCS_CALLBACKID_INVVAL;
	HPVT_SCS_queue_clear(g_scssock);

	if (g_context->connection.reconnection_attempt_on == true) {
		HPVT_change_connection_state(HPVT_VIDEO_CONNECTION_STATE_DISCONNECTED);
	}
	else {
		HPVT_change_connection_state(HPVT_VIDEO_CONNECTION_STATE_CLOSED);
	}

	g_context->connection.timeout_count = 0;
	g_context->connection.no_callback_count = 0;
	g_context->connection.flag_timeout = false;
	g_context->connection.self.receive_throughput = 0;

	g_context->connection.flag_reset_resolution = true;
	HPVT_dispose_socket(g_scssock);
	g_scssock = SCS_SKTID_INVVAL;
	HPVT_logging(LOG_LEVEL_NOTICE, "Closed session");

	HPVT_Unlock(g_context->connection.status_locked);
}
int HPVT_start_thread_notification_receiver(HPVT_Config *config) {

	HPVT_Context_ready_wait(g_context);

	unsigned char receive_buffer[1500];
	int receive_length;
	uint16_t tmp_mtu;
	uint16_t tmp_frame_seqno;
	uint16_t maximum_received_mtu;
	int same_value_count;
	int wait_time;
	int tick_count;

	same_value_count = 0;
	wait_time = 0;
	tick_count = 0;
	maximum_received_mtu = 0;

	while (true) {

		if (g_context->connection.state != HPVT_VIDEO_CONNECTION_STATE_CONNECTED) {
			sleep(1);
			continue;
		}

		if (tick_count < wait_time) {

			sleep(1);
			tick_count++;
			continue;
		}

		g_context->connection.path_mtu_discovery.flag_start_discovery = true;
		memset(receive_buffer, 0, sizeof(receive_buffer));
		receive_length = HPVT_recv(g_scssock, receive_buffer, sizeof(receive_buffer) - 1);

		if (receive_length > 0) {
		}
		else {

			sleep(5);
			continue;
		}

		if (receive_buffer[0] == HPVT_PACKET_TYPE_PATH_MTU_DISCOVERY) {
			memcpy(&tmp_mtu, receive_buffer + 1, sizeof(tmp_mtu));
			memcpy(&tmp_frame_seqno, receive_buffer + 3, sizeof(tmp_frame_seqno));

			HPVT_logging(LOG_LEVEL_NOTICE, "Received notification packet(Path MTU size = %d, frame seqno = %d)", tmp_mtu, tmp_frame_seqno);

			if (tmp_mtu == maximum_received_mtu) {
				same_value_count++;
			}
			else {
				same_value_count = 0;
			}

			if (tmp_mtu > maximum_received_mtu) {

				HPVT_Context_update_receivable_mtu_size(g_context, tmp_mtu);
				maximum_received_mtu = tmp_mtu;
			}

			if (same_value_count == 5) {

				g_context->connection.path_mtu_discovery.flag_start_discovery = false;

				same_value_count = 0;
				maximum_received_mtu = 0;
				tick_count = 0;

				if (wait_time == 0) {
					wait_time = HPVT_PATH_PATH_MTU_DISCOVERY_WAIT_TIME_INITIAL;
				}
				else {
					wait_time = wait_time * 2;
				}

				if (wait_time > (4 * 60 * 60)) {
					wait_time = HPVT_PATH_PATH_MTU_DISCOVERY_WAIT_TIME_INITIAL;
				}

			}
		}
		else {
			HPVT_logging(LOG_LEVEL_WARNING, "Received invalid packet! ");
			continue;
		}

	}

	return 0;
}
boolean HPVT_parse_internal_info_packet(unsigned char *buffer, int length) {

	HPVT_Queue_FRAME_SEQNO tmp_seqno;
	uint64_t tmp_time;
	uint8_t tmp_function_flags;
	uint16_t tmp_adc_max_bitrate = 0;
	uint8_t tmp_adc_max_framerate = 0;
	int point = 0;

	point += 1;

	while (point < length) {
		switch (buffer[point]) {
			case HPVT_TELEMETRY_KEY_CODE_FRAME_GENERATED_TIME:
				point += 2;
				memcpy(&tmp_seqno, buffer + point, sizeof(HPVT_Queue_FRAME_SEQNO));
				point += sizeof(HPVT_Queue_FRAME_SEQNO);
				memcpy(&tmp_time, buffer + point, sizeof(uint64_t));
				point += sizeof(uint64_t);
				break;

			case HPVT_TELEMETRY_KEY_CODE_FUNCTION_FLAGS:
				point += 2;
				memcpy(&tmp_function_flags, buffer + point, sizeof(uint8_t));
				point += sizeof(uint8_t);
				break;

			case HPVT_TELEMETRY_KEY_CODE_ADAPTIVE_CONTROL_STATUS:
				point += 2;
				memcpy(&tmp_adc_max_bitrate, buffer + point, sizeof(uint16_t));
				point += sizeof(uint16_t);
				memcpy(&tmp_adc_max_framerate, buffer + point, sizeof(uint8_t));
				point += sizeof(uint8_t);
				break;

			default:
				HPVT_logging(LOG_LEVEL_ERROR, "Received invalid packet! (%02x)", buffer[point]);
				return false;
		}
	}

	HPVT_logging(LOG_LEVEL_NOTICE, "recv INTERNAL INFO length=%d,frame(seqno=%d,generated_time=%llu),"	//
			"adc=(bitrate=%d),flags=%02x",//
			length, tmp_seqno, tmp_time, tmp_adc_max_bitrate, tmp_function_flags);

	g_context->connection.internal_info.seqno = tmp_seqno;
	g_context->connection.internal_info.captured_time = tmp_time;
	g_context->connection.internal_info.flags = tmp_function_flags;
	g_context->connection.internal_info.adaptive.bitrate = tmp_adc_max_bitrate;

	if (tmp_function_flags & HPVT_TELEMETRY_FUNCTION_FLAG_VIDEO_ADAPTIVE_ON) {
		g_context->settings.video.adaptive_control = true;
	}
	else {
		g_context->settings.video.adaptive_control = false;
	}

	return true;
}
