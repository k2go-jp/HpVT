#ifndef NETWORK_QUEUE_HPVTDECODETIMEMANAGER_H_
#define NETWORK_QUEUE_HPVTDECODETIMEMANAGER_H_

#include "../../common_s.h"
#include "../../logger/logger.h"

class HPVTDecodeTimeManager {

	HPVT_Queue_FRAME_SEQNO base_seqno;
	HPVT_Queue_FRAME_SEQNO latest_seqno;

	uint32_t next_decode_time;
	uint16_t fps;
	uint16_t idr_period;
	uint32_t frame_interval;

	uint32_t delay_time;
	boolean long_delay_flag;

public:
	HPVTDecodeTimeManager();
	~HPVTDecodeTimeManager();

	void reset();
	int compare(HPVT_Queue_FRAME_SEQNO);
	boolean updateFps(uint16_t,uint16_t);

	void updateNextDecodeTime(uint32_t,HPVT_Queue_FRAME_SEQNO, uint16_t);

	uint16_t getBufferedRate(HPVT_Queue_FRAME_SEQNO);
	uint32_t getDesiredDelayTime();

	uint32_t getNextDecodeTime() const {
		return next_decode_time;
	}

	void setNextDecodeTime(uint32_t nextDecodeTime) {
		next_decode_time = nextDecodeTime;
	}

	HPVT_Queue_FRAME_SEQNO getBaseSeqno() const {
		return base_seqno;
	}

	void setBaseSeqno(HPVT_Queue_FRAME_SEQNO baseSeqno) {
		base_seqno = baseSeqno;
	}

	uint32_t getDelayTime() const {
		return delay_time;
	}

	void setDelayTime(uint32_t delayTime) {
		delay_time = delayTime;
	}

	uint16_t getFps() const {
		return fps;
	}

	uint32_t getFrameInterval() const {
		return frame_interval;
	}

	HPVT_Queue_FRAME_SEQNO getLatestSeqno() const {
		return latest_seqno;
	}

	void setLatestSeqno(HPVT_Queue_FRAME_SEQNO latestSeqno) {
		latest_seqno = latestSeqno;
	}

	boolean isLongDelayFlag() const {
		return long_delay_flag;
	}

	void setLongDelayFlag(boolean longDelayFlag) {
		long_delay_flag = longDelayFlag;
	}

	uint16_t getIdrPeriod() const {
		return idr_period;
	}

	void setIdrPeriod(uint16_t idrPeriod) {
		idr_period = idrPeriod;
	}
};

#endif /* NETWORK_QUEUE_HPVTDECODETIMEMANAGER_H_ */
