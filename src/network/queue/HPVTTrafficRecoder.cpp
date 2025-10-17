#define FILE_NUMBER "Q233"

#include "HPVTTrafficRecoder.h"

HPVTTrafficRecoder::HPVTTrafficRecoder() {

	memset(feedback_recv_plr, 0, sizeof(feedback_recv_plr));
	feedback_recv_plr_index = 0;
	memset(feedback_recv_throughput_array, 0, sizeof(feedback_recv_throughput_array));
	feedback_recv_throughput_index = 0;
	memset(send_throughput_array, 0, sizeof(send_throughput_array));
	send_throughput_index = 0;
}
HPVTTrafficRecoder::~HPVTTrafficRecoder() {

}
void HPVTTrafficRecoder::addLatestPLR(uint32_t plr) {

	feedback_recv_plr[feedback_recv_plr_index] = plr;

	if (feedback_recv_plr_index == (TRAFFIC_RECODER_ARRAY_SIZE - 1)) {
		feedback_recv_plr_index = 0;
	}
	else {
		feedback_recv_plr_index++;
	}
}
void HPVTTrafficRecoder::addLatestRecvThroughput(uint64_t throughput) {

	feedback_recv_throughput_array[feedback_recv_throughput_index] = throughput;

	if (feedback_recv_throughput_index == (TRAFFIC_RECODER_ARRAY_SIZE - 1)) {
		feedback_recv_throughput_index = 0;
	}
	else {
		feedback_recv_throughput_index++;
	}
}
static int _HPVT_get_prev_index(int num) {

	if (num == 0) {
		return (TRAFFIC_RECODER_ARRAY_SIZE - 1);
	}

	return (num - 1);
}
uint32_t HPVTTrafficRecoder::getAveragePLR(int second) {

	if (second < 0 || second > TRAFFIC_RECODER_ARRAY_SIZE) {
		return 0;
	}

	int tmp_index;
	int i;
	uint32_t tmp_plr;

	tmp_plr = 0;
	tmp_index = _HPVT_get_prev_index(feedback_recv_plr_index);
	tmp_index = _HPVT_get_prev_index(tmp_index);

	for (i = 0; i < second; i++) {

		tmp_plr += feedback_recv_plr[tmp_index];
		tmp_index = _HPVT_get_prev_index(tmp_index);
	}
	tmp_plr /= second;

	return tmp_plr;
}
uint64_t HPVTTrafficRecoder::getAverageRecvThroughput(int second) {

	if (second <= 0 || second > TRAFFIC_RECODER_ARRAY_SIZE) {
		return 0;
	}

	int tmp_index;
	int i;
	uint64_t tmp_throughput;

	tmp_throughput = 0;
	tmp_index = feedback_recv_throughput_index;
	tmp_index = _HPVT_get_prev_index(tmp_index);

	for (i = 0; i < second; i++) {
		tmp_throughput += feedback_recv_throughput_array[tmp_index];
		tmp_index = _HPVT_get_prev_index(tmp_index);
	}
	tmp_throughput /= second;


	return tmp_throughput;
}
void HPVTTrafficRecoder::clearPLR() {

	memset(feedback_recv_plr, 0, sizeof(feedback_recv_plr));
	feedback_recv_plr_index = 0;
}
void HPVTTrafficRecoder::addLatestSendThroughput(uint32_t rate) {

	send_throughput_array[send_throughput_index] = rate;

	if (send_throughput_index == (TRAFFIC_RECODER_ARRAY_SIZE - 1)) {
		send_throughput_index = 0;
	}
	else {
		send_throughput_index++;
	}
}
uint32_t HPVTTrafficRecoder::getAverageSendThroughput(int second) {


	if (second < 0 || second > TRAFFIC_RECODER_ARRAY_SIZE) {
		return 0;
	}

	int tmp_index;
	int i;
	uint32_t tmp_rate;

	tmp_rate = 0;
	tmp_index = send_throughput_index;
	tmp_index = _HPVT_get_prev_index(tmp_index);

	for (i = 0; i < second; i++) {
		tmp_rate += send_throughput_array[tmp_index];
		tmp_index = _HPVT_get_prev_index(tmp_index);
	}
	tmp_rate /= second;

	return tmp_rate;
}
void HPVTTrafficRecoder::clearThroughput() {
	memset(feedback_recv_throughput_array, 0, sizeof(feedback_recv_throughput_array));
	feedback_recv_throughput_index = 0;
}
