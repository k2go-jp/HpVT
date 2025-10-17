#define FILE_NUMBER "Q211"

#include "HPVTPacket.h"

HPVTPacket::HPVTPacket() {

	memset(buffer, 0, sizeof(buffer));
	buffer_length = 0;
	start_position = 0;
	dummy_data_length = 0;
	padding = 0;

	frame_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;
	frame_type = HPVT_Queue_FRAME_TYPE_NONE;
	frame_length = 0;
	generated_time = 0;
	packet_total = 0;
	packet_seqno = 0;

	width = 0;
	height = 0;
	fps = 0;
	bitrate = 0;
	idr_period = 0;
	h265_flag = 0;
	fec_level = 0;
	received_time = 0;
	ideal_decode_time = 0;
	checksum = 0;

	packet_type = HPVT_PACKET_TYPE_NORMAL;
}
HPVTPacket::~HPVTPacket() {

}
unsigned char* HPVTPacket::getHeader(void) {

	return (buffer + start_position);
}
unsigned char *HPVTPacket::getPayload(void) {

	return (buffer + start_position + HPVT_PACKET_HEADER_LENGTH);
}
uint32_t HPVTPacket::getPayloadLength(void) {

	/* contains padding data and dummy data */
	return (buffer_length - start_position - HPVT_PACKET_HEADER_LENGTH);
}
void HPVTPacket::setBuffer(unsigned char *ptr, uint32_t len) {

	memcpy(buffer, ptr, len);
}
void HPVTPacket::setPayloadData(unsigned char *payload, uint32_t payload_len) {

	if (payload == NULL) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return;
	}

	if (payload_len == 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return;
	}

	//

	memcpy(buffer + start_position + HPVT_PACKET_HEADER_LENGTH, payload, payload_len);
	buffer_length = start_position + HPVT_PACKET_HEADER_LENGTH + payload_len;

	if (dummy_data_length > 0) {
		memset(buffer + start_position + HPVT_PACKET_HEADER_LENGTH + payload_len, 0xff, dummy_data_length);
		buffer_length += dummy_data_length;
	}

}
static uint16_t HPVT_calculate_checksum(unsigned char *buf, int len) {
	uint16_t *p;
	uint32_t sum = 0;

	p = (uint16_t *) buf;
	while (len > 1) {
		sum += *p;
		p++;
		len -= sizeof(uint16_t);
	}
	if (len) {
		sum += *(uint8_t *) p;
	}
	sum = (sum & 0xffff) + (sum >> 16);
	sum += (sum >> 16);

	return (~sum & 0xffff);
}
boolean HPVTPacket::composeHeader() {

	unsigned char *ptr;
	ptr = buffer;

	uint32_t tmp_container_a;
	uint32_t tmp_container_b;
	uint16_t tmp_container_c;

	memcpy(ptr, &packet_type, sizeof(packet_type));
	ptr += sizeof(packet_type);

	memcpy(ptr, &frame_seqno, sizeof(frame_seqno));
	ptr += sizeof(frame_seqno);

	memcpy(ptr, &packet_seqno, sizeof(packet_seqno));
	ptr += sizeof(packet_seqno);

	memcpy(ptr, &packet_total, sizeof(packet_total));
	ptr += sizeof(packet_total);

	//

	tmp_container_a = 0x0;
	tmp_container_a |= (frame_type << 30);
	tmp_container_a |= (width << 21);
	tmp_container_a |= (height << 12);
	tmp_container_a |= (padding << 11);
	tmp_container_a |= (dummy_data_length & 0x07ff);

	memcpy(ptr, &tmp_container_a, sizeof(tmp_container_a));
	ptr += sizeof(tmp_container_a);

	//

	memcpy(ptr, &frame_length, sizeof(frame_length));
	ptr += sizeof(frame_length);

	memcpy(ptr, &generated_time, sizeof(generated_time));
	ptr += sizeof(generated_time);

	tmp_container_b = 0x0;
	tmp_container_b |= (bitrate << 8);
	tmp_container_b |= (fps << 1);
	tmp_container_b |= (h265_flag & 0x01);

	memcpy(ptr, &tmp_container_b, sizeof(tmp_container_b));
	ptr += sizeof(tmp_container_b);

	tmp_container_c = 0x0;
	tmp_container_c |= (idr_period << 4);
	tmp_container_c |= (fec_level & 0x0f);

	memcpy(ptr, &tmp_container_c, sizeof(tmp_container_c));
	ptr += sizeof(tmp_container_c);

	uint16_t tmp_reserved = 0x0;
	memcpy(ptr, &tmp_reserved, sizeof(tmp_reserved));
	ptr += 2;

	checksum = HPVT_calculate_checksum(buffer, buffer_length);

	memcpy(ptr, &checksum, sizeof(checksum));

	return true;
}
boolean HPVTPacket::parseHeader(boolean first_time) {

	unsigned char *ptr;

	ptr = buffer + start_position;

	memcpy(&packet_type, ptr, sizeof(packet_type));
	ptr += sizeof(packet_type);

	memcpy(&frame_seqno, ptr, sizeof(frame_seqno));
	ptr += sizeof(frame_seqno);

	memcpy(&packet_seqno, ptr, sizeof(packet_seqno));
	ptr += sizeof(packet_seqno);

	memcpy(&packet_total, ptr, sizeof(packet_total));
	ptr += sizeof(packet_total);

	uint32_t tmp_container_a;

	memcpy(&tmp_container_a, ptr, sizeof(tmp_container_a));
	ptr += sizeof(tmp_container_a);

	frame_type = tmp_container_a >> 30;
	width = (tmp_container_a >> 21) & 0x000001ff;
	height = (tmp_container_a >> 12) & 0x000001ff;
	padding = (tmp_container_a >> 11) & 0x00000001;
	dummy_data_length = tmp_container_a & 0x000007ff;

	memcpy(&frame_length, ptr, sizeof(frame_length));
	ptr += sizeof(frame_length);

	memcpy(&generated_time, ptr, sizeof(generated_time));
	ptr += sizeof(generated_time);

	uint32_t tmp_container_b;

	memcpy(&tmp_container_b, ptr, sizeof(tmp_container_b));
	ptr += sizeof(tmp_container_b);

	bitrate = tmp_container_b >> 8;
	fps = tmp_container_b >> 1 & 0x0000007f;
	h265_flag = tmp_container_b & 0x00000001;

	uint16_t tmp_container_c;

	memcpy(&tmp_container_c, ptr, sizeof(tmp_container_c));
	ptr += sizeof(tmp_container_c);

	idr_period = (tmp_container_c >> 4) & 0x0fff;
	fec_level = tmp_container_c & 0x000f;

	uint16_t tmp_reserved;
	memcpy(&tmp_reserved, ptr, sizeof(tmp_reserved));
	ptr += sizeof(tmp_reserved);

	if (first_time == true) {

		uint16_t tmp_checksum_received;
		memcpy(&tmp_checksum_received, ptr, sizeof(tmp_checksum_received));

		uint16_t tmp_checksum_calculated;
		memset(ptr, 0, sizeof(checksum));

		tmp_checksum_calculated = HPVT_calculate_checksum(buffer + start_position, buffer_length - start_position);

		memcpy(ptr, &tmp_checksum_received, sizeof(checksum));

		if (tmp_checksum_received != tmp_checksum_calculated) {
			HPVT_logging(LOG_LEVEL_ALERT, "check sum error!");
			return false;
		}
	}

	return true;
}
