#define FILE_NUMBER "Q232"

#include "HPVTLimitter.h"

HPVTLimitter::HPVTLimitter(uint32_t totalSize, uint32_t targetTime) {

	if (targetTime < 100) {
		return;
	}

	sent_size = 0;
	total_byte = totalSize;
	target_rate = totalSize / (targetTime / 100); // byte per 100 msec

	gettimeofday(&begin_time, NULL);
}
HPVTLimitter::~HPVTLimitter() {

}
uint32_t HPVTLimitter::getWaitTime(uint32_t packetSize) {

	if (total_byte == 0 || total_byte > 1000 * 1000) {
		return 0;
	}

	struct timeval tmp_timeval;
	gettimeofday(&tmp_timeval, NULL);

	struct timeval tmp_diff_time;
	timersub(&tmp_timeval, &begin_time, &tmp_diff_time);

	uint32_t tmp_elapsed_time;
	tmp_elapsed_time = (tmp_diff_time.tv_sec * 10 + tmp_diff_time.tv_usec / 100000);


	uint32_t wait_time;

	if (sent_size > (tmp_elapsed_time * target_rate)) {
		wait_time = ((sent_size / target_rate) - tmp_elapsed_time) * 10 * 1000;
	}
	else {

		wait_time = 0;
	}

	sent_size += packetSize;

	return wait_time;
}
