#define FILE_NUMBER "Q212"

#include <unistd.h>

#include "scs/6/fec/fec.h"

#include "HPVTPacketSet.h"

static uint32_t serial_number = 1;

HPVTPacketSet::HPVTPacketSet() {

	count = 0;
	packet_size = HPVT_PACKET_PAYLOAD_LENGTH_DEFAULT;
	packet_padding_enabled = false;
	mtu_discovery_enabled = false;
	path_mtu = 0;
	trial_mtu = 0;

	fec_enabled = true;
	scs_enabled = true;

	frame_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;
	frame_type = HPVT_Queue_FRAME_TYPE_NONE;
	frame_length = 0;
	generated_time = 0;

	width = 0;
	height = 0;
	fps = 0;
	bitrate = 0;
	idr_period = 0;
	h265_flag = 0;
	fec_level = 0;

	deleted = false;

	if (pthread_mutex_init(&packetize_locked, 0)) {
		HPVT_log_UNEXPECTED_ERROR();
	}

}
HPVTPacketSet::~HPVTPacketSet() {

	removePackets();

	if (pthread_mutex_destroy(&packetize_locked)) {
		HPVT_log_UNEXPECTED_ERROR();
	}

}
boolean HPVTPacketSet::packetize(HPVTVideoFrame *frame) {

	HPVT_Lock(packetize_locked);

	unsigned char *ptr;
	uint32_t len;
	uint32_t bufflen;
	int tmp_total_packet_number;

	ptr = (unsigned char *) frame->getBuffer();
	bufflen = frame->getLength();

	if (ptr == NULL) {
		HPVT_Unlock(packetize_locked);
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	int extra_header_size = 0;

	if (scs_enabled == true) {
		extra_header_size += HPVT_PACKET_SCS_HEADER_LENGTH_MAXIMUM;
	}

	if (fec_enabled == true) {
		extra_header_size += FEC_HEADER_SIZE;
	}

	packet_size = path_mtu - HPVT_PACKET_IP_UDP_HEADER_LENGTH - HPVT_PACKET_HEADER_LENGTH - extra_header_size;
	if (bufflen == 0) {
		HPVT_Unlock(packetize_locked);
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}
	else if (bufflen > (HPVT_PACKET_SET_PACKET_ARRAY_SIZE * packet_size)) {
		HPVT_Unlock(packetize_locked);
		HPVT_logging(LOG_LEVEL_WARNING, "frame size is too long! (%d > %d)",bufflen, HPVT_PACKET_SET_PACKET_ARRAY_SIZE * packet_size);
		return false;
	}

	frame_type = frame->getFrameType();
	generated_time = frame->getGeneratedTime();
	frame_seqno = frame->getFrameSeqno();
	frame_length = frame->getLength();
	width = frame->getWidth();
	height = frame->getHeight();
	fps = frame->getFps();
	idr_period = frame->getIdrPeriod();
	h265_flag = frame->getH265Flag();
	bitrate = frame->getBitrate();
	fec_level = frame->getFecLevel();

	tmp_total_packet_number = bufflen / packet_size;

	if (bufflen % packet_size != 0) {
		tmp_total_packet_number += 1;
	}

	count = 0;

	while (bufflen > 0) {

		if (bufflen > packet_size) {
			len = packet_size;
		}
		else {
			len = bufflen;
		}

		HPVTPacket *tmp_packet = new HPVTPacket();
		tmp_packet->setFrameType(frame->getFrameType());
		tmp_packet->setFrameSeqno(frame->getFrameSeqno());
		tmp_packet->setGeneratedTime(frame->getGeneratedTime());
		tmp_packet->setFrameLength(frame->getLength());
		tmp_packet->setWidth(frame->getWidth());
		tmp_packet->setHeight(frame->getHeight());
		tmp_packet->setFps(frame->getFps());
		tmp_packet->setIdrPeriod(frame->getIdrPeriod());
		tmp_packet->setH265Flag(frame->getH265Flag());
		tmp_packet->setBitrate(frame->getBitrate() / 1000);
		tmp_packet->setFecLevel(frame->getFecLevel());
		tmp_packet->setPacketTotal(tmp_total_packet_number);
		tmp_packet->setPacketSeqno(count + 1);

		if (count == tmp_total_packet_number - 1) {
			if (mtu_discovery_enabled == true) {

				if (trial_mtu > HPVT_PACKET_MTU_MAXIMUM) {
					HPVT_Unlock(packetize_locked);
					HPVT_logging(LOG_LEVEL_ERROR, "!");
					return false;
				}

				int dummy_len;
				dummy_len = trial_mtu - (HPVT_PACKET_IP_UDP_HEADER_LENGTH + HPVT_PACKET_HEADER_LENGTH + extra_header_size) - len;

				if (dummy_len > 0) {
					tmp_packet->setDummyDataLength(dummy_len);
					HPVT_logging(LOG_LEVEL_NOTICE, "Try to MTU size %d(extra data size %d)", trial_mtu, dummy_len);
				}
				else {
					HPVT_Unlock(packetize_locked);
					HPVT_logging(LOG_LEVEL_ERROR, "!");
					return false;
				}
			}
		}

		tmp_packet->setPayloadData(ptr, len);

		if (count == tmp_total_packet_number - 1) {

			if (packet_padding_enabled == true) {
				tmp_packet->setPadding(HPVT_PACKET_PADDING_ON);
				tmp_packet->setBufferLength(HPVT_PACKET_HEADER_LENGTH + packet_size);
			}
		}

		tmp_packet->composeHeader();

		packets[count] = tmp_packet;

		count++;
		serial_number++;

		ptr += len;
		bufflen -= len;
	}

	HPVT_Unlock(packetize_locked);

	return true;
}
HPVTPacket* HPVTPacketSet::getPacketData(int index) {

	return packets[index];
}
boolean HPVTPacketSet::removePackets() {

	HPVT_Lock(packetize_locked);

	for (int i = 0; i < count; i++) {
		delete packets[i];
	}
	count = 0;
	HPVT_Unlock(packetize_locked);

	return true;
}
