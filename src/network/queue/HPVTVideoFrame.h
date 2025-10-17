#ifndef QUEUE_HPVTVIDEOFRAME_H_
#define QUEUE_HPVTVIDEOFRAME_H_

#include "../../common_s.h"
#include "../../logger/logger.h"

#include "HPVTPacket.h"

class HPVTVideoFrame {

	unsigned char *buffer;
	uint32_t length;

	HPVT_Queue_FRAME_TYPE frame_type;
	HPVT_Queue_FRAME_SEQNO frame_seqno;
	uint32_t generated_time;
	uint32_t received_time;
	uint32_t ideal_decode_time;

	uint16_t width;
	uint16_t height;
	uint8_t fps;
	uint32_t bitrate;
	uint16_t idr_period;
	uint8_t h265_flag;
	uint8_t fec_level;

	uint32_t filled_data_length;

public:
	HPVTVideoFrame();
	HPVTVideoFrame(unsigned char *, uint32_t, unsigned char *, uint32_t);
	~HPVTVideoFrame();

	boolean setPacketizedData(HPVTPacket *);
	void reset();
	boolean isFilledData();

	uint32_t getLength() const {
		return length;
	}

	void setLength(uint32_t length) {
		this->length = length;
	}

	unsigned char* getBuffer() const {
		return buffer;
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

	uint8_t getFps() const {
		return fps;
	}

	void setFps(uint8_t fps) {
		this->fps = fps;
	}

	uint32_t getBitrate() const {
		return bitrate;
	}

	void setBitrate(uint32_t bitrate) {
		this->bitrate = bitrate;
	}

	uint32_t getFilledDataLength() const {
		return filled_data_length;
	}

	void setFilledDataLength(uint32_t filledDataLength) {
		filled_data_length = filledDataLength;
	}

	uint16_t getIdrPeriod() const {
		return idr_period;
	}

	void setIdrPeriod(uint16_t idrPeriod) {
		idr_period = idrPeriod;
	}

	uint32_t getReceivedTime() const {
		return received_time;
	}

	void setReceivedTime(uint32_t receivedTime) {
		received_time = receivedTime;
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

#endif /* QUEUE_HPVTVIDEOFRAME_H_ */
