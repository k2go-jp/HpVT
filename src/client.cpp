#define FILE_NUMBER "M002"

#include "config/config.h"
#include "network/queue/queue.h"
#include "network/network.h"

extern HPVT_Context *g_context;

static void *thread_decode_routine(void *vp) {

	HPVT_Config *tmp_config;
	tmp_config = (HPVT_Config *) vp;

	if (tmp_config == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	HPVT_start_thread_h264_video_decoding(tmp_config);

	return NULL;
}
static void *thread_process_received_packet_routine(void *vp) {

	HPVT_Config *tmp_config;
	tmp_config = (HPVT_Config *) vp;

	if (tmp_config == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	HPVT_start_thread_received_packet_list(tmp_config);

	return NULL;
}
static void *thread_receieve_routine(void *vp) {

	HPVT_Config *tmp_config;
	tmp_config = (HPVT_Config *) vp;

	if (tmp_config == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	HPVT_start_thread_packet_receiver(tmp_config);

	return NULL;
}
int HPVT_main_client(HPVT_Config *config) {

	printf("-------------------------------------------------------\n");
	printf("Started Client (Video Receiver)\n");

	syslog(LOG_NOTICE, "Started video receiver");

	HPVT_initialize_receiver_objects();

	pthread_attr_t tmp_attribute;
	pthread_t tmp_id[3];

	if (pthread_attr_init(&tmp_attribute) != 0) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (pthread_attr_setdetachstate(&tmp_attribute, PTHREAD_CREATE_DETACHED) != 0) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (pthread_create(&tmp_id[0], &tmp_attribute, thread_decode_routine, config) != 0) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (pthread_create(&tmp_id[1], &tmp_attribute, thread_process_received_packet_routine, config) != 0) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (pthread_create(&tmp_id[2], &tmp_attribute, thread_receieve_routine, config) != 0) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	pthread_attr_destroy(&tmp_attribute);

	while (g_context->termination == false) {
		sleep(1);
	}

	return 0;
}
