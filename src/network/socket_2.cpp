#define FILE_NUMBER "N112"

#include "../network/context.h"
#include "queue/queue.h"
#include "socket.h"

extern HPVT_Context *g_context;

SCSNetworkTrafficInfo *g_traffic_info_self = NULL;
SCSNetworkTrafficInfo *g_traffic_info_peer = NULL;

static uint64_t previous_packet_count = 0;
static SCSSocketCallbackStatus previous_callback_status;

static void _HPVT_calculate_throughout(__const SCSSocketCallbackStatus * __restrict status) {

	if (status->socket.feedback.basic.timestamp == 0) {
		g_context->connection.self.receive_throughput = 0;
		return;
	}

	g_context->connection.self.receive_throughput = HPVT_calculate_recieve_throughout((HPVT_NETWORK_TRAFFIC_INFO *) &previous_callback_status.socket.traffic, //
			(HPVT_NETWORK_TRAFFIC_INFO *) &status->socket.traffic, 1);

	g_context->connection.self.send_throughput = HPVT_calculate_send_throughout((HPVT_NETWORK_TRAFFIC_INFO *) &previous_callback_status.socket.traffic, //
			(HPVT_NETWORK_TRAFFIC_INFO *) &status->socket.traffic, 1);

	g_context->connection.feedback.receive_throughput = HPVT_calculate_recieve_throughout((HPVT_NETWORK_TRAFFIC_INFO *) &previous_callback_status.socket.feedback.basic.traffic, //
			(HPVT_NETWORK_TRAFFIC_INFO *) &status->socket.feedback.basic.traffic, 1);

	g_context->connection.feedback.send_throughput = HPVT_calculate_send_throughout((HPVT_NETWORK_TRAFFIC_INFO *) &previous_callback_status.socket.feedback.basic.traffic, //
			(HPVT_NETWORK_TRAFFIC_INFO *) &status->socket.feedback.basic.traffic, 1);
}
static boolean _HPVT_check_session_timeout(__const SCSSocketCallbackStatus * __restrict status) {

	if (g_context->settings.video.feedback_interval == 0 || g_context->settings.video.feedback_control == false) {
		return false;
	}

	if( g_context->settings.video.timeout_threshold == 0)
	{
		return false;
	}

	if (HPVT_Context_is_session_connected(g_context) == false) {
		return false;
	}
	uint64_t current_packet_count = 0;

	current_packet_count = status->socket.traffic.received.control.packets;

	if (current_packet_count == previous_packet_count) {
		g_context->connection.timeout_count++;

		if (g_context->connection.timeout_count * 1000 > g_context->settings.video.feedback_interval) {
			printf("%2d/%2d\n", g_context->connection.timeout_count, g_context->settings.video.timeout_threshold);
		}

		HPVT_logging(LOG_LEVEL_NOTICE, "Count up (%d) Timeout=%d", g_context->connection.timeout_count, g_context->settings.video.timeout_threshold);

		if (g_context->connection.timeout_count >= g_context->settings.video.timeout_threshold) {
			HPVT_logging(LOG_LEVEL_NOTICE, "Connection timed out (%d)", g_context->connection.timeout_count);
			printf("Timed out.\n");
			previous_packet_count = 0;
			g_context->connection.flag_timeout = true;
		}
		return true;
	}
	else {
		previous_packet_count = current_packet_count;

		if (g_context->connection.timeout_count > 0) {
			HPVT_logging(LOG_LEVEL_NOTICE, "Counter %d to zero", g_context->connection.timeout_count);
			printf("%2d/%2d\n", 0, g_context->settings.video.timeout_threshold);
		}

		g_context->connection.timeout_count = 0;
	}

	return false;

}
uint64_t HPVT_calculate_send_throughout(HPVT_NETWORK_TRAFFIC_INFO *prev, HPVT_NETWORK_TRAFFIC_INFO *curr, int diff_time) {

	uint64_t diff_bytes;
	uint64_t diff_packets;

	diff_bytes = (curr->sent.control.bytes + curr->sent.payload.bytes) //
	- (prev->sent.control.bytes + prev->sent.payload.bytes);

	diff_packets = curr->sent.control.packets - prev->sent.control.packets;

 	diff_bytes += diff_packets * (HPVT_PACKET_IP_UDP_HEADER_LENGTH);

	diff_packets = curr->sent.payload.packets - prev->sent.payload.packets;

 	diff_bytes += diff_packets * (HPVT_PACKET_IP_UDP_HEADER_LENGTH + HPVT_PACKET_SCS_HEADER_LENGTH_MAXIMUM);

	uint64_t throughout;
	throughout = diff_bytes * 8 / diff_time;

	return throughout;
}
uint64_t HPVT_calculate_recieve_throughout(HPVT_NETWORK_TRAFFIC_INFO *prev, HPVT_NETWORK_TRAFFIC_INFO *curr, int diff_time) {

	uint64_t diff_bytes;
	uint64_t diff_packets;

	diff_bytes = (curr->received.control.bytes + curr->received.payload.bytes) //
	- (prev->received.control.bytes + prev->received.payload.bytes);

	diff_packets = curr->received.control.packets - prev->received.control.packets;

	diff_bytes += diff_packets * (HPVT_PACKET_IP_UDP_HEADER_LENGTH);

	diff_packets = curr->received.payload.packets - prev->received.payload.packets;

	diff_bytes += diff_packets * (HPVT_PACKET_IP_UDP_HEADER_LENGTH + HPVT_PACKET_SCS_HEADER_LENGTH_MAXIMUM);

	uint64_t throughout;
	throughout = diff_bytes * 8 / diff_time;

	return throughout;
}
void HPVT_statistics_callback(SCSSocketCallbackConfig * __restrict config, __const SCSSocketCallbackStatus * __restrict status) {

	HPVT_logging(LOG_LEVEL_INFO, "called the socket callback (state=%s)", //
			HPVT_Context_get_connection_state_string(g_context->connection.state));
	g_context->connection.no_callback_count = 0;

	if (config == NULL) {
		HPVT_logging(LOG_LEVEL_EMERGENCY, "!");
		return;
	}

	if (status == NULL) {
		HPVT_logging(LOG_LEVEL_EMERGENCY, "!");
		return;
	}

	_HPVT_calculate_throughout(status);

	if (HPVT_Context_is_transmitter(g_context) == true) {
		HPVT_add_feeedback_data(status->socket.feedback.basic.plr.current, g_context->connection.feedback.receive_throughput, g_context->connection.self.send_throughput);
	}

	if (_HPVT_check_session_timeout(status) == true) {
		return;
	}

	memcpy(&previous_callback_status, status, sizeof(SCSSocketCallbackStatus));

	if (g_traffic_info_self == NULL) {
		g_traffic_info_self = &previous_callback_status.socket.traffic;
	}

	if (g_traffic_info_peer == NULL) {
		g_traffic_info_peer = &previous_callback_status.socket.feedback.basic.traffic;
	}
}
boolean HPVT_SCS_add_socket_callback(SCSSocketId socket_id, SCSCallbackId *callback_id) {

	SCSSocketCallbackConfig tmp_config;

	SCSSocketCallbackConfigInitialize(&tmp_config);
	tmp_config.func = HPVT_statistics_callback;
	tmp_config.argument.ptr = NULL;
	tmp_config.argument.size = 0;
	tmp_config.interval = 1;
	tmp_config.id = socket_id;

	memset(&previous_callback_status, 0, sizeof(SCSSocketCallbackStatus));

	if (SCSAddCallback(SCS_CALLBACKTYPE_SOCKET, &tmp_config, sizeof(tmp_config), callback_id) == false) {
		HPVT_logging(LOG_LEVEL_ALERT, "Failed to add a callback.");
		return false;
	}

	return true;
}
boolean HPVT_SCS_remove_socket_callback(SCSCallbackId callback_id) {


	if(callback_id == SCS_CALLBACKID_INVVAL){
		return true;
	}

	if (SCSRemoveCallback(SCS_CALLBACKTYPE_SOCKET, callback_id) == false) {
		HPVT_logging(LOG_LEVEL_ALERT, "Failed to remove a callback.");
		return false;
	}

	g_traffic_info_self = NULL;
	g_traffic_info_peer = NULL;

	return true;
}
size_t HPVT_custum_callback(SCSFeedbackCallbackConfig * __restrict config, uint8_t * __restrict ptr, size_t size) {

	if (ptr == NULL) {
		return 0;
	}

	if (size < 1) {
		return 0;
	}

	size_t tmp_len = 0;
	if (HPVT_Context_is_transmitter(g_context) == true) {

		tmp_len = 0;
	}
	else {

		tmp_len = 0;
	}

	return tmp_len;
}
boolean HPVT_SCS_set_socket_custom_feedback_callback(SCSSocketId socket_id) {

	SCSFeedbackCallbackConfig tmp_config;
	SCSFeedbackCallbackConfigInitialize(&tmp_config);
	tmp_config.func = HPVT_custum_callback;
	tmp_config.argument.ptr = NULL;
	tmp_config.argument.size = 0;

	if (SCSSetSocketCallback(socket_id, SCS_SKTCBTYPE_FEEDBACK_CALLBACK, &tmp_config, sizeof(tmp_config)) == false) {
		HPVT_logging(LOG_LEVEL_ALERT, "Failed to set a callback.");
		return false;
	}

	return true;
}
