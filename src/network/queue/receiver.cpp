#define FILE_NUMBER "Q201"

#include <vector>
#include <queue>
#include <algorithm>

#include "../../config/config.h"
#include "../socket.h"
#include "../context.h"
#include "../network.h"
#include "HPVTPacket.h"
#include "HPVTPacketSet.h"
#include "HPVTVideoFrame.h"
#include "HPVTVideoFrameBuilder.h"
#include "HPVTVideoFrameSequenceManager.h"
#include "HPVTReceivePacketManager.h"
#include "HPVTDecodeTimeManager.h"
#include "queue.h"
#include "scs/6/fec/fec.h"
#include "scs/6/fec/decoder.h"

#define BUFFERIING_MINIMUM_FRAMES       2
#define RESET_THRESHOLD_OLD_FRAMES     30
#define RESET_THRESHOLD_ZERO_FRAMES  1000

extern SCSSocketId g_scssock;
extern SCSCallbackId g_callback_id;
extern HPVT_Context *g_context;

static HPVTVideoFrameBuilder *builder;
static HPVTVideoFrameSequenceManager *sequenceManager;
static HPVTReceivePacketManager *receivePacketManager;
static HPVTDecodeTimeManager *decodeTimeManager;
static uint32_t zero_count = 0;

void HPVT_UDP_setup_client_socket(uint32_t addr, uint16_t port) {

	if ((g_scssock = HPVT_UDP_create_client_socket(addr, port)) == -1) {
		HPVT_log_UNEXPECTED_ERROR();
	}
	HPVT_change_connection_state(HPVT_VIDEO_CONNECTION_STATE_CONNECTED);
	HPVT_logging(LOG_LEVEL_NOTICE, "Opened session");
}
boolean HPVT_SCS_setup_client_socket(uint32_t addr, uint16_t port) {


	HPVT_logging(LOG_LEVEL_NOTICE, "setup client socket");

	if ((g_scssock = HPVT_SCS_create_client_socket(addr, port)) == SCS_SKTID_INVVAL) {
		HPVT_logging(LOG_LEVEL_ERROR, "SCS_create client socket error!");
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
void _HPVT_reset_state(void) {

	if (HPVT_Context_is_session_connected(g_context) == false) {
		return;
	}
	HPVT_SCS_reset_connection();

	g_context->connection.send_recv_seqno = HPVT_Queue_PACKET_SEQNO_INVALID;
	receivePacketManager->clear();
	sequenceManager->reset();
	decodeTimeManager->reset();
}
void HPVT_initialize_receiver_objects(void) {

	builder = new HPVTVideoFrameBuilder();
	sequenceManager = new HPVTVideoFrameSequenceManager();
	receivePacketManager = new HPVTReceivePacketManager();
	decodeTimeManager = new HPVTDecodeTimeManager();
}
void HPVT_dispose_receiver_objects(void) {

	delete builder;
	delete sequenceManager;
	delete receivePacketManager;
	delete decodeTimeManager;
}
void HPVT_start_thread_packet_receiver(HPVT_Config *config) {

	HPVT_update_buffering_delay_time();

	label_START: ;
	HPVT_Context_deactivate(g_context);

	if (HPVT_Config_is_server(config->connection.type, config->connection.connection_mode) == true) {

		if (config->connection.connection_mode == HPVT_Config_CONNECTION_MODE_CONNECTION_LESS) {
			HPVT_UDP_setup_server_socket(config->connection.listen_port);
		}
		else {

			if (HPVT_SCS_setup_server_socket(config->connection.listen_port) == false) {
				HPVT_logging(LOG_LEVEL_ERROR, "Failed to setup socket (count=%d)");
				goto label_START;
			}
		}
	}
	else {
		if (config->connection.host_ip == 0) {
			printf("Warning! 'ConnectionIP' is not set.\n");
			printf("This process will never attempt a connection with Transmitter.\n");
			return;
		}

		if (config->connection.connection_mode == HPVT_Config_CONNECTION_MODE_CONNECTION_LESS) {
			HPVT_UDP_setup_client_socket(config->connection.host_ip, config->connection.host_port);
		}
		else {

			if (HPVT_SCS_setup_client_socket(config->connection.host_ip, config->connection.host_port) == false) {
				HPVT_logging(LOG_LEVEL_ERROR, "Failed to setup socket (count=%d)");
				goto label_START;
			}

		}
	}

	HPVT_Context_activate(g_context);

	HPVTPacket *tmp_packet;
	int tmp_received_length;
	int difference;

	uint32_t old_frame_count = 0;
	HPVT_Queue_FRAME_SEQNO tmp_old_frame_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;
	boolean tmp_is_fec = false;
	boolean tmp_is_parity = false;
	FECGroupId tmp_gid;

	while (true) {

		tmp_packet = new HPVTPacket();

		tmp_received_length = HPVT_recv(g_scssock, tmp_packet->getBuffer(), HPVT_PACKET_MTU_MAXIMUM);

		if (tmp_received_length == -1) {
			delete tmp_packet;

			_HPVT_reset_state();
			HPVT_Context_deactivate(g_context);
			HPVT_logging(LOG_LEVEL_WARNING, "failed to receive!");
			goto label_START;
		}
		else if (tmp_received_length == -2) {
			HPVT_logging(LOG_LEVEL_ERROR, "source IP/Port is different!");
			continue;
		}

		tmp_packet->setBufferLength(tmp_received_length);
		tmp_packet->setReceivedTime(HPVT_get_current_time());

		HPVT_Queue_FRAME_SEQNO received_seqno;
		tmp_is_fec = false;
		tmp_is_parity = false;

		if (FECIsFEC(tmp_packet->getBuffer(), tmp_received_length, &tmp_gid) == true) {

			tmp_is_fec = true;

			if (FECIsParity((FECHeader *) tmp_packet->getBuffer()) == true) {
				tmp_is_parity = true;
			}
			else {
				tmp_packet->setStartPosition(FEC_HEADER_SIZE);
			}

			received_seqno = tmp_gid;
		}
		else {

			switch (tmp_packet->getBuffer()[0]) {
				case HPVT_PACKET_TYPE_NORMAL:
					break;
				case HPVT_PACKET_TYPE_INTERNAL_INFO:
					HPVT_parse_internal_info_packet(tmp_packet->getBuffer(), tmp_received_length);
					delete tmp_packet;
					continue;
				default:
					HPVT_logging(LOG_LEVEL_ERROR, "received unexpected packet");
					delete tmp_packet;
					continue;
			}
		}

		if (tmp_is_parity == false) {

			if (tmp_packet->parseHeader(true) == false) {
				HPVT_logging(LOG_LEVEL_ALERT, "Invalid packet!");
				delete tmp_packet;
				continue;
			}

			received_seqno = tmp_packet->getFrameSeqno();

		}
		decodeTimeManager->setLatestSeqno(received_seqno);

		if (g_context->connection.send_recv_seqno != received_seqno) {
			g_context->connection.send_recv_seqno = received_seqno;
		}

		if (tmp_packet->getDummyDataLength() > 0) {
			uint16_t tmp_recievable_mtu;
			tmp_recievable_mtu = tmp_received_length + HPVT_PACKET_IP_UDP_HEADER_LENGTH + HPVT_PACKET_SCS_HEADER_LENGTH_MAXIMUM;
			HPVT_send_notification_path_mtu_discovery(tmp_recievable_mtu, received_seqno);
		}

		if (tmp_is_parity == false && sequenceManager->getProcessingSeqno() == HPVT_Queue_FRAME_SEQNO_INVALID) {
			sequenceManager->setProcessingSeqno(received_seqno);
			decodeTimeManager->updateFps(tmp_packet->getFps(), tmp_packet->getIdrPeriod());
			decodeTimeManager->setBaseSeqno(received_seqno);

			uint32_t tmp_wait_margin;
			tmp_wait_margin = decodeTimeManager->getDesiredDelayTime();
			HPVT_logging(LOG_LEVEL_NOTICE, "DEC set first delay %lu", tmp_wait_margin);
			decodeTimeManager->setNextDecodeTime(tmp_packet->getReceivedTime() + tmp_wait_margin);
			HPVT_logging(LOG_LEVEL_NOTICE, "DEC set seqno %d", received_seqno);

		}

		tmp_packet->setIdealDecodeTime(tmp_packet->getReceivedTime() + decodeTimeManager->getDesiredDelayTime());
		difference = sequenceManager->compare(received_seqno);

		if (difference > 0) {
			old_frame_count = 0;
			tmp_old_frame_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;

			if (receivePacketManager->add(tmp_packet, received_seqno, tmp_is_fec, tmp_is_parity) == false) {
				delete tmp_packet;
			}

		}
		else if (difference == 0) {

			if (tmp_is_parity == false) {

				if (decodeTimeManager->getBaseSeqno() == HPVT_Queue_FRAME_SEQNO_INVALID) {
					decodeTimeManager->updateFps(tmp_packet->getFps(), tmp_packet->getIdrPeriod());
					decodeTimeManager->setBaseSeqno(received_seqno);
				}
				builder->update(tmp_packet);
			}

			delete tmp_packet;
			old_frame_count = 0;
			tmp_old_frame_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;
		}
		else {

			if (decodeTimeManager->updateFps(tmp_packet->getFps(), tmp_packet->getIdrPeriod()) == true) {
				sequenceManager->reset();
				receivePacketManager->clear();
			}

			if (tmp_old_frame_seqno != received_seqno) {
				old_frame_count++;
				HPVT_logging(LOG_LEVEL_NOTICE, "DEC old_frame_count %lu (%d %d)", old_frame_count, tmp_old_frame_seqno, received_seqno);
				tmp_old_frame_seqno = received_seqno;
				if (old_frame_count > RESET_THRESHOLD_OLD_FRAMES) {
					decodeTimeManager->setLongDelayFlag(true);
				}
				else {
					decodeTimeManager->setLongDelayFlag(false);
				}
			}

			if (receivePacketManager->add(tmp_packet, received_seqno, tmp_is_fec, tmp_is_parity) == false) {
				delete tmp_packet;
			}
		}
	}
}
void HPVT_start_thread_received_packet_list(HPVT_Config *config) {

	HPVT_Context_ready_wait(g_context);

	int list_length;
	int i;
	HPVT_STOCK_PACKET_ENTRY *tmp_entry;
	int difference;
	uint32_t tmp_received_time = 0;

	FECData * tmp_ptr;
	size_t tmp_cnt;
	FECData tmp_results[1024];
	size_t tmp_results_cnt = 1024;

	HPVTPacket *tmp_packet;

	FECDecoder tmp_decoder;
	FECDecoderInitialize(&tmp_decoder);

	if (FECDecoderStandBy(&tmp_decoder, 1024) == false) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	while (true) {

		sequenceManager->lock();
		sequenceManager->condition_wait();

		receivePacketManager->lock();
		tmp_received_time = 0;

		list_length = receivePacketManager->getPacketListLength();

		for (i = 0; i < list_length; i++) {

			tmp_entry = &(receivePacketManager->getPacketList()[i]);

			if (tmp_entry == NULL) {
				HPVT_logging(LOG_LEVEL_ALERT, "!");
				break;
			}

			if (tmp_entry->packet == NULL) {
				HPVT_logging(LOG_LEVEL_ALERT, "!");
				break;
			}

			if (tmp_entry->seqno == sequenceManager->getProcessingSeqno()) {

				if (tmp_entry->is_fec == true) {

					if (FECDecoderAdd(&tmp_decoder, tmp_entry->packet->getBuffer(), tmp_entry->packet->getBufferLength()) == false) {
						HPVT_log_UNEXPECTED_ERROR();
					}

					if (tmp_received_time == 0) {
						tmp_received_time = tmp_entry->packet->getReceivedTime();
					}
				}
				else {
					builder->update(tmp_entry->packet);
				}
			}
		}

		if (tmp_decoder.count > 0) {
			tmp_ptr = tmp_results;
			tmp_cnt = tmp_results_cnt;

			if (FECDecoderDecode(&tmp_decoder, tmp_ptr, &tmp_cnt) == true) {

				for (int i = 0; i < tmp_cnt; i++) {
					tmp_packet = new HPVTPacket();
					tmp_packet->setBuffer(tmp_results[i].ptr, tmp_results[i].size);
					tmp_packet->setBufferLength(tmp_results[i].size);
					tmp_packet->setReceivedTime(tmp_received_time);
					if (tmp_packet->parseHeader(false) == false) {
						HPVT_logging(LOG_LEVEL_ALERT, "Invalid packet!");
					}
					else {

						builder->update(tmp_packet);
					}

					delete tmp_packet;
					free(tmp_results[i].ptr);
				}
			}
			else {
				HPVT_logging(LOG_LEVEL_WARNING, "failed to FEC Decode(gid=%d,count=%d)", tmp_decoder.gid, tmp_decoder.count);
			}

			uint64_t tmp_parity = tmp_decoder.statistics.parity;
			uint64_t tmp_success = tmp_decoder.statistics.recovery_success;
			uint64_t tmp_failure = tmp_decoder.statistics.recovery_failure;
		}

		FECDecoderCleanUp(&tmp_decoder);

		for (i = 0; i < list_length; i++) {

			tmp_entry = &(receivePacketManager->getPacketList()[i]);

			if (tmp_entry == NULL) {
				HPVT_logging(LOG_LEVEL_ALERT, "!");
				break;
			}

			if (tmp_entry->packet == NULL) {
				HPVT_logging(LOG_LEVEL_ALERT, "!");
				break;
			}

			difference = sequenceManager->compare(tmp_entry->seqno);

			if (difference < 0) {

				tmp_entry->deleted = true;
				delete tmp_entry->packet;

				if (tmp_entry->is_parity == false) {
				}

			}
			else if (difference == 0) {

				tmp_entry->deleted = true;
				delete tmp_entry->packet;
			}

		}

		receivePacketManager->eraseDeletedEntries();
		receivePacketManager->unlock();

		sequenceManager->unlock();
	}

	FECDecoderFinalize(&tmp_decoder);
}
boolean HPVT_get_video_frame_data(unsigned char* buffer, unsigned int* length, HPVT_Queue_FRAME_TYPE* frame_type) {

	if (g_context->connection.flag_timeout == true) {
		_HPVT_reset_state();
		HPVT_logging(LOG_LEVEL_NOTICE, "Detected Timeout");
		usleep(100000);
		*length = 0;
		return false;
	}

	if (buffer == NULL) {
		HPVT_logging(LOG_LEVEL_WARNING, "!");
		*length = 0;
		return false;
	}

	if (g_context->connection.reconnection_attempt_on == false) {
		_HPVT_reset_state();
		usleep(100000);
		*length = 0;
		return false;
	}

	uint32_t tmp_time_now = 0;

	while (true) {

		tmp_time_now = HPVT_get_current_time();
		if (tmp_time_now >= (decodeTimeManager->getNextDecodeTime())) {
			break;
		}

		usleep(2000);
	}

	if (sequenceManager->getProcessingSeqno() == HPVT_Queue_FRAME_SEQNO_INVALID) {
		*length = 0;
		return false;
	}

	sequenceManager->lock();

	g_context->connection.processing_seqno = sequenceManager->getProcessingSeqno();

	HPVTVideoFrame *buffering_frame = builder->getCurrentFrame();
	uint32_t tmp_length = buffering_frame->getLength();
	memcpy(buffer, buffering_frame->getBuffer(), tmp_length);
	*length = tmp_length;
	int tmp_filled_length = buffering_frame->getFilledDataLength();
	HPVT_Queue_FRAME_TYPE tmp_type = buffering_frame->getFrameType();
	*frame_type = tmp_type;

	if (tmp_length > 0) {
		zero_count = 0;

		HPVT_Context_update_encode_parameters(g_context, buffering_frame->getWidth(), buffering_frame->getHeight(), //
			buffering_frame->getFps());
	}
	else {
		zero_count++;
		HPVT_logging(LOG_LEVEL_NOTICE, "DEC zero_frame_count %lu", zero_count);
	}

	builder->changeFrame();

	uint16_t tmp_buffering_rate;
	HPVT_Queue_FRAME_SEQNO next_process_frame_seqno;

	tmp_buffering_rate = decodeTimeManager->getBufferedRate(sequenceManager->getProcessingSeqno());

	decodeTimeManager->updateNextDecodeTime(tmp_time_now, sequenceManager->getProcessingSeqno(), tmp_buffering_rate);

	LOG_LEVEL tmp_log_level;
	if ((sequenceManager->getProcessingSeqno() & 0xff) == 0x01) {
		tmp_log_level = LOG_LEVEL_NOTICE;
	}
	else {
		tmp_log_level = LOG_LEVEL_INFO;
	}
	HPVT_logging(tmp_log_level, "DEC VFrame seq=%04d,type=%d,time=%lu,next=%lu,buffered_rate=%d,length=%lu/%lu", //
			sequenceManager->getProcessingSeqno(), tmp_type, tmp_time_now, decodeTimeManager->getNextDecodeTime(), //
			tmp_buffering_rate, tmp_filled_length, tmp_length);

	if (tmp_buffering_rate < 250) {
		sequenceManager->incrementSeqno();
	}
	else {
		next_process_frame_seqno = receivePacketManager->getOldestIFrameSeqno();

		if (next_process_frame_seqno != HPVT_Queue_FRAME_SEQNO_INVALID) {

			int diff_frame = HPVT_compare_frame_sequence_number(sequenceManager->getProcessingSeqno(), next_process_frame_seqno);

			if (diff_frame >= 1) {

				if (abs(next_process_frame_seqno - sequenceManager->getProcessingSeqno()) != 1) {
					HPVT_logging(LOG_LEVEL_NOTICE, "DEC Skip VFrame seqno %d > %d", sequenceManager->getProcessingSeqno(), next_process_frame_seqno);
				}

				sequenceManager->setProcessingSeqno(next_process_frame_seqno);
			}
			else {
				sequenceManager->incrementSeqno();
			}
		}
		else {
			sequenceManager->incrementSeqno();
		}
	}

	int threshold;
	if (decodeTimeManager->isLongDelayFlag() == true) {

		if (g_context->settings.video.timeout_threshold > 0) {
			threshold = decodeTimeManager->getFps() * g_context->settings.video.timeout_threshold;
		}
		else {
			threshold = decodeTimeManager->getFps() * HPVT_Config_TIMEOUT_DEFAULT;
		}
	}
	else{
		if (g_context->settings.video.timeout_threshold > 0) {
			threshold = decodeTimeManager->getFps() * g_context->settings.video.timeout_threshold;
		}
		else {
			threshold = RESET_THRESHOLD_ZERO_FRAMES;
		}
	}

	if (zero_count > threshold) {
		decodeTimeManager->setLongDelayFlag(false);

		sequenceManager->reset();
		decodeTimeManager->reset();
		receivePacketManager->clear();
		g_context->connection.flag_reset_resolution = true;
		zero_count = 0;
	}

	sequenceManager->signal();
	sequenceManager->unlock();

	return true;

}
void HPVT_update_buffering_delay_time(void) {

	decodeTimeManager->setDelayTime(g_context->settings.video.buffering_delay * 1000);
}
