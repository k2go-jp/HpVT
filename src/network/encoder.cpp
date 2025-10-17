#define FILE_NUMBER "N104"

#include "../camera/camera.h"

#include "context_s.h"

extern HPVT_Context *g_context;

static boolean _HPVT_check_camera_id(HPVT_CAMERA_ID camera_id) {

	if (camera_id == HPVT_CAMERA_ID_0) {
		return true;
	} else if (camera_id == HPVT_CAMERA_ID_1) {
		return true;
	} else {
		return false;
	}
}
int HPVT_start_thread_h264_video_encoding(HPVT_Config *config) {

	HPVT_CAMERA_ID tmp_camera_id;
	tmp_camera_id = config->video_transmitter.camera_id;
	HPVT_activate_standard_camera_capture();

	while (g_context->termination == false) {
		HPVT_start_standard_camera_encoding(tmp_camera_id);
		sleep(1);
	}

	return 0;
}
