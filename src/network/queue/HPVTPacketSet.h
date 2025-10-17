#ifndef QUEUE_HPVTPACKETSET_H_
#define QUEUE_HPVTPACKETSET_H_

#include "../../common_s.h"
#include "../../logger/logger.h"

#include "HPVTPacket.h"
#include "HPVTVideoFrame.h"

class HPVTPacketSet {

	HPVTPacket* packets[HPVT_PACKET_SET_PACKET_ARRAY_SIZE];
	uint16_t count;
	uint16_t packet_size;
	boolean packet_padding_enabled;
	boolean mtu_discovery_enabled;
	uint16_t trial_mtu;

	boolean fec_enabled;
	boolean scs_enabled;

	uint16_t path_mtu;
	pthread_mutex_t packetize_locked;

	HPVT_Queue_FRAME_TYPE frame_type;
	HPVT_Queue_FRAME_SEQNO frame_seqno;
	uint32_t frame_length;
	uint32_t generated_time;

	uint16_t width;
	uint16_t height;
	uint8_t fps;
	uint32_t bitrate;
	uint16_t idr_period;
	uint8_t h265_flag;
	uint8_t fec_level;

	boolean deleted;


public:
	HPVTPacketSet();
	~HPVTPacketSet();

	boolean packetize(HPVTVideoFrame *);
	HPVTPacket* getPacketData(int);
	boolean removePackets();

	uint16_t getCount() const {
		return count;
	}

	uint16_t getPathMtu() const {
		return path_mtu;
	}

	void setPathMtu(uint16_t pathMtu) {
		path_mtu = pathMtu;
	}

	uint16_t getPacketSize() const {
		return packet_size;
	}

	void setPacketSize(uint16_t packetSize) {
		packet_size = packetSize;
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

	uint32_t getFrameLength() const {
		return frame_length;
	}

	void setFrameLength(uint32_t frameLength) {
		frame_length = frameLength;
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

	uint16_t getIdrPeriod() const {
		return idr_period;
	}

	void setIdrPeriod(uint16_t idrPeriod) {
		idr_period = idrPeriod;
	}

	boolean isDeleted() const {
		return deleted;
	}

	void setDeleted(boolean deleted) {
		this->deleted = deleted;
	}

	boolean isPacketPaddingEnabled() const {
		return packet_padding_enabled;
	}

	void setPacketPaddingEnabled(boolean packetPaddingEnabled) {
		packet_padding_enabled = packetPaddingEnabled;
	}

	boolean isFecEnabled() const {
		return fec_enabled;
	}

	void setFecEnabled(boolean fecEnabled) {
		fec_enabled = fecEnabled;
	}

	boolean isMtuDiscoveryEnabled() const {
		return mtu_discovery_enabled;
	}

	void setMtuDiscoveryEnabled(boolean mtuDiscoveryEnabled) {
		mtu_discovery_enabled = mtuDiscoveryEnabled;
	}

	boolean isScsEnabled() const {
		return scs_enabled;
	}

	void setScsEnabled(boolean scsEnabled) {
		scs_enabled = scsEnabled;
	}

	uint16_t getTrialMtu() const {
		return trial_mtu;
	}

	void setTrialMtu(uint16_t trialMtu) {
		trial_mtu = trialMtu;
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
};

#endif /* QUEUE_HPVTPACKETSET_H_ */
