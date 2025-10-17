#define FILE_NUMBER "M003"

#include "config/config.h"
#include "network/queue/queue.h"
#include "network/network.h"

extern HPVT_Context *g_context;

static void *thread_encode_routine(void *vp) {

	HPVT_Config *tmp_config;
	tmp_config = (HPVT_Config *) vp;

	if (tmp_config == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	HPVT_start_thread_h264_video_encoding(tmp_config);

	return NULL;
}
static void *thread_packetize_routine(void *vp) {

	HPVT_Config *tmp_config;
	tmp_config = (HPVT_Config *) vp;

	if (tmp_config == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	HPVT_start_thread_packetizer(tmp_config);

	return NULL;
}
static void *thread_transmit_routine(void *vp) {

	HPVT_Config *tmp_config;
	tmp_config = (HPVT_Config *) vp;

	if (tmp_config == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}
	HPVT_start_thread_packet_transmitter(tmp_config);

	return NULL;
}
static void *thread_send_telemetry_routine(void *vp) {

	HPVT_Config *tmp_config;
	tmp_config = (HPVT_Config *) vp;

	if (tmp_config == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	HPVT_start_thread_internal_info_sender();

	return NULL;
}
static void *thread_adaptive_control_routine(void *vp) {

	HPVT_Config *tmp_config;
	tmp_config = (HPVT_Config *) vp;

	if (tmp_config == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	HPVT_start_thread_adaptive_controller();

	return NULL;
}
static void *thread_receive_notification_routine(void *vp) {

	HPVT_Config *tmp_config;
	tmp_config = (HPVT_Config *) vp;

	if (tmp_config == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	HPVT_start_thread_notification_receiver(tmp_config);

	return NULL;
}
int HPVT_main_server(HPVT_Config *config) {

	printf("-------------------------------------------------------\n");
	printf("Started Server (Video Transmitter)\n");
	syslog(LOG_NOTICE, "Started video transmitter");

	//

	HPVT_initialize_transmitter_objects();
	pthread_attr_t tmp_attribute;
	pthread_t tmp_id[6];

	if (pthread_attr_init(&tmp_attribute) != 0) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (pthread_attr_setdetachstate(&tmp_attribute, PTHREAD_CREATE_DETACHED) != 0) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (pthread_create(&tmp_id[0], &tmp_attribute, thread_packetize_routine, config) != 0) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (pthread_create(&tmp_id[1], &tmp_attribute, thread_transmit_routine, config) != 0) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (pthread_create(&tmp_id[2], &tmp_attribute, thread_encode_routine, config) != 0) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (pthread_create(&tmp_id[3], &tmp_attribute, thread_receive_notification_routine, config) != 0) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (pthread_create(&tmp_id[4], &tmp_attribute, thread_send_telemetry_routine, config) != 0) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (pthread_create(&tmp_id[5], &tmp_attribute, thread_adaptive_control_routine, config) != 0) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	pthread_attr_destroy(&tmp_attribute);

	while (g_context->termination == false) {
		sleep(1);
	}

	return 0;
}
