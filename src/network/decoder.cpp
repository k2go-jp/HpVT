#define FILE_NUMBER "N103"

#include "../preview/h264_decoder.hpp"
#include "queue/queue.h"
#include "context_s.h"
#include "network.h"

extern HPVT_Context *g_context;

int HPVT_start_thread_h264_video_decoding(HPVT_Config *config) {

	unsigned char data_buf[HPVT_FRAME_LENGTH_MAXIMUM];
	uint32_t data_len = 0;
	HPVT_Queue_FRAME_TYPE frame_type;
	boolean ret_get_frame = false;
	boolean flag_setup = false;

	H264Decoder *decoder = new H264Decoder();

	while (true) {

		data_len = 0;

		while (true) {

			ret_get_frame = HPVT_get_video_frame_data((unsigned char *) data_buf, &data_len, &frame_type);

			if (g_context->connection.flag_reset_resolution == true) {
				g_context->connection.flag_reset_resolution = false;
				HPVT_logging(LOG_LEVEL_NOTICE, "DEC reset resolution");
				flag_setup = false;
			}

			if (ret_get_frame == true && data_len > 0) {

				if (flag_setup == false && frame_type == HPVT_Queue_FRAME_TYPE_I) {

					decoder->Stop();
					decoder->SetResolution(g_context->connection.current_resolution_width, g_context->connection.current_resolution_height);
					usleep(10000);
					decoder->Start();
					flag_setup = true;
				}

				break;
			}

			usleep(1000);
		}

		if (flag_setup == false) {
			continue;
		}

		decoder->DecodeBuffer(data_buf, data_len);

		if (g_context->connection.internal_info.seqno != HPVT_Queue_FRAME_SEQNO_INVALID) {

			if (g_context->connection.current_framerate != 0) {
				int tmp_diff_frame_count;
				int tmp_diff_time;

				tmp_diff_frame_count = HPVT_compare_frame_sequence_number( //
						g_context->connection.internal_info.seqno, g_context->connection.processing_seqno);

				tmp_diff_time = (tmp_diff_frame_count / (int) g_context->connection.current_framerate) * 1000;
				g_context->connection.frame_generated_time = g_context->connection.internal_info.captured_time + tmp_diff_time;
			}
		}

		HPVT_logging(LOG_LEVEL_INFO, "put VFrame data to decoder");
	}

	return 0;
}
