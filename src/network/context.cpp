#define FILE_NUMBER "N101"

#include "../logger/logger.h"
#include "../config/config_s.h"
#include "network.h"
#include "context.h"

extern HPVT_Context *g_context;

void HPVT_Context_initialize(HPVT_Context *context) {

	if (context == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	memset(context, 0, sizeof(HPVT_Context));

	if (pthread_mutex_init(&context->settings.settings_locked, 0)) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (pthread_mutex_init(&context->connection.status_locked, 0)) {
		HPVT_log_UNEXPECTED_ERROR();
	}
}
void HPVT_Context_destroy(HPVT_Context *context) {

	if (pthread_mutex_destroy(&context->settings.settings_locked)) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (pthread_mutex_destroy(&context->connection.status_locked)) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	free(context);
}
void HPVT_Context_setup(HPVT_Context *context, HPVT_Config *config) {

	context->settings.system.type = config->connection.type;
	context->settings.system.connection_mode = config->connection.connection_mode;

	if (config->connection.type == HPVT_Config_TYPE_TRANSMITTER) {

		context->settings.video.packet_size_determination_mode = config->video_transmitter.packet_size_determination_mode;
		context->settings.video.mtu_size = config->video_transmitter.mtu_size;
		context->settings.video.path_mtu_discovery_min_size = config->video_transmitter.path_mtu_discovery_min_size;
		context->settings.video.path_mtu_discovery_max_size = config->video_transmitter.path_mtu_discovery_max_size;

		context->settings.video.timeout_threshold = config->connection.server.timeout;
		context->settings.video.timeout_retry_wait = config->connection.server.retry_wait;

		if (config->video_transmitter.pace_control == HPVT_Config_FUNCTION_ON) {
			context->settings.video.pace_control = true;
		}
		else {
			context->settings.video.pace_control = false;
		}

		if (config->video_transmitter.feedback_control == HPVT_Config_FUNCTION_ON) {
			context->settings.video.feedback_control = true;
		}
		else {
			context->settings.video.feedback_control = false;
		}

		if (config->video_adaptive_control == HPVT_Config_FUNCTION_ON) {
			context->settings.video.adaptive_control = true;
		}
		else {
			context->settings.video.adaptive_control = false;
		}

		context->settings.video.adaptive_control_param.measurement_interval = config->video_transmitter.adaptive_control_param.measurement_interval;
		context->settings.video.adaptive_control_param.limit_amount = config->video_transmitter.adaptive_control_param.limit_amount;
		context->settings.video.adaptive_control_param.interval_threshold = config->video_transmitter.adaptive_control_param.interval_threshold;
		context->settings.video.adaptive_control_param.unreceived_rate_threshold = config->video_transmitter.adaptive_control_param.unreceived_rate_threshold;
		context->settings.video.adaptive_control_param.min_bitrate = config->video_transmitter.adaptive_control_param.min_bitrate * 1000;

		if (config->video_transmitter.fec_enabled == HPVT_Config_FUNCTION_ON) {
			context->settings.video.fec_enabled = true;
		}
		else {
			context->settings.video.fec_enabled = false;
		}
		context->settings.video.fec_level = config->video_transmitter.fec_level;

		context->settings.video.encode_framerate = config->video_transmitter.framerate;
		context->settings.video.bitrate = config->video_transmitter.bitrate * 1000;
		context->settings.video.i_frame_interval = config->video_transmitter.idr_period;

		context->settings.video.adaptive_control_param.max_bitrate = context->settings.video.bitrate;

		context->settings.video.profile = config->video_transmitter.profile;

		if (config->video_transmitter.preview_enabled == HPVT_Config_FUNCTION_ON) {
			context->settings.video.preview_enabled = true;
		}
		else {
			context->settings.video.preview_enabled = false;
		}

		context->settings.video.camera.framerate = (float) config->video_transmitter.camera.framerate / 1000.0;
		context->settings.video.camera.width = config->video_transmitter.camera.width;
		context->settings.video.camera.height = config->video_transmitter.camera.height;

		int camera_id;
		camera_id = config->video_transmitter.camera_id;
		context->camera_params[camera_id].width = config->video_transmitter.camera.width;
		context->camera_params[camera_id].height = config->video_transmitter.camera.height;
		context->camera_params[camera_id].framerate = (float) config->video_transmitter.camera.framerate / 1000.0;
		context->camera_params[camera_id].sharpness = (float) config->video_transmitter.camera.sharpness / 1000.0;
		context->camera_params[camera_id].contrast = (float) config->video_transmitter.camera.contrast / 1000.0;
		context->camera_params[camera_id].brightness = (float) config->video_transmitter.camera.brightness / 1000.0;
		context->camera_params[camera_id].saturation = (float) config->video_transmitter.camera.saturation / 1000.0;
		context->camera_params[camera_id].shutter_speed = config->video_transmitter.camera.shutter_speed;
		context->camera_params[camera_id].analog_gain = (float) config->video_transmitter.camera.analog_gain / 1000.0;
		context->camera_params[camera_id].exposure_mode = config->video_transmitter.camera.exposure_mode;
		context->camera_params[camera_id].exposure_compensation = (float) config->video_transmitter.camera.exposure_compensation / 1000.0;
		context->camera_params[camera_id].awb_mode = config->video_transmitter.camera.white_balance.mode;
		context->camera_params[camera_id].awb_red_gain = (float) (config->video_transmitter.camera.white_balance.red_gain) / 1000.0;
		context->camera_params[camera_id].awb_blue_gain = (float) (config->video_transmitter.camera.white_balance.blue_gain) / 1000.0;
		context->camera_params[camera_id].metering_mode = config->video_transmitter.camera.metering_mode;
		context->camera_params[camera_id].rotation = config->video_transmitter.camera.rotation;
		context->camera_params[camera_id].flip = config->video_transmitter.camera.flip;
	}
	else {
		context->settings.video.buffering_delay = config->video_receiver.buffering_delay;
	}

	context->settings.video.feedback_interval = config->video_transmitter.feedback_interval;

	if (config->preview.full_screen == HPVT_Config_FUNCTION_ON) {
		context->settings.video.preview.full_screen = true;
	}
	else {
		context->settings.video.preview.full_screen = false;
	}

	context->connection.reconnection_attempt_on = true;

	context->connection.path_mtu_discovery.try_maximum_size = config->video_transmitter.path_mtu_discovery_max_size;
	context->connection.path_mtu_discovery.receivable_size = config->video_transmitter.path_mtu_discovery_min_size;
	context->connection.state = HPVT_VIDEO_CONNECTION_STATE_DISCONNECTED;
}
boolean HPVT_Context_is_transmitter(HPVT_Context *context) {

	if (context->settings.system.type == HPVT_Config_TYPE_TRANSMITTER) {
		return true;
	}
	else {
		return false;
	}
}

boolean HPVT_Context_is_session_connected(HPVT_Context *context) {

	if (context->connection.state == HPVT_VIDEO_CONNECTION_STATE_CONNECTED) {
		return true;
	}
	else {
		return false;
	}
}
const char* HPVT_Context_get_connection_state_string(int state) {

	if (state == HPVT_VIDEO_CONNECTION_STATE_CLOSED) {
		return "CLOSED";
	}
	else if (state == HPVT_VIDEO_CONNECTION_STATE_DISCONNECTED) {
		return "DISCONNECTED";
	}
	else if (state == HPVT_VIDEO_CONNECTION_STATE_CONNECTING) {
		return "CONNECTING";
	}
	else if (state == HPVT_VIDEO_CONNECTION_STATE_WAITING) {
		return "WAITING";
	}
	else if (state == HPVT_VIDEO_CONNECTION_STATE_CONNECTED) {
		return "CONNECTED";
	}
	else {
		return "?";
	}
}
void HPVT_Context_ready_wait(HPVT_Context *context) {

	while (context->ready == 0) {

		usleep(100000);
	}
}

void HPVT_Context_activate(HPVT_Context *context) {

	HPVT_Lock(context->settings.settings_locked);

	context->ready = 1;

	HPVT_Unlock(context->settings.settings_locked);
}

void HPVT_Context_deactivate(HPVT_Context *context) {

	HPVT_Lock(context->settings.settings_locked);

	context->ready = 0;

	HPVT_Unlock(context->settings.settings_locked);
}

boolean HPVT_Context_is_ready(HPVT_Context *context) {

	boolean ret;

	HPVT_Lock(context->settings.settings_locked);

	if (context->ready == 1) {
		ret = true;
	}
	else {
		ret = false;
	}

	HPVT_Unlock(context->settings.settings_locked);

	return ret;
}
void HPVT_Context_update_encode_parameters(HPVT_Context *context, int width, int height, int framerate) {

	if (width != 0) {
		if (context->connection.current_resolution_width != (width * 8) || context->connection.current_resolution_height != (height * 8)) {

			if (context->connection.current_resolution_width != 0) {
				context->connection.flag_reset_resolution = true;
			}

			context->connection.current_resolution_width = width * 8;
			context->connection.current_resolution_height = height * 8;
		}
	}

	if (framerate != 0) {
		context->connection.current_framerate = framerate;
	}
}
void HPVT_Context_update_receivable_mtu_size(HPVT_Context *context, uint16_t mtu) {

	HPVT_Lock(context->settings.settings_locked);

	context->connection.path_mtu_discovery.receivable_size = mtu;

	HPVT_Unlock(context->settings.settings_locked);

	HPVT_logging(LOG_LEVEL_NOTICE, "Updated Path MTU (%d)", mtu);
}

