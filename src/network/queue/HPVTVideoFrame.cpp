#define FILE_NUMBER "Q221"

#include "HPVTVideoFrame.h"

#include "queue.h"

HPVTVideoFrame::HPVTVideoFrame() {

	buffer = (unsigned char *) malloc(HPVT_FRAME_LENGTH_MAXIMUM);
	memset(buffer, 0, HPVT_FRAME_LENGTH_MAXIMUM);

	length = 0;

	frame_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;
	frame_type = HPVT_Queue_FRAME_TYPE_NONE;
	generated_time = 0;
	received_time = 0;
	ideal_decode_time = 0;

	width = 0;
	height = 0;
	fps = 0;
	bitrate = 0;
	idr_period = 0;
	h265_flag = 0;
	fec_level = 0;

	filled_data_length = 0;
}
HPVTVideoFrame::HPVTVideoFrame(unsigned char *codec_buf, uint32_t codec_len, unsigned char *deta_buf, uint32_t data_len) {

	buffer = NULL;
	length = 0;

	frame_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;
	frame_type = HPVT_Queue_FRAME_TYPE_NONE;
	generated_time = 0;
	received_time = 0;
	ideal_decode_time = 0;
	width = 0;
	height = 0;
	fps = 0;
	bitrate = 0;
	idr_period = 0;
	h265_flag = 0;
	fec_level = 0;
	filled_data_length = 0;

	if (deta_buf == NULL) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return;
	}

	if (data_len == 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return;
	}

	if (data_len > HPVT_FRAME_LENGTH_MAXIMUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return;
	}

	buffer = (unsigned char *) malloc(codec_len + data_len);
	memset(buffer, 0, codec_len + data_len);

	if (buffer == NULL) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return;
	}

	if (codec_len > 0) {
		memcpy(buffer, codec_buf, codec_len);
		memcpy(buffer + codec_len, deta_buf, data_len);
		length = data_len + codec_len;
	}
	else {
		memcpy(buffer, deta_buf, data_len);
		length = data_len;
	}
}
HPVTVideoFrame::~HPVTVideoFrame() {

	free(buffer);

}
boolean HPVTVideoFrame::setPacketizedData(HPVTPacket *packet) {

	int tmp_offset;

	if (packet->getPacketSeqno() == packet->getPacketTotal() && packet->getPadding() == HPVT_PACKET_PADDING_OFF) {
		// last packet
		tmp_offset = packet->getFrameLength() - (packet->getPayloadLength() - packet->getDummyDataLength());
	}
	else {
		tmp_offset = packet->getPayloadLength() * (packet->getPacketSeqno() - 1);
	}

	if (tmp_offset > packet->getFrameLength()) {
		HPVT_logging(LOG_LEVEL_ERROR, "offset exceeds buffer length (%d)", tmp_offset);
		return false;
	}

	memcpy(buffer + tmp_offset, packet->getPayload(), packet->getPayloadLength() - packet->getDummyDataLength());

	if (frame_type == 0) {
		frame_type = packet->getFrameType();
	}

	if (frame_seqno == 0) {
		frame_seqno = packet->getFrameSeqno();
	}

	if (length == 0) {
		length = packet->getFrameLength();
	}

	if (width == 0) {
		width = packet->getWidth();
	}

	if (height == 0) {
		height = packet->getHeight();
	}

	if (bitrate == 0) {
		bitrate = packet->getBitrate() * 1000;
	}

	if (fps == 0) {
		fps = packet->getFps();
	}

	if (idr_period == 0) {
		idr_period = packet->getIdrPeriod();
	}

	fec_level = packet->getFecLevel();
	h265_flag = packet->getH265Flag();

	if (generated_time == 0) {
		generated_time = packet->getGeneratedTime();
	}

	if (received_time == 0) {
		received_time = packet->getReceivedTime();
	}

	if (ideal_decode_time == 0) {
		ideal_decode_time = packet->getIdealDecodeTime();
	}

	if (packet->getPacketSeqno() != packet->getPacketTotal()) {
		filled_data_length += packet->getPayloadLength();
	}
	else {
		filled_data_length += packet->getFrameLength() - tmp_offset;
	}

	HPVT_logging(LOG_LEVEL_DEBUG, "filled packet in VFlame(Fseqno=%u,Pseqno=%u/%u,offset=%d, buffer_length=%d, packet_length=%lu,filled_length=%lu", //
			packet->getFrameSeqno(), packet->getPacketSeqno(), packet->getPacketTotal(), tmp_offset, length, packet->getPayloadLength(), filled_data_length);
	return true;
}
void HPVTVideoFrame::reset() {

	memset(buffer, 0, HPVT_FRAME_LENGTH_MAXIMUM);

	length = 0;
	frame_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;
	frame_type = HPVT_Queue_FRAME_TYPE_NONE;
	generated_time = 0;
	received_time = 0;
	ideal_decode_time = 0;
	width = 0;
	height = 0;
	fps = 0;
	bitrate = 0;
	idr_period = 0;

	filled_data_length = 0;

}
boolean HPVTVideoFrame::isFilledData() {

	if (length == 0) {
		return false;
	}

	if (filled_data_length == 0) {
		return false;
	}

	if (length == filled_data_length) {
		return true;
	}
	else {
		return false;
	}

}
