#define FILE_NUMBER "C101"

#include <arpa/inet.h>
#include <ctype.h>
#include <sys/stat.h>

#include "config.h"

/* ---------------------------------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------------------------------- */

static inline void _HPVT_Config_initialize(HPVT_Config * object) {
	memset(object, 0, sizeof(HPVT_Config));

	object->log.facility = -1;
	object->log.level = -1;
	object->connection.server.timeout = HPVT_Config_VALUE_NOT_SET;
	object->video_transmitter.camera.white_balance.mode = HPVT_Config_VALUE_NOT_SET;
	object->video_transmitter.camera.white_balance.red_gain = HPVT_Config_VALUE_NOT_SET;
	object->video_transmitter.camera.white_balance.blue_gain = HPVT_Config_VALUE_NOT_SET;
	object->video_transmitter.camera.sharpness = HPVT_Config_VALUE_NOT_SET;
	object->video_transmitter.camera.brightness = HPVT_Config_VALUE_NOT_SET;
	object->video_transmitter.camera.contrast = HPVT_Config_VALUE_NOT_SET;
	object->video_transmitter.camera.saturation = HPVT_Config_VALUE_NOT_SET;
	object->video_transmitter.camera.analog_gain = HPVT_Config_VALUE_NOT_SET;
	object->video_transmitter.camera.exposure_mode = HPVT_Config_VALUE_NOT_SET;
	object->video_transmitter.camera.exposure_compensation = HPVT_Config_VALUE_NOT_SET;
	object->video_transmitter.camera.metering_mode = HPVT_Config_VALUE_NOT_SET;
	object->video_transmitter.camera.rotation = HPVT_Config_VALUE_NOT_SET;
	object->video_transmitter.camera.flip = HPVT_Config_VALUE_NOT_SET;
}
static inline void _HPVT_Config_dispose(HPVT_Config * object) {
	if (object->config_file != NULL) {
		free(object->config_file);
	}

	memset(object, 0, sizeof(HPVT_Config));
}

/* ---------------------------------------------------------------------------------------------------- */
void HPVT_Config_initialize(HPVT_Config * object) {
	if (object == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	_HPVT_Config_initialize(object);
}
void HPVT_Config_dispose(HPVT_Config * object) {
	if (object == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	_HPVT_Config_dispose(object);
}
HPVT_Config * HPVT_Config_create(void) {
	HPVT_Config * tmp_object;

	if ((tmp_object = (HPVT_Config *) malloc(sizeof(HPVT_Config))) == NULL) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return NULL;
	}

	_HPVT_Config_initialize(tmp_object);

	return tmp_object;
}
void HPVT_Config_delete(HPVT_Config * object) {
	if (object == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	_HPVT_Config_dispose(object);
	free(object);
}
static const char * _HPVT_Config_logging_convert_name_on_off(uint8_t value) {

	if (value == HPVT_Config_FUNCTION_ON) {
		return "ON";
	}
	else if (value == HPVT_Config_FUNCTION_OFF) {
		return "OFF";
	}
	else {
		return "?";
	}

}
static const char * _HPVT_Config_logging_convert_name_type(uint8_t value) {

	if (value == HPVT_Config_TYPE_TRANSMITTER) {
		return "Transmitter";
	}
	else if (value == HPVT_Config_TYPE_RECEIVER) {
		return "Receiver";
	}
	else {
		return "?";
	}

}
static const char * _HPVT_Config_logging_convert_name_packet_size_determination_mode(uint8_t value) {

	if (value == HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_STANDARD) {
		return "Standard";
	}
	else if (value == HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_AUTO_DISCOVERY) {
		return "Auto Discovery";
	}
	else if (value == HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_FIXED) {
		return "Fixed Size";
	}
	else {
		return "?";
	}

}
static const char * _HPVT_Config_logging_convert_name_connection_mode(uint8_t value) {

	if (value == HPVT_Config_CONNECTION_MODE_INIT_TRANSMITTER) {
		return "InitTransmitter";
	}
	else if (value == HPVT_Config_CONNECTION_MODE_INIT_RECEIVER) {
		return "InitReceiver";
	}
	else if (value == HPVT_Config_CONNECTION_MODE_CONNECTION_LESS) {
		return "Connectionless";
	}
	else {
		return "?";
	}

}
static const char * _HPVT_Config_logging_convert_name_prifile_name(uint8_t value) {

	if (value == HPVT_Config_VIDEO_PROFILE_BASELINE) {
		return "Baseline";
	}
	else if (value == HPVT_Config_VIDEO_PROFILE_MAIN) {
		return "Main";
	}
	else if (value == HPVT_Config_VIDEO_PROFILE_HIGH) {
		return "High";
	}
	else {
		return "?";
	}

}
static const char * HPVT_Config_logging_convert_name_flip(uint8_t value) {

	if (value == HPVT_Config_CAMERA_FLIP_NONE) {
		return "Off";
	}
	else if (value == HPVT_Config_CAMERA_FLIP_VERTICAL) {
		return "Vertical";
	}
	else if (value == HPVT_Config_CAMERA_FLIP_HORIZONTAL) {
		return "Horizontal";
	}
	else if (value == HPVT_Config_CAMERA_FLIP_BOTH) {
		return "Both";
	}
	else {
		return "?";
	}

}
static const char * HPVT_Config_logging_convert_name_whitebalance_mode(uint8_t value) {

	if (value == HPVT_Config_CAMERA_WHITE_BALANCE_MODE_AUTO || value == 0) {
		return "Auto";
	}
	else if (value == HPVT_Config_CAMERA_WHITE_BALANCE_MODE_INCANDESCENT) {
		return "Incandescent";
	}
	else if (value == HPVT_Config_CAMERA_WHITE_BALANCE_MODE_TUNGSTEN) {
		return "Tungsten";
	}
	else if (value == HPVT_Config_CAMERA_WHITE_BALANCE_MODE_FLUORESCENT) {
		return "Fluorescent";
	}
	else if (value == HPVT_Config_CAMERA_WHITE_BALANCE_MODE_INDOOR) {
		return "Indoor";
	}
	else if (value == HPVT_Config_CAMERA_WHITE_BALANCE_MODE_DAYLIGHT) {
		return "Daylight";
	}
	else if (value == HPVT_Config_CAMERA_WHITE_BALANCE_MODE_CLOUDY) {
		return "Cloudy";
	}
	else if (value == HPVT_Config_CAMERA_WHITE_BALANCE_MODE_CUSTOM) {
		return "Custom";
	}
	else {
		return "-";
	}

}
static const char * HPVT_Config_logging_convert_name_exposure_mode(uint8_t value) {

	if (value == HPVT_Config_CAMERA_EXPOSURE_MODE_NORMAL) {
		return "Normal";
	}
	else if (value == HPVT_Config_CAMERA_EXPOSURE_MODE_SHORT) {
		return "Short";
	}
	else if (value == HPVT_Config_CAMERA_EXPOSURE_MODE_LONG) {
		return "Long";
	}
	else {
		return "-";
	}

}
static const char * HPVT_Config_logging_convert_name_metering_mode(uint8_t value) {

	if (value == HPVT_Config_CAMERA_METERING_MODE_CENTRE) {
		return "Centre";
	}
	else if (value == HPVT_Config_CAMERA_METERING_MODE_SPOT) {
		return "Spot";
	}
	else if (value == HPVT_Config_CAMERA_METERING_MODE_MATRIX) {
		return "Matrix";
	}
	else {
		return "-";
	}

}
static boolean _HPVT_check_number_format(const char * string) {

	int length;
	length = strlen(string);
	int i;

	for (i = 0; i < length; i++) {

		if ((string[i] >= '0' && string[i] <= '9')) {

			//OK
		}
		else {
			return false;
		}

	}

	return true;
}
static boolean HPVT_strtouint32(const char * string, int base, uint32_t * value) {
	if (string == NULL) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (value == NULL) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	uint64_t tmp_number;

	errno = 0;
	tmp_number = strtoull(string, NULL, base);

	if (ERANGE == errno && tmp_number == 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (UINT32_MAX < tmp_number) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	*value = (uint32_t) tmp_number;

	return true;

}
static boolean HPVT_strtoint32(const char * string, int base, int32_t * value) {
	if (string == NULL) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (value == NULL) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	int32_t tmp_number;

	errno = 0;
	tmp_number = strtol(string, NULL, base);

	if (ERANGE == errno && tmp_number == 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	*value = tmp_number;

	return true;

}
static HPVT_Config_CONNECTION_MODE _HPVT_Config_convertToConnectionMode(const char * string) {
	if (strlen(string) < 1) {
		return HPVT_Config_CONNECTION_MODE_NONE;
	}

	if (strcasecmp(string, HPVT_Config_CONNECTION_MODE_INIT_TRANSMITTER_NAME) == 0) {
		return HPVT_Config_CONNECTION_MODE_INIT_TRANSMITTER;
	}
	else if (strcasecmp(string, HPVT_Config_CONNECTION_MODE_INIT_RECEIVER_NAME) == 0) {
		return HPVT_Config_CONNECTION_MODE_INIT_RECEIVER;
	}
	else if (strcasecmp(string, HPVT_Config_CONNECTION_MODE_CONNECTION_LESS_NAME) == 0) {
		return HPVT_Config_CONNECTION_MODE_CONNECTION_LESS;
	}

	return HPVT_Config_CONNECTION_MODE_NONE;
}
static HPVT_Config_TYPE _HPVT_Config_convertToType(const char * string) {
	if (strlen(string) < 1) {
		return HPVT_Config_TYPE_NONE;
	}

	if (strcasecmp(string, HPVT_Config_TYPE_TRANSMITTER_NAME) == 0) {
		return HPVT_Config_TYPE_TRANSMITTER;
	}
	else if (strcasecmp(string, HPVT_Config_TYPE_RECEIVER_NAME) == 0) {
		return HPVT_Config_TYPE_RECEIVER;
	}

	return HPVT_Config_TYPE_NONE;
}
static HPVT_Config_CAMERA_ID _HPVT_Config_convertToCameraID(const char * string) {
	if (strlen(string) < 1) {
		return HPVT_Config_CAMERA_ID_NONE;
	}

	if (strcasecmp(string, HPVT_Config_CAMERA_ID_0_NAME) == 0) {
		return HPVT_Config_CAMERA_ID_0;
	}
	else if (strcasecmp(string, HPVT_Config_CAMERA_ID_1_NAME) == 0) {
		return HPVT_Config_CAMERA_ID_1;
	}
	else {
		return -1;
	}

	return HPVT_Config_CAMERA_ID_NONE;
}
static HPVT_Config_PACKET_SIZE_DETERMINATION_MODE _HPVT_Config_convertToPacketSizeDeterminationMode(const char * string) {
	if (strlen(string) < 1) {
		return HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_NONE;
	}

	if (strcasecmp(string, HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_STANDARD_NAME) == 0) {
		return HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_STANDARD;
	}
	else if (strcasecmp(string, HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_FIXED_NAME) == 0) {
		return HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_FIXED;
	}
	else if (strcasecmp(string, HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_AUTO_DISCOVERY_NAME) == 0) {
		return HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_AUTO_DISCOVERY;
	}

	return HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_NONE;
}
static HPVT_Config_VIDEO_PROFILE_TYPE _HPVT_Config_convertToVideoProfile(const char * string) {
	if (strlen(string) < 1) {
		return HPVT_Config_VIDEO_PROFILE_NONE;
	}

	if (strcasecmp(string, HPVT_Config_VIDEO_PROFILE_BASELINE_NAME) == 0) {
		return HPVT_Config_VIDEO_PROFILE_BASELINE;
	}
	else if (strcasecmp(string, HPVT_Config_VIDEO_PROFILE_MAIN_NAME) == 0) {
		return HPVT_Config_VIDEO_PROFILE_MAIN;
	}
	else if (strcasecmp(string, HPVT_Config_VIDEO_PROFILE_HIGH_NAME) == 0) {
		return HPVT_Config_VIDEO_PROFILE_HIGH;
	}

	return HPVT_Config_VIDEO_PROFILE_NONE;
}
static HPVT_Config_CAMERA_FLIP _HPVT_Config_convertToCameraFlip(const char * string) {
	if (strlen(string) < 1) {
		return HPVT_Config_CAMERA_FLIP_NONE;
	}

	if (strcasecmp(string, HPVT_Config_CAMERA_FLIP_OFF_NAME) == 0) {
		return HPVT_Config_CAMERA_FLIP_NONE;
	}
	else if (strcasecmp(string, HPVT_Config_CAMERA_FLIP_VERTICAL_NAME) == 0) {
		return HPVT_Config_CAMERA_FLIP_VERTICAL;
	}
	else if (strcasecmp(string, HPVT_Config_CAMERA_FLIP_HORIZONTAL_NAME) == 0) {
		return HPVT_Config_CAMERA_FLIP_HORIZONTAL;
	}
	else if (strcasecmp(string, HPVT_Config_CAMERA_FLIP_BOTH_NAME) == 0) {
		return HPVT_Config_CAMERA_FLIP_BOTH;
	}

	return HPVT_Config_CAMERA_FLIP_NONE;
}
static HPVT_Config_CAMERA_EXPOSURE_MODE _HPVT_Config_convertToExposureMode(const char * string) {
	if (strlen(string) < 1) {
		return HPVT_Config_VALUE_NOT_SET;
	}

	if (strcasecmp(string, HPVT_Config_CAMERA_EXPOSURE_MODE_NORMAL_NAME) == 0) {
		return HPVT_Config_CAMERA_EXPOSURE_MODE_NORMAL;
	}
	else if (strcasecmp(string, HPVT_Config_CAMERA_EXPOSURE_MODE_SHORT_NAME) == 0) {
		return HPVT_Config_CAMERA_EXPOSURE_MODE_SHORT;
	}
	else if (strcasecmp(string, HPVT_Config_CAMERA_EXPOSURE_MODE_LONG_NAME) == 0) {
		return HPVT_Config_CAMERA_EXPOSURE_MODE_LONG;
	}

	return HPVT_Config_VALUE_NOT_SET;
}
static HPVT_Config_CAMERA_METERING_MODE _HPVT_Config_convertToMeteringMode(const char * string) {
	if (strlen(string) < 1) {
		return HPVT_Config_VALUE_NOT_SET;
	}

	if (strcasecmp(string, HPVT_Config_CAMERA_METERING_MODE_CENTRE_NAME) == 0) {
		return HPVT_Config_CAMERA_METERING_MODE_CENTRE;
	}
	else if (strcasecmp(string, HPVT_Config_CAMERA_METERING_MODE_SPOT_NAME) == 0) {
		return HPVT_Config_CAMERA_METERING_MODE_SPOT;
	}
	else if (strcasecmp(string, HPVT_Config_CAMERA_METERING_MODE_MATRIX_NAME) == 0) {
		return HPVT_Config_CAMERA_METERING_MODE_MATRIX;
	}

	return HPVT_Config_VALUE_NOT_SET;
}
static HPVT_Config_CAMERA_WHITE_BALANCE_MODE _HPVT_Config_convertToWhiteBalanceMode(const char * string) {
	if (strlen(string) < 1) {
		return HPVT_Config_VALUE_NOT_SET;
	}

	if (strcasecmp(string, HPVT_Config_CAMERA_WHITE_BALANCE_MODE_AUTO_NAME) == 0) {
		return HPVT_Config_CAMERA_WHITE_BALANCE_MODE_AUTO;
	}
	else if (strcasecmp(string, HPVT_Config_CAMERA_WHITE_BALANCE_MODE_INCANDESCENT_NAME) == 0) {
		return HPVT_Config_CAMERA_WHITE_BALANCE_MODE_INCANDESCENT;
	}
	else if (strcasecmp(string, HPVT_Config_CAMERA_WHITE_BALANCE_MODE_TUNGSTEN_NAME) == 0) {
		return HPVT_Config_CAMERA_WHITE_BALANCE_MODE_TUNGSTEN;
	}
	else if (strcasecmp(string, HPVT_Config_CAMERA_WHITE_BALANCE_MODE_FLUORESCENT_NAME) == 0) {
		return HPVT_Config_CAMERA_WHITE_BALANCE_MODE_FLUORESCENT;
	}
	else if (strcasecmp(string, HPVT_Config_CAMERA_WHITE_BALANCE_MODE_INDOOR_NAME) == 0) {
		return HPVT_Config_CAMERA_WHITE_BALANCE_MODE_INDOOR;
	}
	else if (strcasecmp(string, HPVT_Config_CAMERA_WHITE_BALANCE_MODE_DAYLIGHT_NAME) == 0) {
		return HPVT_Config_CAMERA_WHITE_BALANCE_MODE_DAYLIGHT;
	}
	else if (strcasecmp(string, HPVT_Config_CAMERA_WHITE_BALANCE_MODE_CUSTOM_NAME) == 0) {
		return HPVT_Config_CAMERA_WHITE_BALANCE_MODE_CUSTOM;
	}

	return HPVT_Config_VALUE_NOT_SET;
}
static HPVT_Config_FUNCTION _HPVT_Config_convertToFunction(const char * string) {
	if (strlen(string) < 1) {
		return HPVT_Config_FUNCTION_NONE;
	}

	if (strcasecmp(string, HPVT_Config_FUNCTION_ON_NAME) == 0) {
		return HPVT_Config_FUNCTION_ON;
	}
	else if (strcasecmp(string, HPVT_Config_FUNCTION_OFF_NAME) == 0) {
		return HPVT_Config_FUNCTION_OFF;
	}

	return HPVT_Config_FUNCTION_NONE;
}
static boolean _HPVT_isValidLogFacility(LOG_FACILITY value) {

	switch (value) {
		case LOG_FACILITY_LOCAL0:
		case LOG_FACILITY_LOCAL1:
		case LOG_FACILITY_LOCAL2:
		case LOG_FACILITY_LOCAL3:
		case LOG_FACILITY_LOCAL4:
		case LOG_FACILITY_LOCAL5:
		case LOG_FACILITY_LOCAL6:
		case LOG_FACILITY_LOCAL7:
			return true;
		case LOG_FACILITY_NONE:
			return false;
	}

	return false;
}
static boolean _HPVT_isValidLogLevel(LOG_LEVEL value) {

	switch (value) {
		case LOG_LEVEL_ALERT:
		case LOG_LEVEL_CRITICAL:
		case LOG_LEVEL_DEBUG:
		case LOG_LEVEL_EMERGENCY:
		case LOG_LEVEL_ERROR:
		case LOG_LEVEL_INFO:
		case LOG_LEVEL_NOTICE:
		case LOG_LEVEL_WARNING:
			return true;
		case LOG_LEVEL_NONE:
			return false;
	}

	return false;
}
static boolean _HPVT_Config_isValidIpv4Address(uint32_t value) {

	if (value == 0xffffffff) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}
	return true;
}
static boolean _HPVT_Config_isValidPortNumber(uint16_t value) {

	if (value == 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidTimeout(uint32_t value) {
	if (value < HPVT_Config_TIMEOUT_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_TIMEOUT_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidRetryWait(uint32_t value) {
	if (value < HPVT_Config_RETRY_WAIT_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_RETRY_WAIT_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidConnectionMode(HPVT_Config_CONNECTION_MODE value) {
	switch (value) {
		case HPVT_Config_CONNECTION_MODE_NONE:
		case HPVT_Config_CONNECTION_MODE_INIT_TRANSMITTER:
		case HPVT_Config_CONNECTION_MODE_INIT_RECEIVER:
		case HPVT_Config_CONNECTION_MODE_CONNECTION_LESS:
			return true;
	}

	HPVT_logging(LOG_LEVEL_ERROR, "!");

	return false;
}
static boolean HPVTConfig_isValidType(HPVT_Config_TYPE value) {
	switch (value) {
		case HPVT_Config_TYPE_NONE:
		case HPVT_Config_TYPE_TRANSMITTER:
		case HPVT_Config_TYPE_RECEIVER:
			return true;
	}

	HPVT_logging(LOG_LEVEL_ERROR, "!");

	return false;
}
static boolean _HPVT_Config_isValidPacketSizeDeterminationMode(HPVT_Config_PACKET_SIZE_DETERMINATION_MODE value) {
	switch (value) {
		case HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_NONE:
		case HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_STANDARD:
		case HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_FIXED:
		case HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_AUTO_DISCOVERY:
			return true;
	}

	HPVT_logging(LOG_LEVEL_ERROR, "!");

	return false;
}
static boolean _HPVT_Config_isValidMTUSize(uint32_t value) {
	if (value < HPVT_Config_MTU_SIZE_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_MTU_SIZE_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean _HPVT_Config_isValidFeedbackInterval(uint32_t value) {

	if (value < HPVT_Config_FEEDBACK_INTERVAL_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_FEEDBACK_INTERVAL_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean _HPVT_Config_isValidFunction(HPVT_Config_FUNCTION value) {
	switch (value) {
		case HPVT_Config_FUNCTION_NONE:
		case HPVT_Config_FUNCTION_ON:
		case HPVT_Config_FUNCTION_OFF:
			return true;
	}

	HPVT_logging(LOG_LEVEL_ERROR, "!");

	return false;
}
static boolean HPVT_Config_isValidCameraCaptureVideoWidth(uint32_t value) {
	if (value < HPVT_Config_CAMERA_WIDTH_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_CAMERA_WIDTH_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidCameraCaptureVideoHeight(uint32_t value) {
	if (value < HPVT_Config_CAMERA_HEIGHT_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_CAMERA_HEIGHT_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidVideoBitrate(uint32_t value) {
	if (value < HPVT_Config_VIDEO_BITRATE_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_VIDEO_BITRATE_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidCameraFramerate(uint32_t value) {
	if (value < HPVT_Config_CAMERA_FRAMERATE_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_CAMERA_FRAMERATE_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidVideoFramerate(uint32_t value) {
	if (value < HPVT_Config_VIDEO_FRAMERATE_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_VIDEO_FRAMERATE_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidVideoIdrPeriod(uint32_t value) {
	if (value < HPVT_Config_VIDEO_IDR_PERIOD_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_VIDEO_IDR_PERIOD_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidVideoProfile(HPVT_Config_VIDEO_PROFILE_TYPE value) {
	switch (value) {
		case HPVT_Config_VIDEO_PROFILE_NONE:
		case HPVT_Config_VIDEO_PROFILE_BASELINE:
		case HPVT_Config_VIDEO_PROFILE_MAIN:
		case HPVT_Config_VIDEO_PROFILE_HIGH:
			return true;
	}

	HPVT_logging(LOG_LEVEL_ERROR, "!");

	return false;
}
static boolean _HPVT_isValidLogVideoCameraID(HPVT_Config_CAMERA_ID value) {

	switch (value) {
		case HPVT_Config_CAMERA_ID_1:
		case HPVT_Config_CAMERA_ID_0:
			return true;
	}

	return false;
}
static boolean _HPVT_Config_isValidCameraRotation(uint32_t value) {
	if (value != 0 && value != 180) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidCameraFlip(HPVT_Config_CAMERA_FLIP value) {
	switch (value) {
		case HPVT_Config_CAMERA_FLIP_NONE:
		case HPVT_Config_CAMERA_FLIP_VERTICAL:
		case HPVT_Config_CAMERA_FLIP_HORIZONTAL:
		case HPVT_Config_CAMERA_FLIP_BOTH:
			return true;
	}

	HPVT_logging(LOG_LEVEL_ERROR, "!");

	return false;
}
static boolean HPVT_Config_isValidCameraSharpness(int16_t value) {

	if (value < HPVT_Config_CAMERA_SHARPNESS_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_CAMERA_SHARPNESS_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidCameraBrightness(int16_t value) {

	if (value < HPVT_Config_CAMERA_BRIGHTNESS_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_CAMERA_BRIGHTNESS_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidCameraContrast(int16_t value) {

	if (value < HPVT_Config_CAMERA_CONTRAST_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_CAMERA_CONTRAST_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidCameraSaturation(int32_t value) {

	if (value < HPVT_Config_CAMERA_SATURATION_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_CAMERA_SATURATION_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidCameraShutterSpeed(uint32_t value) {

	if (value < HPVT_Config_CAMERA_SHUTTER_SPEED_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_CAMERA_SHUTTER_SPEED_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidCameraAnalogGain(int16_t value) {

	if (value < HPVT_Config_CAMERA_ANALOG_GAIN_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_CAMERA_ANALOG_GAIN_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidCameraExposureMode(HPVT_Config_CAMERA_EXPOSURE_MODE value) {
	switch (value) {

		case HPVT_Config_CAMERA_EXPOSURE_MODE_NORMAL:
		case HPVT_Config_CAMERA_EXPOSURE_MODE_SHORT:
		case HPVT_Config_CAMERA_EXPOSURE_MODE_LONG:
			return true;
	}

	HPVT_logging(LOG_LEVEL_ERROR, "!");

	return false;
}
static boolean HPVT_Config_isValidCameraEVCompensation(int16_t value) {

	if (value < HPVT_Config_CAMERA_EV_COMPENSATION_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_CAMERA_EV_COMPENSATION_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidCameraMeteringMode(HPVT_Config_CAMERA_METERING_MODE value) {
	switch (value) {

		case HPVT_Config_CAMERA_METERING_MODE_CENTRE:
		case HPVT_Config_CAMERA_METERING_MODE_SPOT:
		case HPVT_Config_CAMERA_METERING_MODE_MATRIX:
			return true;
	}

	HPVT_logging(LOG_LEVEL_ERROR, "!");

	return false;
}
static boolean HPVT_Config_isValidCameraWhiteBalanceMode(HPVT_Config_CAMERA_WHITE_BALANCE_MODE value) {
	switch (value) {
		case HPVT_Config_CAMERA_WHITE_BALANCE_MODE_AUTO:
		case HPVT_Config_CAMERA_WHITE_BALANCE_MODE_INCANDESCENT:
		case HPVT_Config_CAMERA_WHITE_BALANCE_MODE_TUNGSTEN:
		case HPVT_Config_CAMERA_WHITE_BALANCE_MODE_FLUORESCENT:
		case HPVT_Config_CAMERA_WHITE_BALANCE_MODE_INDOOR:
		case HPVT_Config_CAMERA_WHITE_BALANCE_MODE_DAYLIGHT:
		case HPVT_Config_CAMERA_WHITE_BALANCE_MODE_CLOUDY:
		case HPVT_Config_CAMERA_WHITE_BALANCE_MODE_CUSTOM:
			return true;
	}

	HPVT_logging(LOG_LEVEL_ERROR, "!");

	return false;
}
static boolean HPVT_Config_isValidACMeasurementInterval(uint16_t value) {
	if (value < HPVT_Config_AC_MEASUREMENT_DURATION_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_AC_MEASUREMENT_DURATION_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidACLimitAmount(uint16_t value) {
	if (value < HPVT_Config_AC_CHANGE_LIMIT_AMOUNT_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_AC_CHANGE_LIMIT_AMOUNT_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidACMinBitrate(uint32_t value) {
	if (value < HPVT_Config_AC_MIN_BITRATE_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_AC_MIN_BITRATE_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidACIntervalThreshold(uint32_t value) {
	if (value < HPVT_Config_AC_INTERVAL_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_AC_INTERVAL_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidACUnreceivedRateThreshold(uint32_t value) {
	if (value < HPVT_Config_AC_LOSS_RATE_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_AC_LOSS_RATE_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidWhiteBalanceGain(uint32_t value) {

	if (value == 0) {
		return true;
	}

	if (value < HPVT_Config_CAMERA_WHITE_BALANCE_GAIN_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_CAMERA_WHITE_BALANCE_GAIN_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static void _HPVT_Config_adjust_width_height(uint16_t *width, uint16_t *height) {

	if (*width == 0 && *height == 0) {
		return;
	}

	int tmp_width = 0;
	int tmp_height = 0;

	tmp_width = *width;
	tmp_height = *height;

	if (tmp_width == 0) {
		tmp_width = 16 * tmp_height / 9;
	}

	if (tmp_height == 0) {
		tmp_height = 9 * tmp_width / 16;
	}

	*width = tmp_width;
	*height = tmp_height;

}
static boolean _HPVT_Config_check_framerate(HPVT_Config *object) {

	if (object->video_transmitter.camera.framerate > 30000 && object->video_transmitter.camera.height > HPVT_Config_CAMERA_HEIGHT_DEFAULT) {

		printf("Config error!: %s and resolution are both too high\n", //
				HPVT_CONFIG_KEY_CAMERA_CAPTURE_FRAMERATE);
		return false;
	}

	if (object->video_transmitter.camera.framerate >= 1000) {
		if ((int)object->video_transmitter.camera.framerate / 1000 != object->video_transmitter.framerate) {
			printf("Config error!(Simple Video mode): %s is not equal to %s \n", //
					HPVT_CONFIG_KEY_CAMERA_CAPTURE_FRAMERATE, HPVT_CONFIG_KEY_VIDEO_FRAMERATE);
			return false;
		}
	}

	return true;
}
static boolean _HPVT_Config_isValidBufferingDelay(uint32_t value) {
	if (value < HPVT_Config_VIDEO_BUFFERING_DELAY_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_VIDEO_BUFFERING_DELAY_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static boolean HPVT_Config_isValidFECLevel(uint32_t value) {

	if (value == 0) {
		return true;
	}

	if (value < HPVT_Config_FEC_LEVEL_MIN_NUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	if (HPVT_Config_FEC_LEVEL_MAX_NUM < value) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return false;
	}

	return true;
}
static LOG_FACILITY _HPVT_parseLogFacility(const char * string) {
	if (string == NULL) {
		return LOG_FACILITY_NONE;
	}

	if (strlen(string) < 1) {
		return LOG_FACILITY_NONE;
	}

	if (strcmp(string, LOG_FACILITY_DAEMON_NAME) == 0) {
		return LOG_FACILITY_DAEMON;
	}
	else if (strcmp(string, LOG_FACILITY_USER_NAME) == 0) {
		return LOG_FACILITY_USER;
	}
	else if (strcmp(string, LOG_FACILITY_LOCAL0_NAME) == 0) {
		return LOG_FACILITY_LOCAL0;
	}
	else if (strcmp(string, LOG_FACILITY_LOCAL1_NAME) == 0) {
		return LOG_FACILITY_LOCAL1;
	}
	else if (strcmp(string, LOG_FACILITY_LOCAL2_NAME) == 0) {
		return LOG_FACILITY_LOCAL2;
	}
	else if (strcmp(string, LOG_FACILITY_LOCAL3_NAME) == 0) {
		return LOG_FACILITY_LOCAL3;
	}
	else if (strcmp(string, LOG_FACILITY_LOCAL4_NAME) == 0) {
		return LOG_FACILITY_LOCAL4;
	}
	else if (strcmp(string, LOG_FACILITY_LOCAL5_NAME) == 0) {
		return LOG_FACILITY_LOCAL5;
	}
	else if (strcmp(string, LOG_FACILITY_LOCAL6_NAME) == 0) {
		return LOG_FACILITY_LOCAL6;
	}
	else if (strcmp(string, LOG_FACILITY_LOCAL7_NAME) == 0) {
		return LOG_FACILITY_LOCAL7;
	}

	return LOG_FACILITY_NONE;
}
static LOG_LEVEL _HPVT_parseLogLevel(const char * string) {
	if (strlen(string) < 1) {
		return LOG_LEVEL_NONE;
	}

	if (strcasecmp(string, LOG_LEVEL_DEBUG_NAME) == 0) {
		return LOG_LEVEL_DEBUG;
	}
	else if (strcasecmp(string, LOG_LEVEL_INFO_NAME) == 0) {
		return LOG_LEVEL_INFO;
	}
	else if (strcasecmp(string, LOG_LEVEL_NOTICE_NAME) == 0) {
		return LOG_LEVEL_NOTICE;
	}
	else if (strcasecmp(string, LOG_LEVEL_WARNING_NAME) == 0) {
		return LOG_LEVEL_WARNING;
	}
	else if (strcasecmp(string, LOG_LEVEL_ERROR_NAME) == 0) {
		return LOG_LEVEL_ERROR;
	}
	else if (strcasecmp(string, LOG_LEVEL_CRITICAL_NAME) == 0) {
		return LOG_LEVEL_CRITICAL;
	}
	else if (strcasecmp(string, LOG_LEVEL_ALERT_NAME) == 0) {
		return LOG_LEVEL_ALERT;
	}
	else if (strcasecmp(string, LOG_LEVEL_EMERGENCY_NAME) == 0) {
		return LOG_LEVEL_EMERGENCY;
	}

	return LOG_LEVEL_NONE;
}
static boolean _HPVT_parseIpAddress(const char * string, uint32_t * ip_address) {

	char tmp_buffer[HPVT_Config_MAX_LINE_LENGTH];

	memset(tmp_buffer, 0, sizeof(tmp_buffer));
	sprintf(tmp_buffer, "%s", string);

	uint32_t tmp_ip_addr;

	tmp_ip_addr = inet_addr(tmp_buffer);

	if (tmp_ip_addr == 0xffffffff) {
		return false;
	}

	*ip_address = tmp_ip_addr;

	return true;
}
static boolean _HPVT_parseServerIpAddress(const char * string, uint32_t * ip_address) {

	if (strlen(string) < 1) {
		return false;
	}

	return _HPVT_parseIpAddress(string, ip_address);
}
static void _HPVT_trim(char * buffer) {
	char * tmp_buffer;
	char * tmp_p;
	int tmp_length;
	int i;

	if ((tmp_length = strlen(buffer)) < 1) {
		return;
	}

	if ((tmp_buffer = (char *) strdup(buffer)) == NULL) {
		abort();
	}

	tmp_p = tmp_buffer;

	for (i = 0; i < tmp_length; i++) {
		if (!isspace(*tmp_p)) {
			break;
		}

		tmp_p++;
	}

	strcpy(buffer, tmp_p);
	free(tmp_buffer);

	for (i = (strlen(buffer) - 1); 0 <= i; i--) {
		if (!isspace(buffer[i])) {
			break;
		}

		buffer[i] = '\0';
	}
}
static boolean _HPVT_isValid(char * buffer) {
	if (buffer[0] == HPVT_Config_COMMENTOUT_1) {
		return false;
	}

	if (buffer[0] == HPVT_Config_COMMENTOUT_2) {
		return false;
	}

	if (buffer[0] == HPVT_Config_COMMENTOUT_3) {
		return false;
	}

	return true;
}
static boolean _HPVT_split(char * buffer, char ** key, char ** value) {
	char * tmp_p;
	char * tmp_key;
	char * tmp_value;

	if ((tmp_p = strchr(buffer, HPVT_Config_DELIMITER)) == NULL) {
		return false;
	}

	*tmp_p = '\0';

	tmp_key = buffer;
	tmp_value = tmp_p + 1;

	_HPVT_trim(tmp_key);
	_HPVT_trim(tmp_value);

	if (strlen(tmp_key) < 1) {
		return false;
	}

	*key = tmp_key;
	*value = tmp_value;

	return true;
}
static boolean _HPVT_load(HPVT_Config * object, const char * key, const char * value) {

	syslog(LOG_LEVEL_NOTICE, "key=%s, value=%s\n", key, value);

	if (strcasecmp(key, HPVT_CONFIG_KEY_LOG_FACILITY) == 0) {
		object->log.facility = _HPVT_parseLogFacility(value);
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_LOG_LEVEL) == 0) {
		object->log.level = _HPVT_parseLogLevel(value);
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CONNECTION_MODE) == 0) {
		object->connection.connection_mode = _HPVT_Config_convertToConnectionMode(value);
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_TYPE) == 0) {
		object->connection.type = _HPVT_Config_convertToType(value);
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_LISTEN_PORT) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_LISTEN_PORT);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_LISTEN_PORT);
			return false;
		}

		if (tmp_value == 0 || tmp_value > UINT16_MAX) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_LISTEN_PORT);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_LISTEN_PORT);
			return false;
		}

		object->connection.listen_port = (uint16_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CONNECTION_IP) == 0) {

		if (_HPVT_parseServerIpAddress(value, &object->connection.host_ip) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CONNECTION_IP);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CONNECTION_IP);
			return false;
		}
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CONNECTION_PORT) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CONNECTION_PORT);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CONNECTION_PORT);
			return false;
		}

		if (tmp_value == 0 || tmp_value > UINT16_MAX) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CONNECTION_PORT);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CONNECTION_PORT);
			return false;
		}

		object->connection.host_port = (uint16_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_MTU_SIZE) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_MTU_SIZE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_MTU_SIZE);
			return false;
		}

		object->video_transmitter.mtu_size = (uint16_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_PACKET_SIZE_DETERMINATION_MODE) == 0) {
		object->video_transmitter.packet_size_determination_mode = _HPVT_Config_convertToPacketSizeDeterminationMode(value);
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_PATH_MTU_DISCOVERY_MIN_SIZE) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_PATH_MTU_DISCOVERY_MIN_SIZE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_PATH_MTU_DISCOVERY_MIN_SIZE);
			return false;
		}

		object->video_transmitter.path_mtu_discovery_min_size = (uint16_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_PATH_MTU_DISCOVERY_MAX_SIZE) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_PATH_MTU_DISCOVERY_MAX_SIZE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_PATH_MTU_DISCOVERY_MAX_SIZE);
			return false;
		}

		object->video_transmitter.path_mtu_discovery_max_size = (uint16_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_FEEDBACK_CONTROL) == 0) {
		object->video_transmitter.feedback_control = _HPVT_Config_convertToFunction(value);
		return true;
	}

	else if (strcasecmp(key, HPVT_CONFIG_KEY_FEEDBACK_INTERVAL) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_FEEDBACK_INTERVAL);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_FEEDBACK_INTERVAL);
			return false;
		}
		object->video_transmitter.feedback_interval = (uint32_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_FEC_LEVEL) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_FEC_LEVEL);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_FEC_LEVEL);

			return false;
		}

		if (tmp_value == 0) {
			object->video_transmitter.fec_enabled = HPVT_Config_FUNCTION_OFF;
			object->video_transmitter.fec_level = 0;
		}
		else {
			object->video_transmitter.fec_enabled = HPVT_Config_FUNCTION_ON;
			object->video_transmitter.fec_level = (uint16_t) tmp_value;
		}

		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_TIMEOUT) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_TIMEOUT);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_TIMEOUT);
			return false;
		}
		object->connection.server.timeout = (uint32_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_RETRY_WAIT) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_RETRY_WAIT);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_RETRY_WAIT);
			return false;
		}
		object->connection.server.retry_wait = (uint32_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_PACE_CONTROL) == 0) {
		object->video_transmitter.pace_control = _HPVT_Config_convertToFunction(value);
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_VIDEO_CAMERA_ID) == 0) {
		object->video_transmitter.camera_id = _HPVT_Config_convertToCameraID(value);
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_VIDEO_BITRATE) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_VIDEO_BITRATE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_VIDEO_BITRATE);
			return false;
		}

		object->video_transmitter.bitrate = (uint32_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_VIDEO_FRAMERATE) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_VIDEO_FRAMERATE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_VIDEO_FRAMERATE);
			return false;
		}

		object->video_transmitter.framerate = (uint32_t) tmp_value;
		return true;
	}

	else if (strcasecmp(key, HPVT_CONFIG_KEY_VIDEO_I_FRAME_INTERVAL) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_VIDEO_I_FRAME_INTERVAL);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_VIDEO_I_FRAME_INTERVAL);
			return false;
		}

		object->video_transmitter.idr_period = (uint32_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_CAPTURE_WIDTH) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_CAPTURE_WIDTH);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_CAPTURE_WIDTH);

			return false;
		}

		object->video_transmitter.camera.width = (uint32_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_CAPTURE_HEIGHT) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {

			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_CAPTURE_HEIGHT);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_CAPTURE_HEIGHT);

			return false;
		}

		object->video_transmitter.camera.height = (uint32_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_VIDEO_PROFILE) == 0) {
		object->video_transmitter.profile = _HPVT_Config_convertToVideoProfile(value);
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_VIDEO_PREVIEW) == 0) {
		object->video_transmitter.preview_enabled = _HPVT_Config_convertToFunction(value);
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_BUFFERING_DELAY) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_BUFFERING_DELAY);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_BUFFERING_DELAY);
			return false;
		}

		object->video_receiver.buffering_delay = (uint32_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_ROTATION) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {

			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_ROTATION);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_ROTATION);
			return false;
		}

		object->video_transmitter.camera.rotation = (uint32_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_FLIP) == 0) {

		object->video_transmitter.camera.flip = _HPVT_Config_convertToCameraFlip(value);
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_SHARPNESS) == 0) {

		double tmp_work;
		uint16_t tmp_value;

		tmp_work = atof(value);

		if (tmp_work < (HPVT_Config_CAMERA_SHARPNESS_MIN_NUM / 1000) || tmp_work > (HPVT_Config_CAMERA_SHARPNESS_MAX_NUM / 1000)) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_SHARPNESS);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_SHARPNESS);
			return false;
		}

		tmp_work *= 1000.0;
		tmp_value = (uint16_t) (tmp_work);

		object->video_transmitter.camera.sharpness = tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_BRIGHTNESS) == 0) {

		double tmp_work;
		int16_t tmp_value;

		tmp_work = atof(value);

		if (tmp_work < (HPVT_Config_CAMERA_BRIGHTNESS_MIN_NUM / 1000) || tmp_work > (HPVT_Config_CAMERA_BRIGHTNESS_MAX_NUM / 1000)) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_BRIGHTNESS);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_BRIGHTNESS);
			return false;
		}

		tmp_work *= 1000.0;
		tmp_value = (int16_t) (tmp_work);

		object->video_transmitter.camera.brightness = tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_CONTRAST) == 0) {

		double tmp_work;
		uint16_t tmp_value;

		tmp_work = atof(value);

		if (tmp_work < (HPVT_Config_CAMERA_CONTRAST_MIN_NUM / 1000) || tmp_work > (HPVT_Config_CAMERA_CONTRAST_MAX_NUM / 1000)) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_CONTRAST);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_CONTRAST);
			return false;
		}

		tmp_work *= 1000.0;
		tmp_value = (uint16_t) (tmp_work);

		object->video_transmitter.camera.contrast = tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_SATURATION) == 0) {

		double tmp_work;
		uint16_t tmp_value;

		tmp_work = atof(value);

		if (tmp_work < (HPVT_Config_CAMERA_SATURATION_MIN_NUM / 1000) || tmp_work > (HPVT_Config_CAMERA_SATURATION_MAX_NUM / 1000)) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_SATURATION);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_SATURATION);
			return false;
		}

		tmp_work *= 1000.0;
		tmp_value = (uint16_t) (tmp_work);

		object->video_transmitter.camera.saturation = tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_SHUTTER_SPEED) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_SHUTTER_SPEED);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_SHUTTER_SPEED);
			return false;
		}

		object->video_transmitter.camera.shutter_speed = (uint32_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_ANALOG_GAIN) == 0) {

		double tmp_work;
		uint16_t tmp_value;

		tmp_work = atof(value);

		if (tmp_work < (HPVT_Config_CAMERA_ANALOG_GAIN_MIN_NUM / 1000) || tmp_work > (HPVT_Config_CAMERA_ANALOG_GAIN_MAX_NUM / 1000)) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_ANALOG_GAIN);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_ANALOG_GAIN);
			return false;
		}

		tmp_work *= 1000.0;
		tmp_value = (uint16_t) (tmp_work);

		object->video_transmitter.camera.analog_gain = tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_EXPOSURE_MODE) == 0) {

		object->video_transmitter.camera.exposure_mode = _HPVT_Config_convertToExposureMode(value);

		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_EV_COMPENSATAION) == 0) {

		double tmp_work;
		int16_t tmp_value;

		tmp_work = atof(value);

		if (tmp_work < (HPVT_Config_CAMERA_EV_COMPENSATION_MIN_NUM / 1000) || tmp_work > (HPVT_Config_CAMERA_EV_COMPENSATION_MAX_NUM / 1000)) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_EV_COMPENSATAION);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_EV_COMPENSATAION);
			return false;
		}

		tmp_work *= 1000.0;
		tmp_value = (int16_t) (tmp_work);

		object->video_transmitter.camera.exposure_compensation = tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_METERING_MODE) == 0) {

		object->video_transmitter.camera.metering_mode = _HPVT_Config_convertToMeteringMode(value);
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_MODE) == 0) {

		object->video_transmitter.camera.white_balance.mode = _HPVT_Config_convertToWhiteBalanceMode(value);
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_RED_GAIN) == 0) {

		double tmp_work;
		uint16_t tmp_value;

		tmp_work = atof(value);

		if (tmp_work < HPVT_Config_CAMERA_WHITE_BALANCE_GAIN_MIN_NUM || tmp_work > (HPVT_Config_CAMERA_WHITE_BALANCE_GAIN_MAX_NUM / 1000)) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_RED_GAIN);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_RED_GAIN);
			return false;
		}

		tmp_work *= 1000.0;
		tmp_value = (uint16_t) (tmp_work);

		object->video_transmitter.camera.white_balance.red_gain = tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_BLUE_GAIN) == 0) {

		double tmp_work;
		uint16_t tmp_value;

		tmp_work = atof(value);

		if (tmp_work < HPVT_Config_CAMERA_WHITE_BALANCE_GAIN_MIN_NUM || tmp_work > (HPVT_Config_CAMERA_WHITE_BALANCE_GAIN_MAX_NUM / 1000)) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_BLUE_GAIN);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_BLUE_GAIN);
			return false;
		}

		tmp_work *= 1000.0;
		tmp_value = (uint16_t) (tmp_work);

		object->video_transmitter.camera.white_balance.blue_gain = tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_CAMERA_CAPTURE_FRAMERATE) == 0) {

		double tmp_work;
		uint32_t tmp_value;

		tmp_work = atof(value);
		tmp_work *= 1000.0;
		tmp_value = (uint32_t)tmp_work;

		if (tmp_value < HPVT_Config_CAMERA_FRAMERATE_MIN_NUM || tmp_value > (HPVT_Config_CAMERA_FRAMERATE_MAX_NUM)) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_CAPTURE_FRAMERATE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_CAPTURE_FRAMERATE);
			return false;
		}

		object->video_transmitter.camera.framerate = (uint32_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_ADAPTIVE_CONTROL) == 0) {
		object->video_adaptive_control = _HPVT_Config_convertToFunction(value);
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_AC_MEASUREMENT_INTERVAL) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_AC_MEASUREMENT_INTERVAL);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_AC_MEASUREMENT_INTERVAL);
			return false;
		}

		object->video_transmitter.adaptive_control_param.measurement_interval = (uint16_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_AC_LIMIT_OF_VARIATION) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_AC_LIMIT_OF_VARIATION);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_AC_LIMIT_OF_VARIATION);
			return false;
		}

		object->video_transmitter.adaptive_control_param.limit_amount = (uint16_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_AC_MIN_BITRATE) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_AC_MIN_BITRATE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_AC_MIN_BITRATE);
			return false;
		}

		object->video_transmitter.adaptive_control_param.min_bitrate = tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_AC_INTERVAL_THRESHOLD) == 0) {

		uint32_t tmp_value;

		tmp_value = 0;

		if (HPVT_strtouint32(value, 10, &tmp_value) == false) {

			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_AC_INTERVAL_THRESHOLD);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_AC_INTERVAL_THRESHOLD);
			return false;
		}

		object->video_transmitter.adaptive_control_param.interval_threshold = (uint16_t) tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_AC_UNRECEIVED_RATE_THRESHOLD) == 0) {

		double tmp_work;
		uint16_t tmp_value;

		tmp_work = atof(value);

		tmp_work *= 1000.0;
		tmp_work /= 100.0;

		if (tmp_work < HPVT_Config_AC_LOSS_RATE_MIN_NUM || tmp_work > (HPVT_Config_AC_LOSS_RATE_MAX_NUM)) {
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_AC_UNRECEIVED_RATE_THRESHOLD);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_AC_UNRECEIVED_RATE_THRESHOLD);
			return false;
		}

		tmp_value = (uint16_t) (tmp_work);

		object->video_transmitter.adaptive_control_param.unreceived_rate_threshold = tmp_value;
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_PREVIEW_FULL_SCREEN) == 0) {
		object->preview.full_screen = _HPVT_Config_convertToFunction(value);
	}
	else {
		// Ignore
	}

	return true;

}
static boolean _HPVT_load_only_logging_settings(HPVT_Config * object, const char * key, const char * value) {

	if (strcasecmp(key, HPVT_CONFIG_KEY_LOG_FACILITY) == 0) {
		object->log.facility = _HPVT_parseLogFacility(value);
		return true;
	}
	else if (strcasecmp(key, HPVT_CONFIG_KEY_LOG_LEVEL) == 0) {
		object->log.level = _HPVT_parseLogLevel(value);
		return true;
	}

	return true;
}
static boolean _HPVT_Config_load(HPVT_Config * object, FILE * fp, int particular) {

	char tmp_buffer[HPVT_Config_MAX_LINE_LENGTH];
	int n;

	for (n = 1;; n++) {
		char * tmp_key;
		char * tmp_value;

		if (fgets(tmp_buffer, sizeof(tmp_buffer), fp) == NULL) {
			break;
		}

		if (strlen(tmp_buffer) < 1) {
			continue;
		}

		if (_HPVT_isValid(tmp_buffer) == false) {
			continue;
		}

		if (_HPVT_split(tmp_buffer, &tmp_key, &tmp_value) == false) {
			continue;
		}

		if (particular == HPVT_Config_LOAD_LOGGING) {
			if (_HPVT_load_only_logging_settings(object, tmp_key, tmp_value) == false) {
				return false;
			}
		}

		else {
			if (_HPVT_load(object, tmp_key, tmp_value) == false) {
				return false;
			}
		}

	}

	return true;
}

/* ---------------------------------------------------------------------------------------------------- */
boolean HPVT_Config_load(HPVT_Config * object, const char * path, int particular) {
	boolean tmp_result;
	FILE * tmp_fp;

	if (object == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (path == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	struct stat tmp_stat;

	if (stat(path, &tmp_stat) == 0) {

		if (S_ISDIR(tmp_stat.st_mode)) {
			printf("<<%s>> is a directory\n", path);
			return false;
		}
	}

	if ((tmp_fp = fopen(path, "r")) == NULL) {

		printf("Could not find the file <<%s>>\n", path);
		return false;
	}

	tmp_result = _HPVT_Config_load(object, tmp_fp, particular);

	fclose(tmp_fp);

	if ((object->config_file = strdup(path)) == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
		return false;
	}

	return tmp_result;
}
boolean HPVT_Config_fix(HPVT_Config * object, HPVT_Context *context) {

	if (object == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	/* Log Facility */
	if (_HPVT_isValidLogFacility(object->log.facility) == false) {
		object->log.facility = HPVT_LOG_FACILITY_DEFAULT;
	}

	/* Log Level */
	if (_HPVT_isValidLogLevel(object->log.level) == false) {
		object->log.level = HPVT_LOG_LEVEL_DEFAULT;
	}

	/* Logging Directory */

	if (object->connection.connection_mode == HPVT_Config_CONNECTION_MODE_NONE) {
		object->connection.connection_mode = HPVT_Config_CONNECTION_MODE_DEFAULT;
	}

	if (object->connection.type == HPVT_Config_TYPE_NONE) {
		object->connection.type = HPVT_Config_TYPE_DEFAULT;
	}

	if (object->connection.listen_port == 0) {
		object->connection.listen_port = HPVT_Config_UDP_PORT_DEFAULT;
	}

	if (object->connection.host_port == 0) {
		object->connection.host_port = HPVT_Config_UDP_PORT_DEFAULT;
	}

	if (object->connection.type == HPVT_Config_TYPE_TRANSMITTER) {

		if (object->video_transmitter.mtu_size == 0) {
			object->video_transmitter.mtu_size = HPVT_Config_MTU_SIZE_DEFAULT;
		}

		if (object->video_transmitter.packet_size_determination_mode == 0) {
			object->video_transmitter.packet_size_determination_mode = HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_DEFAULT;
		}

		if (object->video_transmitter.path_mtu_discovery_min_size == 0) {
			object->video_transmitter.path_mtu_discovery_min_size = HPVT_Config_PATH_MTU_DISCOVERY_MINIMUM_SIZE_DEFAULT;
		}

		if (object->video_transmitter.path_mtu_discovery_max_size == 0) {
			object->video_transmitter.path_mtu_discovery_max_size = HPVT_Config_PATH_MTU_DISCOVERY_MAXIMUM_SIZE_DEFAULT;
		}

		if (object->connection.server.timeout == HPVT_Config_VALUE_NOT_SET) {
			object->connection.server.timeout = HPVT_Config_TIMEOUT_DEFAULT;
		}

		if (object->connection.server.retry_wait == 0) {
			object->connection.server.retry_wait = HPVT_Config_RETRY_WAIT_DEFAULT;
		}

		if (object->video_transmitter.pace_control == HPVT_Config_FUNCTION_NONE) {
			object->video_transmitter.pace_control = HPVT_Config_FUNCTION_ON;
		}

		if (object->video_transmitter.preview_enabled == HPVT_Config_FUNCTION_NONE) {
			object->video_transmitter.preview_enabled = HPVT_Config_FUNCTION_ON;
		}

		if (object->video_transmitter.feedback_control == HPVT_Config_FUNCTION_NONE) {
			object->video_transmitter.feedback_control = HPVT_Config_FUNCTION_ON;
		}

		if (object->video_transmitter.feedback_interval == 0) {
			object->video_transmitter.feedback_interval = HPVT_Config_FEEDBACK_INTERVAL_DEFAULT;
		}

		if (object->video_adaptive_control == HPVT_Config_FUNCTION_NONE) {
			object->video_adaptive_control = HPVT_Config_FUNCTION_OFF;
		}

		if (object->video_transmitter.adaptive_control_param.measurement_interval == 0) {
			object->video_transmitter.adaptive_control_param.measurement_interval = HPVT_Config_AC_MEASUREMENT_DURATION_DEFAULT;
		}

		if (object->video_transmitter.adaptive_control_param.limit_amount == 0) {
			object->video_transmitter.adaptive_control_param.limit_amount = HPVT_Config_AC_CHANGE_LIMIT_AMOUNT_DEFAULT;
		}

		if (object->video_transmitter.adaptive_control_param.interval_threshold == 0) {
			object->video_transmitter.adaptive_control_param.interval_threshold = HPVT_Config_AC_INTERVAL_DEFAULT;
		}

		if (object->video_transmitter.adaptive_control_param.unreceived_rate_threshold == 0) {
			object->video_transmitter.adaptive_control_param.unreceived_rate_threshold = HPVT_Config_AC_LOSS_RATE_DEFAULT;
		}

		if (object->video_transmitter.adaptive_control_param.min_bitrate == 0) {
			object->video_transmitter.adaptive_control_param.min_bitrate = HPVT_Config_AC_MIN_BITRATE_DEFAULT;
		}

		if (object->video_transmitter.fec_enabled == 0) {
			object->video_transmitter.fec_enabled = HPVT_Config_FUNCTION_ON;
			object->video_transmitter.fec_level = HPVT_Config_FEC_LEVEL_DEFAULT;
		}

		if (object->video_transmitter.camera.framerate == 0) {
			object->video_transmitter.camera.framerate = HPVT_Config_CAMERA_FRAMERATE_DEFAULT;
		}

		if (object->video_transmitter.camera.width == 0 && object->video_transmitter.camera.height == 0) {
			object->video_transmitter.camera.width = HPVT_Config_CAMERA_WIDTH_DEFAULT;
			object->video_transmitter.camera.height = HPVT_Config_CAMERA_HEIGHT_DEFAULT;
		}
		else if (object->video_transmitter.camera.width == 0 || object->video_transmitter.camera.height == 0) {
			_HPVT_Config_adjust_width_height(&object->video_transmitter.camera.width, &object->video_transmitter.camera.height);
		}

		if (object->video_transmitter.camera.rotation == HPVT_Config_VALUE_NOT_SET) {
			object->video_transmitter.camera.rotation = HPVT_Config_CAMERA_ROTATION_DEFAULT;
		}

		if (object->video_transmitter.camera.flip == HPVT_Config_VALUE_NOT_SET) {
			object->video_transmitter.camera.flip = HPVT_Config_CAMERA_FLIP_DEFAULT;
		}

		if (object->video_transmitter.camera.white_balance.mode == HPVT_Config_VALUE_NOT_SET) {
			object->video_transmitter.camera.white_balance.mode = HPVT_Config_CAMERA_WHITE_BALANCE_MODE_DEFAULT;
		}

		if (object->video_transmitter.camera.white_balance.red_gain == HPVT_Config_VALUE_NOT_SET) {
			object->video_transmitter.camera.white_balance.red_gain = HPVT_Config_CAMERA_WHITE_BALANCE_RED_GAIN_DEFAULT;
		}

		if (object->video_transmitter.camera.white_balance.blue_gain == HPVT_Config_VALUE_NOT_SET) {
			object->video_transmitter.camera.white_balance.blue_gain = HPVT_Config_CAMERA_WHITE_BALANCE_BLUE_GAIN_DEFAULT;
		}

		if (object->video_transmitter.camera.sharpness == HPVT_Config_VALUE_NOT_SET) {
			object->video_transmitter.camera.sharpness = HPVT_Config_CAMERA_SHARPNESS_DEFAULT;
		}

		if (object->video_transmitter.camera.brightness == HPVT_Config_VALUE_NOT_SET) {
			object->video_transmitter.camera.brightness = HPVT_Config_CAMERA_BRIGHTNESS_DEFAULT;
		}

		if (object->video_transmitter.camera.contrast == HPVT_Config_VALUE_NOT_SET) {
			object->video_transmitter.camera.contrast = HPVT_Config_CAMERA_CONTRAST_DEFAULT;
		}

		if (object->video_transmitter.camera.saturation == HPVT_Config_VALUE_NOT_SET) {
			object->video_transmitter.camera.saturation = HPVT_Config_CAMERA_SATURATION_DEFAULT;
		}

		if (object->video_transmitter.camera.shutter_speed == 0) {
			object->video_transmitter.camera.shutter_speed = HPVT_Config_CAMERA_SHUTTER_SPEED_DEFAULT;
		}

		if (object->video_transmitter.camera.analog_gain == HPVT_Config_VALUE_NOT_SET) {
			object->video_transmitter.camera.analog_gain = HPVT_Config_CAMERA_ANALOG_GAIN_DEFAULT;
		}

		if (object->video_transmitter.camera.exposure_mode == HPVT_Config_VALUE_NOT_SET) {
			object->video_transmitter.camera.exposure_mode = HPVT_Config_CAMERA_EXPOSURE_MODE_DEFAULT;
		}

		if (object->video_transmitter.camera.exposure_compensation == HPVT_Config_VALUE_NOT_SET) {
			object->video_transmitter.camera.exposure_compensation = HPVT_Config_CAMERA_EV_COMPENSATION_DEFAULT;
		}

		if (object->video_transmitter.camera.metering_mode == HPVT_Config_VALUE_NOT_SET) {
			object->video_transmitter.camera.metering_mode = HPVT_Config_CAMERA_METERING_MODE_DEFAULT;
		}

		if (object->video_transmitter.bitrate == 0) {
			object->video_transmitter.bitrate = HPVT_Config_VIDEO_BITRATE_DEFAULT;
		}

		if (object->video_transmitter.framerate == 0) {
			object->video_transmitter.framerate = HPVT_Config_VIDEO_FRAMERATE_DEFAULT;
		}

		if (object->video_transmitter.idr_period == 0) {
			object->video_transmitter.idr_period = HPVT_Config_VIDEO_FRAMERATE_DEFAULT;
		}

		if (object->video_transmitter.profile == 0) {
			object->video_transmitter.profile = HPVT_Config_VIDEO_PROFILE_DEFAULT;
		}
	}
	else {

		if (object->video_receiver.buffering_delay == 0) {
			object->video_receiver.buffering_delay = HPVT_Config_VIDEO_BUFFERING_DELAY_DEFAULT;
		}
	}

	if (object->preview.full_screen == 0) {
		object->preview.full_screen = HPVT_Config_FUNCTION_OFF;
	}

	return true;
}
boolean HPVT_Config_verify(HPVT_Config * object, HPVT_Context *context) {
	boolean tmp_result;
	char * tmp_home_path;
	char tmp_path[HPVT_PATH_BUFFER_SIZE];

	if (object == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (context == NULL) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	tmp_result = true;

	/* Log */
	if (_HPVT_isValidLogFacility(object->log.facility) == false) {
		tmp_result = false;
		HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_LOG_FACILITY);
		printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_LOG_FACILITY);
	}

	if (_HPVT_isValidLogLevel(object->log.level) == false) {
		tmp_result = false;
		HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_LOG_LEVEL);
		printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_LOG_LEVEL);
	}

	if (HPVT_Config_isValidConnectionMode(object->connection.connection_mode) == false) {
		tmp_result = false;
		HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CONNECTION_MODE);
		printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CONNECTION_MODE);
	}

	if (HPVTConfig_isValidType(object->connection.type) == false) {
		tmp_result = false;
		HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_TYPE);
		printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_TYPE);
	}

	if (_HPVT_Config_isValidFunction(object->video_adaptive_control) == false) {
		tmp_result = false;
		HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_ADAPTIVE_CONTROL);
		printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_ADAPTIVE_CONTROL);
	}

	if (HPVT_Config_is_server(object->connection.type, object->connection.connection_mode) == true) {

		if (_HPVT_Config_isValidPortNumber(object->connection.listen_port) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_LISTEN_PORT);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_LISTEN_PORT);
		}
	}
	else {
		if (_HPVT_Config_isValidIpv4Address(object->connection.host_ip) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CONNECTION_IP);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CONNECTION_IP);
		}

		if (_HPVT_Config_isValidPortNumber(object->connection.host_port) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CONNECTION_PORT);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CONNECTION_PORT);
		}
	}

	if (object->connection.type == HPVT_Config_TYPE_TRANSMITTER) {

		if (_HPVT_Config_isValidMTUSize(object->video_transmitter.mtu_size) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_MTU_SIZE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_MTU_SIZE);
		}

		if (_HPVT_Config_isValidPacketSizeDeterminationMode(object->video_transmitter.packet_size_determination_mode) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_PACKET_SIZE_DETERMINATION_MODE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_PACKET_SIZE_DETERMINATION_MODE);
		}

		if (_HPVT_Config_isValidMTUSize(object->video_transmitter.path_mtu_discovery_min_size) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_PATH_MTU_DISCOVERY_MIN_SIZE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_PATH_MTU_DISCOVERY_MIN_SIZE);
		}

		if (_HPVT_Config_isValidMTUSize(object->video_transmitter.path_mtu_discovery_max_size) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_PATH_MTU_DISCOVERY_MAX_SIZE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_PATH_MTU_DISCOVERY_MAX_SIZE);
		}

		if (object->video_transmitter.path_mtu_discovery_min_size > object->video_transmitter.path_mtu_discovery_max_size) {

			tmp_result = false;
			printf("Config error!:PathMTU Discovery MaxSize must be more than MinSize\n");
			return false;
		}

		if (HPVT_Config_isValidTimeout(object->connection.server.timeout) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_TIMEOUT);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_TIMEOUT);
		}

		if (HPVT_Config_isValidRetryWait(object->connection.server.retry_wait) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_RETRY_WAIT);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_RETRY_WAIT);
		}

		if (_HPVT_Config_isValidFunction(object->video_transmitter.pace_control) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_PACE_CONTROL);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_PACE_CONTROL);
		}

		if (_HPVT_Config_isValidFunction(object->video_transmitter.feedback_control) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_FEEDBACK_CONTROL);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_FEEDBACK_CONTROL);
		}

		if (_HPVT_Config_isValidFeedbackInterval(object->video_transmitter.feedback_interval) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_FEEDBACK_INTERVAL);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_FEEDBACK_INTERVAL);
		}

		if (_HPVT_isValidLogVideoCameraID(object->video_transmitter.camera_id) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_VIDEO_CAMERA_ID);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_VIDEO_CAMERA_ID);
		}

		if (HPVT_Config_isValidVideoBitrate(object->video_transmitter.bitrate) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_VIDEO_BITRATE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_VIDEO_BITRATE);
		}

		if (HPVT_Config_isValidVideoFramerate(object->video_transmitter.framerate) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_VIDEO_FRAMERATE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_VIDEO_FRAMERATE);
		}

		if (HPVT_Config_isValidVideoIdrPeriod(object->video_transmitter.idr_period) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_VIDEO_I_FRAME_INTERVAL);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_VIDEO_I_FRAME_INTERVAL);
		}

		if (HPVT_Config_isValidVideoProfile(object->video_transmitter.profile) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_VIDEO_PROFILE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_VIDEO_PROFILE);
		}

		if (_HPVT_Config_isValidCameraRotation(object->video_transmitter.camera.rotation) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_ROTATION);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_ROTATION);
		}

		if (HPVT_Config_isValidCameraFlip(object->video_transmitter.camera.flip) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_FLIP);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_FLIP);
		}

		if (HPVT_Config_isValidCameraSharpness(object->video_transmitter.camera.sharpness) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_SHARPNESS);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_SHARPNESS);
		}

		if (HPVT_Config_isValidCameraBrightness(object->video_transmitter.camera.brightness) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_BRIGHTNESS);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_BRIGHTNESS);
		}

		if (HPVT_Config_isValidCameraContrast(object->video_transmitter.camera.contrast) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_CONTRAST);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_CONTRAST);
		}

		if (HPVT_Config_isValidCameraSaturation(object->video_transmitter.camera.saturation) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_SATURATION);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_SATURATION);
		}

		if (HPVT_Config_isValidCameraShutterSpeed(object->video_transmitter.camera.shutter_speed) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_SHUTTER_SPEED);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_SHUTTER_SPEED);
		}

		if (HPVT_Config_isValidCameraAnalogGain(object->video_transmitter.camera.analog_gain) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_ANALOG_GAIN);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_ANALOG_GAIN);
		}

		if (HPVT_Config_isValidCameraExposureMode(object->video_transmitter.camera.exposure_mode) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_EXPOSURE_MODE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_EXPOSURE_MODE);
		}

		if (HPVT_Config_isValidCameraEVCompensation(object->video_transmitter.camera.exposure_compensation) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_EV_COMPENSATAION);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_EV_COMPENSATAION);
		}

		if (HPVT_Config_isValidCameraMeteringMode(object->video_transmitter.camera.metering_mode) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_METERING_MODE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_METERING_MODE);
		}

		if (HPVT_Config_isValidCameraWhiteBalanceMode(object->video_transmitter.camera.white_balance.mode) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_MODE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_MODE);
		}

		if (HPVT_Config_isValidWhiteBalanceGain(object->video_transmitter.camera.white_balance.red_gain) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_RED_GAIN);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_RED_GAIN);
		}

		if (HPVT_Config_isValidWhiteBalanceGain(object->video_transmitter.camera.white_balance.blue_gain) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_BLUE_GAIN);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_BLUE_GAIN);
		}

		if (HPVT_Config_isValidCameraCaptureVideoWidth(object->video_transmitter.camera.width) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_CAPTURE_WIDTH);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_CAPTURE_WIDTH);
		}

		if (HPVT_Config_isValidCameraCaptureVideoHeight(object->video_transmitter.camera.height) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_CAMERA_CAPTURE_HEIGHT);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_CAMERA_CAPTURE_HEIGHT);
		}

		if (HPVT_Config_isValidFECLevel(object->video_transmitter.fec_level) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_FEC_LEVEL);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_FEC_LEVEL);
		}

		if (HPVT_Config_isValidACMeasurementInterval(object->video_transmitter.adaptive_control_param.measurement_interval) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_AC_MEASUREMENT_INTERVAL);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_AC_MEASUREMENT_INTERVAL);
		}

		if (HPVT_Config_isValidACLimitAmount(object->video_transmitter.adaptive_control_param.limit_amount) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_AC_LIMIT_OF_VARIATION);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_AC_LIMIT_OF_VARIATION);
		}

		if (HPVT_Config_isValidACMinBitrate(object->video_transmitter.adaptive_control_param.min_bitrate) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_AC_MIN_BITRATE);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_AC_MIN_BITRATE);
		}

		if (HPVT_Config_isValidACIntervalThreshold(object->video_transmitter.adaptive_control_param.interval_threshold) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_AC_INTERVAL_THRESHOLD);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_AC_INTERVAL_THRESHOLD);
		}

		if (HPVT_Config_isValidACUnreceivedRateThreshold(object->video_transmitter.adaptive_control_param.unreceived_rate_threshold) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_AC_UNRECEIVED_RATE_THRESHOLD);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_AC_UNRECEIVED_RATE_THRESHOLD);
		}

		if (_HPVT_Config_check_framerate(object) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:frame rate is invalid");

		}

	}
	else if (object->connection.type == HPVT_Config_TYPE_RECEIVER) {

		if (_HPVT_Config_isValidBufferingDelay(object->video_receiver.buffering_delay) == false) {
			tmp_result = false;
			HPVT_logging(LOG_LEVEL_ERROR, "Config error!:'%s' is invalid", HPVT_CONFIG_KEY_BUFFERING_DELAY);
			printf("Config error!:'%s' is invalid\n", HPVT_CONFIG_KEY_BUFFERING_DELAY);
		}

	}

	return tmp_result;
}
boolean HPVT_Config_is_server(HPVT_Config_TYPE type, HPVT_Config_CONNECTION_MODE mode) {

	if (type == HPVT_Config_TYPE_TRANSMITTER) {

		if (mode == HPVT_Config_CONNECTION_MODE_INIT_RECEIVER) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		if (mode != HPVT_Config_CONNECTION_MODE_INIT_RECEIVER) {
			return true;
		}
		else {
			return false;
		}
	}
}
void HPVT_Config_logging(HPVT_Config * object) {
	syslog(LOG_NOTICE, "### Configuration");
	syslog(LOG_NOTICE, "# Config File                    : %s", (object->config_file != NULL ? object->config_file : "none"));
	syslog(LOG_NOTICE, "# Log Facility                   : %s", LOG_FACILITY_TO_STRING(object->log.facility));
	syslog(LOG_NOTICE, "# Log Level                      : %s", LOG_LEVEL_TO_STRING(object->log.level));

	syslog(LOG_NOTICE, "# Type                           : %s", _HPVT_Config_logging_convert_name_type(object->connection.type));
	syslog(LOG_NOTICE, "# Connection Mode                : %s", _HPVT_Config_logging_convert_name_connection_mode(object->connection.connection_mode));

	if (HPVT_Config_is_server(object->connection.type, object->connection.connection_mode) == true) {
		syslog(LOG_NOTICE, "# Listen Port                    : %d", object->connection.listen_port);
	}
	else {
		syslog(LOG_NOTICE, "# Connection IP                  : %x", object->connection.host_ip);
		syslog(LOG_NOTICE, "# Connection Port                : %d", object->connection.host_port);
	}

	if (object->connection.type == HPVT_Config_TYPE_TRANSMITTER) {

		syslog(LOG_NOTICE, "##[Transmitter]");
		syslog(LOG_NOTICE, "# Packet Size Determination Mode : %s", //
				_HPVT_Config_logging_convert_name_packet_size_determination_mode(object->video_transmitter.packet_size_determination_mode));
		syslog(LOG_NOTICE, "# MTU Size                       : %d bytes", object->video_transmitter.mtu_size);
		syslog(LOG_NOTICE, "# Path MTU Discovery Min Size    : %d bytes", object->video_transmitter.path_mtu_discovery_min_size);
		syslog(LOG_NOTICE, "# Path MTU Discovery Max Size    : %d bytes", object->video_transmitter.path_mtu_discovery_max_size);
		syslog(LOG_NOTICE, "# Timeout                        : %d sec", object->connection.server.timeout);
		syslog(LOG_NOTICE, "# Retry Wait                     : %d sec", object->connection.server.retry_wait);
		syslog(LOG_NOTICE, "# Pace Control                   : %s", _HPVT_Config_logging_convert_name_on_off(object->video_transmitter.pace_control));
		syslog(LOG_NOTICE, "# FEC Function                   : %s", _HPVT_Config_logging_convert_name_on_off(object->video_transmitter.fec_enabled));
		syslog(LOG_NOTICE, "# FEC Level                      : %d", object->video_transmitter.fec_level);
		syslog(LOG_NOTICE, "# Camera Capture Frame Rate      : %d fps", object->video_transmitter.camera.framerate);
		syslog(LOG_NOTICE, "# Camera Capture Width           : %d px", object->video_transmitter.camera.width);
		syslog(LOG_NOTICE, "# Camera Capture Height          : %d px", object->video_transmitter.camera.height);
		syslog(LOG_NOTICE, "# Camera Rotation                : %d degrees", object->video_transmitter.camera.rotation);
		syslog(LOG_NOTICE, "# Camera Flip                    : %s", HPVT_Config_logging_convert_name_flip(object->video_transmitter.camera.flip));
		syslog(LOG_NOTICE, "# Camera Sharpness               : %d", object->video_transmitter.camera.sharpness);
		syslog(LOG_NOTICE, "# Camera Brightness              : %d", object->video_transmitter.camera.brightness);
		syslog(LOG_NOTICE, "# Camera Contrast                : %d", object->video_transmitter.camera.contrast);
		syslog(LOG_NOTICE, "# Camera Saturation              : %d", object->video_transmitter.camera.saturation);
		syslog(LOG_NOTICE, "# Camera Shutter Speed           : %d usec", object->video_transmitter.camera.shutter_speed);
		syslog(LOG_NOTICE, "# Camera Exposure Mode           : %s", HPVT_Config_logging_convert_name_exposure_mode(object->video_transmitter.camera.exposure_mode));
		syslog(LOG_NOTICE, "# Camera EV Compensation         : %d", object->video_transmitter.camera.exposure_compensation);
		syslog(LOG_NOTICE, "# Camera Metering Mode           : %s", HPVT_Config_logging_convert_name_metering_mode(object->video_transmitter.camera.metering_mode));
		syslog(LOG_NOTICE, "# Camera White Balance Mode      : %s", HPVT_Config_logging_convert_name_whitebalance_mode(object->video_transmitter.camera.white_balance.mode));
		syslog(LOG_NOTICE, "# Camera White Balance Red Gain  : %d", object->video_transmitter.camera.white_balance.red_gain);
		syslog(LOG_NOTICE, "# Camera White Balance Blue Gain : %d", object->video_transmitter.camera.white_balance.blue_gain);
		syslog(LOG_NOTICE, "# Camera Analog Gain             : %d", object->video_transmitter.camera.analog_gain);
		syslog(LOG_NOTICE, "# Video Camera ID                : %d", object->video_transmitter.camera_id);
		syslog(LOG_NOTICE, "# Video Bit Rate                 : %d kbps", object->video_transmitter.bitrate);
		syslog(LOG_NOTICE, "# Video Frame Rate               : %d fps", object->video_transmitter.framerate);
		syslog(LOG_NOTICE, "# Video I Frame Interval         : %d", object->video_transmitter.idr_period);
		syslog(LOG_NOTICE, "# Video Profile                  : %s", _HPVT_Config_logging_convert_name_prifile_name(object->video_transmitter.profile));
		syslog(LOG_NOTICE, "# Video Preview                  : %s", _HPVT_Config_logging_convert_name_on_off(object->video_transmitter.preview_enabled));
		syslog(LOG_NOTICE, "# Feedback Control               : %s", _HPVT_Config_logging_convert_name_on_off(object->video_transmitter.feedback_control));
		syslog(LOG_NOTICE, "# Feedback Interval              : %d msec", object->video_transmitter.feedback_interval);
		syslog(LOG_NOTICE, "# Video Adaptive Control         : %s", _HPVT_Config_logging_convert_name_on_off(object->video_adaptive_control));
		syslog(LOG_NOTICE, "#  Measurement Interval          : %d sec", object->video_transmitter.adaptive_control_param.measurement_interval);
		syslog(LOG_NOTICE, "#  Limit Of Variation            : %d", object->video_transmitter.adaptive_control_param.limit_amount);
		syslog(LOG_NOTICE, "#  Bitrate Increase Interval     : %d sec", object->video_transmitter.adaptive_control_param.interval_threshold);
		syslog(LOG_NOTICE, "#  Unreceived Rate               : %d.%d %%", object->video_transmitter.adaptive_control_param.unreceived_rate_threshold / 10, //
		object->video_transmitter.adaptive_control_param.unreceived_rate_threshold % 10);
		syslog(LOG_NOTICE, "#  Min Bitrate                   : %d kbps", object->video_transmitter.adaptive_control_param.min_bitrate);
	}
	else if (object->connection.type == HPVT_Config_TYPE_RECEIVER) {
		syslog(LOG_NOTICE, "##[Receiver]");
		syslog(LOG_NOTICE, "# Buffering Delay                : %d msec", object->video_receiver.buffering_delay);
	}
	else {
		HPVT_logging(LOG_WARNING, "# Type                  : %d(Unknown!)", object->connection.type);
	}

	syslog(LOG_NOTICE, "# Preview Full Screen            : %s", _HPVT_Config_logging_convert_name_on_off(object->preview.full_screen));
}
