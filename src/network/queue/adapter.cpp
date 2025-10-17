#define FILE_NUMBER "Q204"

#include <sys/time.h>

#include "../context.h"
#include "../../camera/camera.h"
#include "../../config/config.h"
#include "HPVTTrafficRecoder.h"
#include "queue.h"

extern HPVT_Context *g_context;

static HPVTTrafficRecoder *recoder = NULL;

#define PRESET_TABLE_SIZE         18

#define PRESET_50K        (50 * 1000)
#define PRESET_100K      (100 * 1000)
#define PRESET_200K      (200 * 1000)
#define PRESET_400K      (400 * 1000)
#define PRESET_600K      (600 * 1000)
#define PRESET_800K      (800 * 1000)
#define PRESET_1M       (1000 * 1000)
#define PRESET_1_2M     (1200 * 1000)
#define PRESET_1_5M     (1500 * 1000)
#define PRESET_2M       (2000 * 1000)
#define PRESET_2_5M     (2500 * 1000)
#define PRESET_3M       (3000 * 1000)
#define PRESET_3_5M     (3500 * 1000)
#define PRESET_4M       (4000 * 1000)
#define PRESET_5M       (5000 * 1000)
#define PRESET_6M       (6000 * 1000)
#define PRESET_8M       (8000 * 1000)
#define PRESET_10M     (10000 * 1000)

static uint32_t preset_bitrate_table[PRESET_TABLE_SIZE] = {
		PRESET_50K, PRESET_100K, PRESET_200K, PRESET_400K, //
		PRESET_600K, PRESET_800K, PRESET_1M, PRESET_1_2M, //
		PRESET_1_5M, PRESET_2M, PRESET_2_5M, PRESET_3M, PRESET_3_5M, //
		PRESET_4M, PRESET_5M, PRESET_6M, PRESET_8M, PRESET_10M };

static int _HPVT_get_min_candidate_index(int current_value, int min_value, uint32_t *table) {

	int limit_amount = g_context->settings.video.adaptive_control_param.limit_amount;

	if (limit_amount == 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return 0;
	}

	int i;
	int tmp_value;
	for (i = 0; i < PRESET_TABLE_SIZE; i++) {
		if (table[i] < min_value) {
			continue;
		}

		if (table[i] < (current_value / limit_amount)) {
			continue;
		}

		tmp_value = i;
		break;
	}

	if (i == PRESET_TABLE_SIZE) {
		return 0;
	}

	return i;
}
static int _HPVT_get_max_candidate_index(int current_value, int max_value, uint32_t *table) {

	int limit_amount = g_context->settings.video.adaptive_control_param.limit_amount;

	int i;
	int tmp_value = 0;
	for (i = 0; i < PRESET_TABLE_SIZE; i++) {

		if (table[i] > max_value) {
			break;
		}

		if (table[i] > (current_value * limit_amount)) {
			break;
		}
		tmp_value = i;
	}

	if (i == PRESET_TABLE_SIZE) {
		return PRESET_TABLE_SIZE - 1;
	}

	return tmp_value;
}
static int _HPVT_get_lower(int current_value, int min_value, uint32_t *table) {

	int limit_amount = g_context->settings.video.adaptive_control_param.limit_amount;

	if (limit_amount == 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return table[0];
	}

	int i;
	int tmp_value;
	for (i = 0; i < PRESET_TABLE_SIZE; i++) {
		if (table[i] < min_value) {
			return min_value;
		}

		if (table[i] < (current_value / limit_amount)) {
			continue;
		}

		tmp_value = i;
		break;
	}

	if (i == PRESET_TABLE_SIZE) {
		tmp_value = 0;
	}

	return table[tmp_value];
}
static int _HPVT_get_upper(int current_value, int max_value, uint32_t *table) {

	int limit_amount = g_context->settings.video.adaptive_control_param.limit_amount;

	int i;
	int tmp_value = 0;
	for (i = 0; i < PRESET_TABLE_SIZE; i++) {

		if (table[i] > max_value) {
			return max_value;
		}

		if (table[i] > (current_value * limit_amount)) {
			break;
		}
		tmp_value = i;
	}

	if (i == PRESET_TABLE_SIZE) {
		tmp_value = PRESET_TABLE_SIZE - 1;
	}

	return table[tmp_value];
}
static uint32_t _HPVT_get_optimal_value(uint32_t recv_throughput, int current_value, int min_value, int max_value, uint32_t *table) {

	int ret_value = 0;

	int min_idx;
	int max_idx;

	min_idx = _HPVT_get_min_candidate_index(current_value, min_value, table);
	max_idx = _HPVT_get_max_candidate_index(current_value, max_value, table);

	int lower_value;
	int upper_value;

	lower_value = _HPVT_get_lower(current_value, min_value, table);
	upper_value = _HPVT_get_upper(current_value, max_value, table);

	if (recv_throughput < table[1]) {
		ret_value = table[0];
	}
	else if (recv_throughput < table[2]) {
		ret_value = table[1];
	}
	else if (recv_throughput < table[3]) {
		ret_value = table[2];
	}
	else if (recv_throughput < table[4]) {
		ret_value = table[3];
	}
	else if (recv_throughput < table[5]) {
		ret_value = table[4];
	}
	else if (recv_throughput < table[6]) {
		ret_value = table[5];
	}
	else if (recv_throughput < table[7]) {
		ret_value = table[6];
	}
	else if (recv_throughput < table[8]) {
		ret_value = table[7];
	}
	else if (recv_throughput < table[9]) {
		ret_value = table[8];
	}
	else if (recv_throughput < table[10]) {
		ret_value = table[9];
	}
	else if (recv_throughput < table[11]) {
		ret_value = table[10];
	}
	else if (recv_throughput < table[12]) {
		ret_value = table[11];
	}
	else if (recv_throughput < table[13]) {
		ret_value = table[12];
	}
	else if (recv_throughput < table[14]) {
		ret_value = table[13];
	}
	else if (recv_throughput < table[15]) {
		ret_value = table[14];
	}
	else if (recv_throughput < table[16]) {
		ret_value = table[15];
	}
	else if (recv_throughput < table[17]) {
		ret_value = table[16];
	}
	else {
		ret_value = table[17];
	}

	if (ret_value < lower_value) {
		ret_value = lower_value;
	}
	else if (ret_value > upper_value) {
		ret_value = upper_value;
	}

	HPVT_logging(LOG_LEVEL_NOTICE, "ADC recv_throughput=%lu,FEC=%d,lower=%d,upper=%d,ret_value=%d", //
			recv_throughput, g_context->settings.video.fec_level, lower_value, upper_value, ret_value);

	return ret_value;

}
static uint32_t _HPVT_get_low_throughput_value(uint32_t bitrate, uint32_t *table) {

	int i;
	for (i = PRESET_TABLE_SIZE - 1; i >= 0; i--) {

		if (bitrate > table[i]) {
			break;
		}

	}

	if (i < 0) {
		i = 0;
	}

	return table[i];
}
static uint32_t _HPVT_get_high_throughput_value(uint32_t bitrate, uint32_t *table) {

	int i;
	for (i = 0; i < PRESET_TABLE_SIZE; i++) {

		if (bitrate < table[i]) {
			break;
		}

	}

	if (i == PRESET_TABLE_SIZE) {
		i = PRESET_TABLE_SIZE - 1;
	}

	return table[i];
}
static int64_t _HPVT_get_unreceived_rate(uint64_t sent, uint64_t received) {

	uint64_t tmp_value;
	int64_t ret_value;

	if (sent == 0) {
		return 0;
	}

	if (sent < received) {
		tmp_value = ((received - sent) * 1000 / sent);
		ret_value = (-1) * (int64_t) tmp_value;
	}
	else {
		tmp_value = ((sent - received) * 1000 / sent);
		ret_value = tmp_value;
	}

	return ret_value;
}

static void _HPVT_adaptive_control_default(void) {

	int tmp_duration;

	uint64_t average_recv_thr;
	uint64_t average_send_thr;
	int64_t unreceived_rate;

	tmp_duration = g_context->settings.video.adaptive_control_param.measurement_interval;
	average_recv_thr = recoder->getAverageRecvThroughput(tmp_duration);
	average_send_thr = recoder->getAverageSendThroughput(tmp_duration);

	unreceived_rate = _HPVT_get_unreceived_rate(average_send_thr, average_recv_thr);

	HPVT_logging(LOG_LEVEL_NOTICE, "ADC Params bps=max %d min %d,interval=%d,rate=%d", //
			g_context->settings.video.adaptive_control_param.max_bitrate, //
			g_context->settings.video.adaptive_control_param.min_bitrate,//
			g_context->settings.video.adaptive_control_param.interval_threshold,//
			g_context->settings.video.adaptive_control_param.unreceived_rate_threshold);

	HPVT_logging(LOG_LEVEL_NOTICE, "ADC %d sec,send_th=%llu,recv_th=%llu,unrecv_rate=%lld", tmp_duration, average_send_thr, average_recv_thr, unreceived_rate);
	HPVT_logging(LOG_LEVEL_NOTICE, "ADC current settings bitrate=%d", g_context->settings.video.bitrate);

	uint32_t next_bitrate = 0;

	uint64_t average_recv_thr_long;
	uint64_t average_send_thr_long;
	int64_t unreceived_rate_long;

	next_bitrate = _HPVT_get_optimal_value((uint32_t) average_recv_thr, //
			g_context->settings.video.bitrate, g_context->settings.video.adaptive_control_param.min_bitrate, //
			g_context->settings.video.adaptive_control_param.max_bitrate, preset_bitrate_table);

	tmp_duration = g_context->settings.video.adaptive_control_param.interval_threshold;

	average_send_thr_long = recoder->getAverageSendThroughput(tmp_duration);
	average_recv_thr_long = recoder->getAverageRecvThroughput(tmp_duration);
	unreceived_rate_long = _HPVT_get_unreceived_rate(average_send_thr_long, average_recv_thr_long);

	HPVT_logging(LOG_LEVEL_NOTICE, "ADC %d sec,send_th=%llu,recv_th=%llu,unrecv_rate=%lld", //
			tmp_duration, average_send_thr_long, average_recv_thr_long, unreceived_rate_long);

	if (next_bitrate == g_context->settings.video.adaptive_control_param.max_bitrate) {

		HPVT_logging(LOG_LEVEL_NOTICE, "ADC next=%lu,maximum", next_bitrate);
		//
	}
	else {

		if (next_bitrate < g_context->settings.video.bitrate) {
			HPVT_logging(LOG_LEVEL_NOTICE, "ADC next=%lu,down", next_bitrate);
		}
		else if (next_bitrate == g_context->settings.video.bitrate) {

			if ((unreceived_rate_long > (-20)) && unreceived_rate_long <= g_context->settings.video.adaptive_control_param.unreceived_rate_threshold) {

				next_bitrate = _HPVT_get_high_throughput_value(next_bitrate, preset_bitrate_table);
				if (next_bitrate > g_context->settings.video.adaptive_control_param.max_bitrate) {
					next_bitrate = g_context->settings.video.adaptive_control_param.max_bitrate;
				}
				HPVT_logging(LOG_LEVEL_NOTICE, "ADC next bitrate=%lu,up", next_bitrate);
			}
			else {

				if (unreceived_rate_long > 20) {
					next_bitrate = _HPVT_get_low_throughput_value(next_bitrate, preset_bitrate_table);

					if (next_bitrate < g_context->settings.video.adaptive_control_param.min_bitrate) {
						next_bitrate = g_context->settings.video.adaptive_control_param.min_bitrate;
					}

					HPVT_logging(LOG_LEVEL_NOTICE, "ADC next=%lu,down", next_bitrate);

				}
				else {
					HPVT_logging(LOG_LEVEL_NOTICE, "ADC next=%lu,maintain", next_bitrate);
				}
			}
		}
		else {
			if (unreceived_rate_long <= g_context->settings.video.adaptive_control_param.unreceived_rate_threshold) {
				HPVT_logging(LOG_LEVEL_NOTICE, "ADC next=%lu,up OK", next_bitrate);
			}
			else {
				next_bitrate = 0;

				HPVT_logging(LOG_LEVEL_NOTICE, "ADC next=%lu,up NG maintain", next_bitrate);
			}

		}
	}
	HPVT_logging(LOG_LEVEL_NOTICE, "#TRACE-ac,%d,%d,%d,%d,%d,%llu,%llu,%lld,%llu,%llu,%lld", //
			g_context->settings.video.bitrate,//
			g_context->settings.video.adaptive_control_param.max_bitrate,//
			g_context->settings.video.adaptive_control_param.min_bitrate,//
			g_context->settings.video.adaptive_control_param.interval_threshold,//
			g_context->settings.video.adaptive_control_param.unreceived_rate_threshold,//
			average_send_thr, average_recv_thr, unreceived_rate, //
			average_send_thr_long, average_recv_thr_long, unreceived_rate_long //
			);

	if (next_bitrate == 0 || next_bitrate == g_context->settings.video.bitrate) {
		HPVT_logging(LOG_LEVEL_NOTICE, "ADC next=%lu,no change", next_bitrate);
		return;
	}
	HpVT_libcam_set_video_bitrate(next_bitrate);
	HPVT_logging(LOG_LEVEL_NOTICE, "ADC done");
}

void HPVT_start_thread_adaptive_controller(void) {

	struct timeval tv_now;
	struct timeval tv_last_changed, tv_diff_changed;

	HPVT_Context_ready_wait(g_context);

	timerclear(&tv_now);
	timerclear(&tv_last_changed);
	gettimeofday(&tv_last_changed, NULL);

	recoder = new HPVTTrafficRecoder();

	while (true) {

		usleep(499900);

		//
		gettimeofday(&tv_now, NULL);
		timersub(&tv_now, &tv_last_changed, &tv_diff_changed);

		if (tv_diff_changed.tv_sec >= g_context->settings.video.adaptive_control_param.measurement_interval) {
			gettimeofday(&tv_last_changed, NULL);

			if (g_context->settings.video.adaptive_control == true) {
				HPVT_logging(LOG_LEVEL_NOTICE, "ADC call default function");
				_HPVT_adaptive_control_default();
			}
			else {
				HPVT_logging(LOG_LEVEL_INFO, "ADC adaptive control is off");
			}
		}
	}
}
void HPVT_add_feeedback_data(uint32_t plr, uint64_t recv_throughput, uint64_t send_throughput) {

	if (recoder == NULL) {
		return;
	}

	recoder->addLatestPLR(plr);
	recoder->addLatestRecvThroughput(recv_throughput);
	recoder->addLatestSendThroughput(send_throughput);
}

