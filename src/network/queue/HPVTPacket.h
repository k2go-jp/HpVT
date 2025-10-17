#ifndef QUEUE_HPVTPACKET_H_
#define QUEUE_HPVTPACKET_H_

#include "../../common_s.h"
#include "../../logger/logger.h"

class HPVTPacket {

	unsigned char buffer[HPVT_PACKET_MTU_MAXIMUM];
	uint32_t buffer_length;
	uint16_t start_position;
	uint8_t packet_type;

	HPVT_Queue_FRAME_TYPE frame_type;
	HPVT_Queue_FRAME_SEQNO frame_seqno;
	uint32_t generated_time;
	uint32_t frame_length;

	uint8_t padding;
	uint16_t dummy_data_length;
	uint16_t packet_total;
	HPVT_Queue_PACKET_SEQNO packet_seqno;

	uint16_t width;
	uint16_t height;
	uint8_t fps;
	uint32_t bitrate;
	uint16_t idr_period;
	uint8_t h265_flag;
	uint8_t fec_level;

	uint32_t received_time;
	uint32_t ideal_decode_time;

	uint16_t checksum;

public:
	HPVTPacket();
	~HPVTPacket();

	void setBuffer(unsigned char *, uint32_t);
	void setPayloadData(unsigned char *, uint32_t);

	unsigned char *getHeader(void);
	unsigned char *getPayload(void);
	uint32_t getPayloadLength();

	boolean composeHeader();
	boolean parseHeader(boolean);

	unsigned char* getBuffer() {
		return buffer;
	}

	uint32_t getBufferLength() const {
		return buffer_length;
	}

	HPVT_Queue_FRAME_TYPE getFrameType() const {
		return frame_type;
	}

	void setFrameType(HPVT_Queue_FRAME_TYPE frameType) {
		frame_type = frameType;
	}

	uint32_t getGeneratedTime() const {
		return generated_time;
	}

	void setGeneratedTime(uint32_t generatedTime) {
		generated_time = generatedTime;
	}

	HPVT_Queue_FRAME_SEQNO getFrameSeqno() const {
		return frame_seqno;
	}

	void setFrameSeqno(HPVT_Queue_FRAME_SEQNO frameSeqno) {
		frame_seqno = frameSeqno;
	}

	uint16_t getPacketTotal() const {
		return packet_total;
	}

	void setPacketTotal(uint16_t packetTotal) {
		packet_total = packetTotal;
	}

	HPVT_Queue_PACKET_SEQNO getPacketSeqno() const {
		return packet_seqno;
	}

	void setPacketSeqno(HPVT_Queue_PACKET_SEQNO packetSeqno) {
		packet_seqno = packetSeqno;
	}


	void setBufferLength(int32_t bufferLength) {
		buffer_length = bufferLength;
	}

	uint32_t getFrameLength() const {
		return frame_length;
	}

	void setFrameLength(uint32_t frameLength) {
		frame_length = frameLength;
	}

	uint32_t getReceivedTime() const {
		return received_time;
	}

	void setReceivedTime(uint32_t receivedTime) {
		received_time = receivedTime;
	}

	uint8_t getPacketType() const {
		return packet_type;
	}

	void setPacketType(uint8_t packetType) {
		packet_type = packetType;
	}

	uint16_t getWidth() const {
		return width;
	}

	void setWidth(uint16_t width) {
		this->width = width;
	}

	uint16_t getHeight() const {
		return height;
	}

	void setHeight(uint16_t height) {
		this->height = height;
	}

	uint32_t getBitrate() const {
		return bitrate;
	}

	void setBitrate(uint32_t bitrate) {
		this->bitrate = bitrate;
	}

	uint8_t getFps() const {
		return fps;
	}

	void setFps(uint8_t fps) {
		this->fps = fps;
	}

	uint16_t getIdrPeriod() const {
		return idr_period;
	}

	void setIdrPeriod(uint16_t idrPeriod) {
		idr_period = idrPeriod;
	}

	uint16_t getChecksum() const {
		return checksum;
	}

	uint16_t getStartPosition() const {
		return start_position;
	}

	void setStartPosition(uint16_t startPosition) {
		start_position = startPosition;
	}

	uint16_t getDummyDataLength() const {
		return dummy_data_length;
	}

	void setDummyDataLength(uint16_t dummyDataLength) {
		dummy_data_length = dummyDataLength;
	}

	uint8_t getPadding() const {
		return padding;
	}

	void setPadding(uint8_t padding) {
		this->padding = padding;
	}

	uint8_t getFecLevel() const {
		return fec_level;
	}

	void setFecLevel(uint8_t fecLevel) {
		fec_level = fecLevel;
	}

	uint8_t getH265Flag() const {
		return h265_flag;
	}

	void setH265Flag(uint8_t h265Flag) {
		h265_flag = h265Flag;
	}

	uint32_t getIdealDecodeTime() const {
		return ideal_decode_time;
	}

	void setIdealDecodeTime(uint32_t idealDecodeTime) {
		ideal_decode_time = idealDecodeTime;
	}
};

#endif /* QUEUE_HPVTPACKET_H_ */
