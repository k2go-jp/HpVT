#define FILE_NUMBER "Q231"

#include "queue.h"
#include "HPVTDecodeTimeManager.h"

HPVTDecodeTimeManager::HPVTDecodeTimeManager() {

	base_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;
	latest_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;
	next_decode_time = 0;
	fps = 0;
	idr_period = 0;
	frame_interval = 0;
	delay_time = 0;
	long_delay_flag = false;
}
HPVTDecodeTimeManager::~HPVTDecodeTimeManager() {

}
void HPVTDecodeTimeManager::reset() {
	base_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;
	latest_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;
	next_decode_time = 0;
	HPVT_logging(LOG_LEVEL_NOTICE, "DEC init decode time");
}
boolean HPVTDecodeTimeManager::updateFps(uint16_t fps_value, uint16_t idr_period_value) {
	if (fps != fps_value && fps_value != 0) {
		fps = fps_value;
		frame_interval = (1000000 / fps_value);
		idr_period = idr_period_value;
		HPVT_logging(LOG_LEVEL_NOTICE, "DEC set fps %d, interval=%d", fps_value, frame_interval);
		return true;
	}
	return false;
}
void HPVTDecodeTimeManager::updateNextDecodeTime(uint32_t present, HPVT_Queue_FRAME_SEQNO processing_seqno, uint16_t buffered_rate) {

	if (next_decode_time == 0) {
		if (fps == 0) {
			usleep(100000);
			return;
		}

	}

	int diff_frame_num = HPVT_compare_frame_sequence_number(base_seqno, processing_seqno);

	int32_t correct_time;

	if (diff_frame_num <= 2) {
		correct_time = 0;
	}
	else {
		if (buffered_rate < 30) {
			correct_time = frame_interval;
		}
		else if (buffered_rate >= 30 && buffered_rate < 50) {
			correct_time = frame_interval / 2;
		}
		else if (buffered_rate >= 50 && buffered_rate < 70) {
			correct_time = frame_interval / 3;
		}
		else if (buffered_rate >= 70 && buffered_rate < 80) {
			correct_time = frame_interval / 4;
		}
		else if (buffered_rate >= 80 && buffered_rate < 90) {
			correct_time = frame_interval / 8;
		}
		else if (buffered_rate >= 90 && buffered_rate < 110) {
			correct_time = 0;
		}
		else if (buffered_rate >= 110 && buffered_rate < 120) {
			correct_time = -(frame_interval / 8);
		}
		else if (buffered_rate >= 120 && buffered_rate < 130) {
			correct_time = -(frame_interval / 4);
		}
		else if (buffered_rate >= 130 && buffered_rate < 150) {
			correct_time = -(frame_interval / 3);
		}
		else if (buffered_rate >= 150 && buffered_rate < 250) {
			correct_time = -(frame_interval * 2 / 3);
		}
		else {
			correct_time = -(frame_interval * 3 / 4);
		}
	}

	next_decode_time = present + frame_interval + correct_time;

	if (next_decode_time > (UINT32_MAX - frame_interval)) {
		HPVT_logging(LOG_LEVEL_NOTICE, "DEC time rounded");
		next_decode_time = 1;
	}

	if (correct_time != 0) {
		HPVT_logging(LOG_LEVEL_INFO, "DEC time correction %zd,fps=%d,interval=%zd", correct_time, fps, frame_interval);
		base_seqno = processing_seqno;
	}
}
uint16_t HPVTDecodeTimeManager::getBufferedRate(HPVT_Queue_FRAME_SEQNO processing_seqno) {

	if (delay_time == 0) {
		return 0;
	}

	if (processing_seqno == HPVT_Queue_FRAME_SEQNO_INVALID) {
		HPVT_logging(LOG_LEVEL_NOTICE, "Frame seqno is invalid");
		return 0;
	}

	int diff = HPVT_compare_frame_sequence_number(processing_seqno, latest_seqno);

	uint16_t tmp_buffred_rate;

	if (diff < 0) {
		tmp_buffred_rate = 0;
	}
	else {
		tmp_buffred_rate = diff * frame_interval * 100 / (getDesiredDelayTime());
	}

	return tmp_buffred_rate;
}
uint32_t HPVTDecodeTimeManager::getDesiredDelayTime() {

	return (frame_interval + delay_time);
}
