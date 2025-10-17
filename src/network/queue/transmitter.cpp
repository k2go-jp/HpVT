#define FILE_NUMBER "Q202"

#include <list>
#include <algorithm>

#include "../../config/config.h"
#include "../socket.h"
#include "../context.h"
#include "../network.h"
#include "HPVTPacket.h"
#include "HPVTPacketSet.h"
#include "HPVTPacketSetQueue.h"
#include "HPVTVideoFrame.h"
#include "HPVTVideoFrameQueue.h"
#include "HPVTLimitter.h"
#include "queue.h"
#include "scs/5/feature/traffic/controller.h"
#include "scs/6/fec/fec.h"
#include "scs/6/fec/encoder.h"
#include "scs/6/fec/decoder.h"

extern HPVT_Context *g_context;
extern SCSSocketId g_scssock;

static HPVTVideoFrameQueue *serverFrameQueue = NULL;
static HPVTPacketSetQueue *serverPacketSetQueue = NULL;

void HPVT_initialize_transmitter_objects(void) {

	serverFrameQueue = new HPVTVideoFrameQueue();
	serverPacketSetQueue = new HPVTPacketSetQueue();
}
void HPVT_dispose_transmitter_objects(void) {

	delete serverFrameQueue;
	delete serverPacketSetQueue;
}
static HPVT_Queue_FRAME_SEQNO _HPVT_generate_frame_seqno(void) {

	static unsigned long long counter = 0;

	counter++;

	if (counter > HPVT_Queue_FRAME_SEQNO_MAXIMUM) {
		counter = HPVT_Queue_FRAME_SEQNO_MINIMUM;
	}

	return (HPVT_Queue_FRAME_SEQNO) counter;

}
static void HPVT_get_real_time_millisecond(uint64_t *result) {

	struct timespec tmp_ts;
	clock_gettime(CLOCK_REALTIME, &tmp_ts);
	*result = (uint64_t) tmp_ts.tv_sec * 1000 + (uint64_t) tmp_ts.tv_nsec / 1000000;
}
void HPVT_enqueue_counter_up(void) {
	HPVT_Queue_FRAME_SEQNO tmp_seqno = _HPVT_generate_frame_seqno();
	HPVT_logging(LOG_LEVEL_NOTICE, "ENC VFrame skipped seq=%d", tmp_seqno);

}
int HPVT_enqueue_video_frame_encoded(void* buffer, int length, boolean is_keyframe, boolean is_codec_hevc) {

	if (length == 0) {
		return 0;
	}

	if (g_context->settings.video.encode_framerate == 0) {
		return 0;
	}

	static unsigned char tmp_codec_info_header[64];
	static int tmp_codec_info_header_len = 0;
	static unsigned char tmp_codec_buf[64];
	static int tmp_codec_len = 0;
	static boolean skipped_frame_enqueued = false;
	static unsigned long long number = 0;
	static unsigned char tmp_partial_frame[HPVT_FRAME_LENGTH_MAXIMUM];
	static int tmp_partial_frame_len = 0;

	unsigned int tmp_flags;
	HPVT_Queue_FRAME_TYPE tmp_frame_type;

	if (is_keyframe == 1) {
		tmp_frame_type = HPVT_Queue_FRAME_TYPE_I;
	}
	else {
		tmp_frame_type = HPVT_Queue_FRAME_TYPE_P;
	}

	uint32_t tmp_current_time;
	tmp_current_time = HPVT_get_current_time();
	HPVTVideoFrame *tmp_frame;

	tmp_frame = new HPVTVideoFrame(tmp_codec_info_header, tmp_codec_info_header_len, (unsigned char *)buffer, length);

	tmp_frame->setFrameType(tmp_frame_type);
	tmp_frame->setFrameSeqno(_HPVT_generate_frame_seqno());

	tmp_frame->setWidth(g_context->settings.video.camera.width / 8);
	tmp_frame->setHeight(g_context->settings.video.camera.height / 8);

	tmp_frame->setFps(g_context->settings.video.encode_framerate);
	tmp_frame->setIdrPeriod(g_context->settings.video.i_frame_interval);
	tmp_frame->setFecLevel(g_context->settings.video.fec_level);
	tmp_frame->setBitrate(g_context->settings.video.bitrate);
	tmp_frame->setGeneratedTime(tmp_current_time);
	if (is_codec_hevc) {
		tmp_frame->setH265Flag(1);
	}

	g_context->connection.processing_seqno = tmp_frame->getFrameSeqno();
	HPVT_get_real_time_millisecond(&g_context->connection.frame_generated_time);

	number++;

	LOG_LEVEL tmp_log_level;
	if ((tmp_frame->getFrameSeqno() & 0xff) == 0x01) {
		tmp_log_level = LOG_LEVEL_NOTICE;
	}
	else {
		tmp_log_level = LOG_LEVEL_DEBUG;
	}

	HPVT_logging(tmp_log_level, "ENC VFrame N=%llu,seq=%d,type=%d,len=%lu,time=%lu, head=%d,W=%d,H=%d,F=%d,I=%d,B=%d,FEC=%d", //
			number, tmp_frame->getFrameSeqno(), tmp_frame->getFrameType(), tmp_frame->getLength(), tmp_frame->getGeneratedTime(), //
			tmp_codec_info_header_len, tmp_frame->getWidth() * 8, tmp_frame->getHeight() * 8, g_context->settings.video.encode_framerate, //
			g_context->settings.video.i_frame_interval,	g_context->settings.video.bitrate / 1000, g_context->settings.video.fec_level);

	if (tmp_frame_type == HPVT_Queue_FRAME_TYPE_P) {

		if (skipped_frame_enqueued == true) {
			delete tmp_frame;
			HPVT_logging(LOG_LEVEL_NOTICE, "ENC VFrame seq=%d, P Frame skipped", tmp_frame->getFrameSeqno());
			return 0;
		}
	}

	if (serverFrameQueue->enqueue(tmp_frame) != 0) {

		HPVT_logging(tmp_log_level, "cannot enqueue VFrame(seq=%d)", tmp_frame->getFrameSeqno());
		delete tmp_frame;
		skipped_frame_enqueued = true;
		serverFrameQueue->cut(2);
		return -1;
	}
	else {
		skipped_frame_enqueued = false;
		HPVT_logging(LOG_LEVEL_DEBUG, "enqueued VFrame (seq=%d)", tmp_frame->getFrameSeqno());
	}

	return 0;
}
static void _HPVT_discover_path_MTU(HPVTPacketSet *packetSet, int fps) {

	static uint32_t received_frame_count = 0;

	received_frame_count++;

	switch (g_context->settings.video.packet_size_determination_mode) {

		case HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_STANDARD: //
		{
			packetSet->setPacketPaddingEnabled(false);
			packetSet->setPathMtu(g_context->settings.video.mtu_size);
			break;
		}
		case HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_FIXED: //
		{
			packetSet->setPacketPaddingEnabled(true);
			packetSet->setPathMtu(g_context->settings.video.mtu_size);
			break;
		}
		case HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_AUTO_DISCOVERY: //
		{
			if (g_context->connection.path_mtu_discovery.flag_start_discovery == true) {

				int diff_size;

				diff_size = (g_context->connection.path_mtu_discovery.try_maximum_size - g_context->connection.path_mtu_discovery.receivable_size);

				if (diff_size > 0) {

					int tmp_remainder;

					tmp_remainder = received_frame_count % (fps * 5);

					if (tmp_remainder == 0) {
						packetSet->setMtuDiscoveryEnabled(true);
						packetSet->setTrialMtu(g_context->connection.path_mtu_discovery.receivable_size);
					}
					else if (tmp_remainder == 2) {
						int trial_size;
						if (diff_size < 4) {
							trial_size = g_context->connection.path_mtu_discovery.receivable_size + diff_size;
						}
						else {
							trial_size = g_context->connection.path_mtu_discovery.receivable_size + 4;
						}

						packetSet->setMtuDiscoveryEnabled(true);
						packetSet->setTrialMtu(trial_size);

					}
					else if (tmp_remainder == 3) {

						int trial_size;
						trial_size = g_context->connection.path_mtu_discovery.receivable_size + (diff_size / 8);
						packetSet->setMtuDiscoveryEnabled(true);
						packetSet->setTrialMtu(trial_size);

					}
					else if (tmp_remainder == 4) {

						int trial_size;
						trial_size = g_context->connection.path_mtu_discovery.receivable_size + (diff_size / 2);
						packetSet->setMtuDiscoveryEnabled(true);
						packetSet->setTrialMtu(trial_size);
					}

				}

			}

			packetSet->setPathMtu(g_context->connection.path_mtu_discovery.receivable_size);

			break;
		}
		default:
		{
			HPVT_logging(LOG_LEVEL_ERROR, "Invalid parameter %d", g_context->settings.video.packet_size_determination_mode);
		}

	}

}
void HPVT_start_thread_packetizer(HPVT_Config *config) {

	HPVTVideoFrame *tmp_frame;
	HPVTPacketSet *tmp_packetSet;

	while (true) {

		if (serverFrameQueue == NULL) {
			sleep(1);
			continue;
		}

		if (serverFrameQueue->isEmpty()) {
			usleep(1000);
			continue;
		}

		tmp_frame = serverFrameQueue->dequeue();

		if (tmp_frame == NULL) {
			HPVT_logging(LOG_LEVEL_WARNING, "VFrame dequeue error");
			continue;
		}

		HPVT_logging(LOG_LEVEL_DEBUG, "dequeued VFrame (seq=%d)", tmp_frame->getFrameSeqno());

		//

		tmp_packetSet = new HPVTPacketSet();

		if (g_context->settings.system.connection_mode == HPVT_Config_CONNECTION_MODE_CONNECTION_LESS) {
			tmp_packetSet->setScsEnabled(false);
		}

		if (g_context->settings.video.fec_enabled == false) {
			tmp_packetSet->setFecEnabled(false);
		}

		_HPVT_discover_path_MTU(tmp_packetSet, tmp_frame->getFps());

		if (tmp_packetSet->packetize(tmp_frame) == false) {
			delete tmp_packetSet;
			delete tmp_frame;
			continue;
		}

		delete tmp_frame;

		HPVT_logging(LOG_LEVEL_DEBUG, "packetized VFrame (seq=%d)", tmp_packetSet->getFrameSeqno());

		if (serverPacketSetQueue->enqueue(tmp_packetSet) != 0) {
			delete tmp_packetSet;
		}
	}
}
static boolean _HPVT_send_pacing(HPVTPacketSet *packet_set, boolean pace_control_enable, SCSVideoStreamTrafficController *tc) {

	if (packet_set == NULL) {
		HPVT_logging(LOG_LEVEL_ALERT, "parameter error!");
		return false;
	}

	if (packet_set->getFps() == 0) {
		HPVT_logging(LOG_LEVEL_ALERT, "Invalid fps value!");
		return false;
	}

	if (packet_set->getBitrate() == 0) {
		HPVT_logging(LOG_LEVEL_ALERT, "Invalid bitrate value!");
		return false;
	}

	if (packet_set->getCount() == 0) {
		HPVT_logging(LOG_LEVEL_ALERT, "Invalid packet count value!");
		return false;
	}

	FECGroupId tmp_gid;
	FECEncodeLevel tmp_level;
	FECData tmp_outputs[1024];
	size_t tmp_outputs_count = 1024;
	FECEncoder tmp_encoder;
	FECData * tmp_ptr;
	size_t tmp_result_count;
	boolean tmp_flag_encoded_done = false;
	HPVTPacket *tmp_packet;
	boolean ret_send = true;
	int sent_length;
	int i;

	if (packet_set->getFecLevel() != 0) {

		tmp_gid = packet_set->getFrameSeqno();
		tmp_level = packet_set->getFecLevel();

		FECEncoderInitialize(&tmp_encoder);

		if (FECEncoderStandBy(&tmp_encoder, 1024)) {
			FECEncoderSetGroupId(&tmp_encoder, tmp_gid);
			FECEncoderSetLevel(&tmp_encoder, tmp_level);
		}
		else {
			HPVT_log_UNEXPECTED_ERROR();
		}

		for (i = 0; i < packet_set->getCount(); i++) {

			tmp_packet = packet_set->getPacketData(i);

			if (tmp_packet == NULL) {
				HPVT_logging(LOG_LEVEL_ALERT, "packet error!");
				continue;
			}

			if (FECEncoderAdd(&tmp_encoder, (uint8_t *) tmp_packet->getBuffer(), tmp_packet->getBufferLength()) == false) {
				HPVT_log_UNEXPECTED_ERROR();
			}

		}

		tmp_ptr = tmp_outputs;
		tmp_result_count = tmp_outputs_count;
		memset(&tmp_outputs, 0, sizeof(tmp_outputs));

		if (FECEncoderEncode(&tmp_encoder, tmp_ptr, &tmp_result_count) == true) {
			tmp_flag_encoded_done = true;
		}
		else {
			tmp_flag_encoded_done = false;
		}

		FECEncoderCleanUp(&tmp_encoder);
		FECEncoderFinalize(&tmp_encoder);
	}

	if (tmp_flag_encoded_done == false) {

		for (i = 0; i < packet_set->getCount(); i++) {

			tmp_packet = packet_set->getPacketData(i);

			if (tmp_packet == NULL) {
				HPVT_logging(LOG_LEVEL_ALERT, "packet error!");
				continue;
			}
			tmp_outputs[i].ptr = tmp_packet->getBuffer();
			tmp_outputs[i].size = tmp_packet->getBufferLength();
		}
		tmp_result_count = packet_set->getCount();
	}

	uint64_t tmp_total_data_byte = 0;
	scs_bit_rate tmp_calculated_bitrate;
	scs_bit_rate tmp_threshold_bitrate;
	scs_frame_rate tmp_fps;
	int additional_rate = 3; // %
	scs_bit_rate tmp_necessary_bitrate;

	tmp_calculated_bitrate = packet_set->getBitrate() * (100 + additional_rate) / 100;
	tmp_fps = packet_set->getFps();
	tmp_threshold_bitrate = tmp_calculated_bitrate;

	for (int i = 0; i < tmp_result_count; i++) {
		tmp_total_data_byte += tmp_outputs[i].size;
	}

	tmp_necessary_bitrate = (tmp_total_data_byte * 8) * tmp_fps;

	if (tmp_calculated_bitrate < tmp_necessary_bitrate) {
		tmp_calculated_bitrate = tmp_necessary_bitrate;
	}

	static unsigned long long tmp_total_size_one_second = 0;
	static unsigned long long tmp_base_number = 0;

	tmp_base_number++;
	tmp_total_size_one_second += tmp_total_data_byte;

	if (tmp_base_number == packet_set->getFps()) {
		tmp_base_number = 0;
		tmp_total_size_one_second = 0;
	}

	if (pace_control_enable == true) {
		SCSVideoStreamTrafficControllerSetBitRate(tc, tmp_calculated_bitrate);
		SCSVideoStreamTrafficControllerSetFrameRate(tc, tmp_fps);
	}

	uint32_t tmp_desired_time_msec;
	tmp_desired_time_msec = packet_set->getIdrPeriod() * 850 / tmp_fps;

	HPVTLimitter *tmp_limitter = new HPVTLimitter(tmp_total_data_byte, tmp_desired_time_msec);

	for (int i = 0; i < tmp_result_count; i++) {

		if (pace_control_enable == true) {
			scs_timespec tmp_delaytime;
			SCSTimespecInitialize(tmp_delaytime);

			SCS_TCRETVAL tmp_result;
			tmp_result = SCSVideoStreamTrafficControllerUpdate(tc, tmp_outputs[i].size, &tmp_delaytime);

			switch (tmp_result) {
				case SCS_TCRETVAL_OK: {
					break;
				}
				case SCS_TCRETVAL_OVER: {
					SCSVideoStreamTrafficControllerWait(&tmp_tc, tmp_delaytime);
					break;
				}
				case SCS_TCRETVAL_NG: {
					HPVT_logging(LOG_LEVEL_ERROR, "pacing wait Error!");
					break;
				}
			}
		}

		sent_length = HPVT_send(g_scssock, (unsigned char *) tmp_outputs[i].ptr, tmp_outputs[i].size);

		if (tmp_flag_encoded_done == true) {
			free(tmp_outputs[i].ptr);
		}

		if (sent_length == -1) {
			HPVT_logging(LOG_LEVEL_WARNING, "send error!");
			ret_send = false;
			break;
		}
	}

	delete tmp_limitter;
	packet_set->removePackets();

	return ret_send;
}
void HPVT_start_thread_packet_transmitter(HPVT_Config *config) {

	HPVTPacketSet *tmp_packet_set;

	SCSVideoStreamTrafficController tmp_traffic_controller;
	SCSVideoStreamTrafficControllerInitialize(&tmp_traffic_controller);

	label_START: ;

	HPVT_Context_deactivate(g_context);

	if (HPVT_Config_is_server(config->connection.type, config->connection.connection_mode) == true) {

		if (config->connection.connection_mode == HPVT_Config_CONNECTION_MODE_CONNECTION_LESS) {
			HPVT_UDP_setup_server_socket(config->connection.listen_port);
		}
		else {
			if (HPVT_SCS_setup_server_socket(config->connection.listen_port) == false) {
				HPVT_logging(LOG_LEVEL_ERROR, "Failed to setup socket");
				goto label_START;
			}
		}
	}
	else {

		if (config->connection.host_ip == 0) {
			printf("Warning! 'ConnectionIP' is not set.\n");
			printf("This process will never attempt a connection with Receiver.\n");
			HPVT_Context_activate(g_context);
			return;
		}

		if (config->connection.connection_mode == HPVT_Config_CONNECTION_MODE_CONNECTION_LESS) {
			HPVT_UDP_setup_client_socket(config->connection.host_ip, config->connection.host_port);
		}
		else {
			if (HPVT_SCS_setup_client_socket(config->connection.host_ip, config->connection.host_port) == false) {
				HPVT_logging(LOG_LEVEL_ERROR, "Failed to setup socket");
				goto label_START;
			}
		}
	}

	HPVT_Context_activate(g_context);

	boolean ret_send = true;

	while (true) {

		if (ret_send == false || g_context->connection.reconnection_attempt_on == false) {

			if (g_context->settings.video.pace_control == HPVT_Config_FUNCTION_ON) {
				SCSVideoStreamTrafficControllerFinalize(&tmp_traffic_controller);
				SCSVideoStreamTrafficControllerInitialize(&tmp_traffic_controller);
			}

			serverPacketSetQueue->clear();
			if (HPVT_Context_is_session_connected(g_context) == true) {
				if (g_context->connection.flag_timeout == false) {
					HPVT_SCS_reset_connection();
				}
			}

			if (g_context->connection.reconnection_attempt_on == true) {
				HPVT_logging(LOG_LEVEL_NOTICE, "Retry wait %d sec", g_context->settings.video.timeout_retry_wait);
				sleep(g_context->settings.video.timeout_retry_wait);
			}

			goto label_START;
		}

		if (serverPacketSetQueue->isEmpty()) {
			usleep(1000);
			continue;
		}

		if (serverPacketSetQueue->isOverLimitIFrames() == true) {

			serverPacketSetQueue->cutOldFrames();
			continue;
		}

		tmp_packet_set = serverPacketSetQueue->dequeue();

		if (tmp_packet_set == NULL) {
			HPVT_logging(LOG_LEVEL_WARNING, "packetSet dequeue error!");
			continue;
		}

		if (serverPacketSetQueue->isOverMaxFrames(tmp_packet_set->getFps()) == true) {
			serverPacketSetQueue->cutPFrames(2 * tmp_packet_set->getFps());
		}

		if (g_context->connection.flag_send_wait == true) {
			HPVT_logging(LOG_LEVEL_NOTICE, "skip sending packetSet seqno=%d", tmp_packet_set->getFrameSeqno());
			delete tmp_packet_set;
			continue;
		}

		ret_send = _HPVT_send_pacing(tmp_packet_set, g_context->settings.video.pace_control, &tmp_traffic_controller);

		if (g_context->settings.video.pace_control == true) {

			scs_timespec tmp_delaytime;
			SCSTimespecInitialize(tmp_delaytime);

			SCS_TCRETVAL tmp_result;
			tmp_result = SCSVideoStreamTrafficControllerNextFrame(&tmp_traffic_controller, &tmp_delaytime);

			switch (tmp_result) {
				case SCS_TCRETVAL_OK: {
					break;
				}
				case SCS_TCRETVAL_OVER: {
					SCSVideoStreamTrafficControllerWait(&tmp_traffic_controller, tmp_delaytime);
					break;
				}
				case SCS_TCRETVAL_NG: {
					HPVT_logging(LOG_LEVEL_ERROR, "pacing wait Error!");
					break;
				}
			}

		}

		if (tmp_packet_set->isMtuDiscoveryEnabled() == true) {
			ret_send = true;
		}

		g_context->connection.send_recv_seqno = tmp_packet_set->getFrameSeqno();
		delete tmp_packet_set;
	}

	SCSVideoStreamTrafficControllerFinalize(&tmp_traffic_controller);
}
static boolean _HPVT_add_frame_generated_time(unsigned char *buffer, int *remaining_length, int *write_length, //
		HPVT_Queue_FRAME_SEQNO seqno, uint64_t generated_time) {

	char tmp_key_value_pair[32];
	char *ptr;
	uint8_t tmp_code_size = 1;
	uint8_t tmp_value_size;
	uint8_t tmp_total_length;

	memset(tmp_key_value_pair, 0, sizeof(tmp_key_value_pair));
	ptr = tmp_key_value_pair;

	*ptr = HPVT_TELEMETRY_KEY_CODE_FRAME_GENERATED_TIME;
	ptr += 1;

	tmp_value_size = sizeof(seqno) + sizeof(generated_time);
	memcpy(ptr, &tmp_value_size, sizeof(tmp_value_size));
	ptr += sizeof(tmp_value_size);

	memcpy(ptr, &seqno, sizeof(seqno));
	ptr += sizeof(seqno);

	memcpy(ptr, &generated_time, sizeof(generated_time));
	ptr += sizeof(generated_time);

	tmp_total_length = tmp_code_size + sizeof(tmp_value_size) + tmp_value_size;

	if (tmp_total_length > *remaining_length) {
		return false;
	}

	memcpy(buffer + *write_length, tmp_key_value_pair, tmp_total_length);

	*write_length += tmp_total_length;
	*remaining_length -= tmp_total_length;

	return true;
}
static boolean _HPVT_add_request_change_parameters(unsigned char *buffer, //
		int *remaining_length, int *write_length, uint8_t function_flags) {

	char tmp_key_value_pair[32];
	char *ptr;
	uint8_t tmp_code_size = 1;
	uint8_t tmp_value_size;
	uint8_t tmp_total_length;

	memset(tmp_key_value_pair, 0, sizeof(tmp_key_value_pair));
	ptr = tmp_key_value_pair;

	*ptr = HPVT_TELEMETRY_KEY_CODE_FUNCTION_FLAGS;
	ptr += 1;

	tmp_value_size = sizeof(function_flags);
	memcpy(ptr, &tmp_value_size, sizeof(tmp_value_size));
	ptr += sizeof(tmp_value_size);

	memcpy(ptr, &function_flags, sizeof(function_flags));
	ptr += sizeof(function_flags);

	tmp_total_length = tmp_code_size + sizeof(tmp_value_size) + tmp_value_size;

	if (tmp_total_length > *remaining_length) {
		return false;
	}

	memcpy(buffer + *write_length, tmp_key_value_pair, tmp_total_length);

	*write_length += tmp_total_length;
	*remaining_length -= tmp_total_length;

	return true;
}
static boolean _HPVT_add_adaptive_control_status(unsigned char *buffer, //
		int *remaining_length, int *write_length, uint16_t bitrate) {

	char tmp_key_value_pair[32];
	char *ptr;
	uint8_t tmp_code_size = 1;
	uint8_t tmp_value_size;
	uint8_t tmp_total_length;

	memset(tmp_key_value_pair, 0, sizeof(tmp_key_value_pair));
	ptr = tmp_key_value_pair;

	*ptr = HPVT_TELEMETRY_KEY_CODE_ADAPTIVE_CONTROL_STATUS;
	ptr += 1;

	tmp_value_size = sizeof(bitrate);
	memcpy(ptr, &tmp_value_size, sizeof(tmp_value_size));
	ptr += sizeof(tmp_value_size);

	memcpy(ptr, &bitrate, sizeof(bitrate));
	ptr += sizeof(bitrate);

	tmp_total_length = tmp_code_size + sizeof(tmp_value_size) + tmp_value_size;

	if (tmp_total_length > *remaining_length) {
		return false;
	}

	memcpy(buffer + *write_length, tmp_key_value_pair, tmp_total_length);

	*write_length += tmp_total_length;
	*remaining_length -= tmp_total_length;

	return true;
}
static boolean _HPVT_make_internal_info_buffer(unsigned char *buffer, int length, int *write_length) {

	buffer[0] = HPVT_PACKET_TYPE_INTERNAL_INFO;
	*write_length = 1;

	HPVT_Queue_FRAME_SEQNO tmp_seqno;
	uint64_t tmp_frame_generated_time;
	uint8_t tmp_function_flags;
	uint16_t tmp_adc_max_bitrate;

	tmp_seqno = g_context->connection.processing_seqno;
	tmp_frame_generated_time = g_context->connection.frame_generated_time;
	tmp_adc_max_bitrate = g_context->settings.video.adaptive_control_param.max_bitrate / 1000;

	tmp_function_flags = 0x0;

	if (g_context->settings.video.adaptive_control == true) {
		tmp_function_flags |= HPVT_TELEMETRY_FUNCTION_FLAG_VIDEO_ADAPTIVE_ON;
	}
	else {
		tmp_function_flags &= (~HPVT_TELEMETRY_FUNCTION_FLAG_VIDEO_ADAPTIVE_ON);
	}

	if (tmp_seqno == HPVT_Queue_FRAME_SEQNO_INVALID) {
		return false;
	}

	if (_HPVT_add_frame_generated_time(buffer, &length, write_length, tmp_seqno, tmp_frame_generated_time) == false) {
		return false;
	}

	if (_HPVT_add_request_change_parameters(buffer, &length, write_length, tmp_function_flags) == false) {
		return false;
	}

	if (g_context->settings.video.adaptive_control == true) {
		if (_HPVT_add_adaptive_control_status(buffer, &length, write_length, tmp_adc_max_bitrate) == false) {
			return false;
		}
	}

	HPVT_logging(LOG_LEVEL_NOTICE, "send INTERNAL INFO length=%d,frame(seqno=%d,generated_time=%llu)," //
			"adc=(enabled=%d, bitrate=%d),flags=%02x", *write_length, tmp_seqno, tmp_frame_generated_time, //
			g_context->settings.video.adaptive_control, tmp_adc_max_bitrate, tmp_function_flags);

	return true;
}
void HPVT_start_thread_internal_info_sender(void) {

	HPVT_Context_ready_wait(g_context);

	unsigned char tmp_buffer[512];
	int packet_length;

	int count = 0;
	while (true) {

		sleep(3);

		if (g_context->connection.state != HPVT_VIDEO_CONNECTION_STATE_CONNECTED) {
			continue;
		}

		memset(tmp_buffer, 0, sizeof(tmp_buffer));
		packet_length = 0;

		if (_HPVT_make_internal_info_buffer(tmp_buffer, sizeof(tmp_buffer), &packet_length) == true) {

			if (HPVT_send(g_scssock, tmp_buffer, packet_length) == -1) {
				HPVT_logging(LOG_LEVEL_WARNING, "failed to send!");
			}
		}
	}
}
