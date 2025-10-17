#define FILE_NUMBER "N113"

#include "../config/config_s.h"
#include "context_s.h"
#include "socket.h"

extern HPVT_Context *g_context;

boolean HPVT_SCS_get_socket_info(SCSSocketId socket_id, char *local_addr, char *remote_addr, int *mtu, uint32_t *connection_id) {

	SCSSocketInfo tmp_socket_info;
	if (SCSGetSocketInfo(socket_id, &tmp_socket_info) == false) {
		HPVT_logging(LOG_LEVEL_ERROR, "socket info error");
		return false;
	}

	SCSSockAddrToString(tmp_socket_info.addr.self, local_addr, 32);
	SCSSockAddrToString(tmp_socket_info.addr.peer, remote_addr, 32);
	*mtu = tmp_socket_info.mtu;
	*connection_id = tmp_socket_info.connid;

	return true;
}
boolean HPVT_SCS_set_transmitter_hasty_data(SCSSocketId socket_id) {

	char tmp_buf[7];
	memset(tmp_buf, 0, sizeof(tmp_buf));

	uint8_t tmp_timeout_threshold;
	uint8_t tmp_feedback_control;
	uint16_t tmp_feedback_interval;

	uint8_t tmp_version_major;
	uint8_t tmp_version_minor;
	uint8_t tmp_version_rivision;

	tmp_timeout_threshold = g_context->settings.video.timeout_threshold;
	tmp_feedback_control = g_context->settings.video.feedback_control;
	tmp_feedback_interval = g_context->settings.video.feedback_interval;

	tmp_version_major = HPVT_VERSION_MAJOR;
	tmp_version_minor = HPVT_VERSION_MINOR;
	tmp_version_rivision =  HPVT_VERSION_REVISION;

	memcpy(tmp_buf, &tmp_timeout_threshold, sizeof(uint8_t));
	memcpy(tmp_buf + 1, &tmp_feedback_control, sizeof(uint8_t));
	memcpy(tmp_buf + 2, &tmp_feedback_interval, sizeof(uint16_t));
	memcpy(tmp_buf + 4, &tmp_version_major, sizeof(uint8_t));
	memcpy(tmp_buf + 5, &tmp_version_minor, sizeof(uint8_t));
	memcpy(tmp_buf + 6, &tmp_version_rivision, sizeof(uint8_t));

	if (SCSSetSocketOption(socket_id, SCS_SKTOPTNAME_HASTYDATA, tmp_buf, sizeof(tmp_buf)) == false) {
		HPVT_logging(LOG_LEVEL_ERROR, "socket option error");
		return false;
	}

	SCSSocketOptionName tmp_name;

	tmp_name = SCS_SKTOPTNAME_PATHMTUD_DISABLE;

	if (SCSSetSocketOption(socket_id, tmp_name, tmp_buf, sizeof(tmp_buf)) == false) {
		HPVT_logging(LOG_LEVEL_ERROR, "socket option error");
		return false;
	}

	HPVT_logging(LOG_LEVEL_NOTICE, "sent Transmitter config Timeout=%d sec, Feedback=%d, Interval=%d,Version=%d.%d.%d",//
			tmp_timeout_threshold, tmp_feedback_control,tmp_feedback_interval,tmp_version_major,tmp_version_minor,tmp_version_rivision);

	return true;

}
boolean HPVT_SCS_get_transmitter_hasty_data(SCSSocketId socket_id) {

	char tmp_buf[7];
	memset(tmp_buf, 0, sizeof(tmp_buf));

	if (SCSGetSocketOption(socket_id, SCS_SKTOPTNAME_HASTYDATA, tmp_buf, sizeof(tmp_buf)) == false) {
		HPVT_logging(LOG_LEVEL_ERROR, "socket option error");
		return false;
	}

	uint8_t tmp_timeout_threshold;
	uint8_t tmp_feedback_control;
	uint16_t tmp_feedback_interval;
	uint8_t tmp_version_major;
	uint8_t tmp_version_minor;
	uint8_t tmp_version_rivision;

	memcpy(&tmp_timeout_threshold, tmp_buf, sizeof(uint8_t));
	memcpy(&tmp_feedback_control, tmp_buf + 1, sizeof(uint8_t));
	memcpy(&tmp_feedback_interval, tmp_buf + 2, sizeof(uint16_t));
	memcpy(&tmp_version_major, tmp_buf + 4, sizeof(uint8_t));
	memcpy(&tmp_version_minor, tmp_buf + 5, sizeof(uint8_t));
	memcpy(&tmp_version_rivision, tmp_buf + 6, sizeof(uint8_t));

	g_context->settings.video.timeout_threshold = tmp_timeout_threshold;
	g_context->settings.video.feedback_control = tmp_feedback_control;
	g_context->settings.video.feedback_interval = tmp_feedback_interval;
	g_context->connection.peer.version_major = tmp_version_major;
	g_context->connection.peer.version_minor = tmp_version_minor;
	g_context->connection.peer.version_rivision = tmp_version_rivision;
	HPVT_logging(LOG_LEVEL_NOTICE, "received Transmitter config Timeout=%d sec, Feedback=%d, Interval=%d,Version=%d.%d.%d",//
			tmp_timeout_threshold, tmp_feedback_control,tmp_feedback_interval,tmp_version_major,tmp_version_minor,tmp_version_rivision);

	return true;
}
void HPVT_SCS_queue_clear(SCSSocketId socket_id) {

	SCSClear(socket_id, SCS_SKTDRCTN_BOTH);
}
