#ifndef CONFIG_CONFIG_S_H_
#define CONFIG_CONFIG_S_H_

#include "../common_s.h"
#include "../logger/logger.h"

#define HPVT_Config_MAX_LINE_LENGTH				(4096)

#define HPVT_Config_COMMENTOUT_1 				'#'
#define HPVT_Config_COMMENTOUT_2 				'!'
#define HPVT_Config_COMMENTOUT_3 				';'

#define HPVT_Config_DELIMITER 					'='

#define HPVT_Config_LOAD_REGULAR				(0)
#define HPVT_Config_LOAD_LOGGING 				(1)

#define HPVT_Config_VALUE_NOT_SET				(65535)

typedef int16_t HPVT_Config_CONNECTION_MODE;
#define HPVT_Config_CONNECTION_MODE_NONE						(0x00)
#define HPVT_Config_CONNECTION_MODE_INIT_TRANSMITTER			(0x01)
#define HPVT_Config_CONNECTION_MODE_INIT_TRANSMITTER_NAME		"initTransmitter"
#define HPVT_Config_CONNECTION_MODE_INIT_RECEIVER				(0x02)
#define HPVT_Config_CONNECTION_MODE_INIT_RECEIVER_NAME			"initReceiver"
#define HPVT_Config_CONNECTION_MODE_CONNECTION_LESS				(0x03)
#define HPVT_Config_CONNECTION_MODE_CONNECTION_LESS_NAME		"connectionless"
#define HPVT_Config_CONNECTION_MODE_DEFAULT						HPVT_Config_CONNECTION_MODE_INIT_TRANSMITTER

#define HPVT_Config_UDP_PORT_DEFAULT				(65535)

typedef int16_t HPVT_Config_TYPE;
#define HPVT_Config_TYPE_NONE						(0x00)
#define HPVT_Config_TYPE_TRANSMITTER				(0x01)
#define HPVT_Config_TYPE_TRANSMITTER_NAME			"transmitter"
#define HPVT_Config_TYPE_RECEIVER					(0x02)
#define HPVT_Config_TYPE_RECEIVER_NAME				"receiver"
#define HPVT_Config_TYPE_DEFAULT					HPVT_Config_TYPE_RECEIVER

typedef int16_t HPVT_Config_CAMERA_ID;
#define HPVT_Config_CAMERA_ID_NONE					    (0x00)
#define HPVT_Config_CAMERA_ID_0							(0x00)
#define HPVT_Config_CAMERA_ID_0_NAME					"0"
#define HPVT_Config_CAMERA_ID_1							(0x01)
#define HPVT_Config_CAMERA_ID_1_NAME					"1"
#define HPVT_Config_CAMERA_ID_DEFAULT					HPVT_Config_CAMERA_ID_0

typedef uint16_t HPVT_Config_MTU_SIZE;
#define HPVT_Config_MTU_SIZE_MIN_NUM				(512)
#define HPVT_Config_MTU_SIZE_MAX_NUM				(1500)
#define HPVT_Config_MTU_SIZE_DEFAULT				(1200)

#define HPVT_Config_PATH_MTU_DISCOVERY_MINIMUM_SIZE_DEFAULT			(512)
#define HPVT_Config_PATH_MTU_DISCOVERY_MAXIMUM_SIZE_DEFAULT			(1500)

typedef uint16_t HPVT_Config_TIMEOUT;
#define HPVT_Config_TIMEOUT_MIN_NUM					(0)
#define HPVT_Config_TIMEOUT_MAX_NUM					(600)
#define HPVT_Config_TIMEOUT_DEFAULT					(10)

typedef uint16_t HPVT_Config_RETRY_WAIT;
#define HPVT_Config_RETRY_WAIT_MIN_NUM				(1)
#define HPVT_Config_RETRY_WAIT_MAX_NUM				(60)
#define HPVT_Config_RETRY_WAIT_DEFAULT				(1)

typedef uint16_t HPVT_Config_FEEDBACK_INTERVAL;
#define HPVT_Config_FEEDBACK_INTERVAL_MIN_NUM				(100)
#define HPVT_Config_FEEDBACK_INTERVAL_MAX_NUM				(10000)
#define HPVT_Config_FEEDBACK_INTERVAL_DEFAULT				(500)

typedef int16_t HPVT_Config_PACKET_SIZE_DETERMINATION_MODE;
#define HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_NONE					(0x00)
#define HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_STANDARD				(0x01)
#define HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_STANDARD_NAME		"standard"
#define HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_FIXED				(0x02)
#define HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_FIXED_NAME			"fixedSize"
#define HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_AUTO_DISCOVERY		(0x03)
#define HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_AUTO_DISCOVERY_NAME	"autoDiscovery"

#define HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_DEFAULT              HPVT_Config_PACKET_SIZE_DETERMINATION_MODE_AUTO_DISCOVERY

typedef int16_t HPVT_Config_FUNCTION;
#define HPVT_Config_FUNCTION_NONE				(0x00)
#define HPVT_Config_FUNCTION_ON					(0x01)
#define HPVT_Config_FUNCTION_ON_NAME			"on"
#define HPVT_Config_FUNCTION_OFF				(0x02)
#define HPVT_Config_FUNCTION_OFF_NAME			"off"

#define HPVT_Config_FEC_LEVEL_MIN_NUM       (2)
#define HPVT_Config_FEC_LEVEL_MAX_NUM       (10)
#define HPVT_Config_FEC_LEVEL_DEFAULT       (9)

#define HPVT_Config_CAMERA_WIDTH_MIN_NUM              (32)
#define HPVT_Config_CAMERA_WIDTH_MAX_NUM              (1920)
#define HPVT_Config_CAMERA_WIDTH_DEFAULT              (1280)

#define HPVT_Config_CAMERA_HEIGHT_MIN_NUM             (32)
#define HPVT_Config_CAMERA_HEIGHT_MAX_NUM             (1080)
#define HPVT_Config_CAMERA_HEIGHT_DEFAULT             (720)

#define HPVT_Config_CAMERA_FRAMERATE_MIN_NUM          (100)
#define HPVT_Config_CAMERA_FRAMERATE_MAX_NUM          (60000)
#define HPVT_Config_CAMERA_FRAMERATE_DEFAULT          (30000)

#define HPVT_Config_CAMERA_ROTATION_DEFAULT           (0)

#define HPVT_Config_CAMERA_SHARPNESS_MIN_NUM          (0)
#define HPVT_Config_CAMERA_SHARPNESS_MAX_NUM          (2000)
#define HPVT_Config_CAMERA_SHARPNESS_DEFAULT          (1000)

#define HPVT_Config_CAMERA_BRIGHTNESS_MIN_NUM         (-1000)
#define HPVT_Config_CAMERA_BRIGHTNESS_MAX_NUM         (1000)
#define HPVT_Config_CAMERA_BRIGHTNESS_DEFAULT         (0)

#define HPVT_Config_CAMERA_CONTRAST_MIN_NUM        	  (0)
#define HPVT_Config_CAMERA_CONTRAST_MAX_NUM        	  (2000)
#define HPVT_Config_CAMERA_CONTRAST_DEFAULT        	  (1000)

#define HPVT_Config_CAMERA_SATURATION_MIN_NUM         (0)
#define HPVT_Config_CAMERA_SATURATION_MAX_NUM         (2000)
#define HPVT_Config_CAMERA_SATURATION_DEFAULT         (1000)

#define HPVT_Config_CAMERA_SHUTTER_SPEED_MIN_NUM      (0)
#define HPVT_Config_CAMERA_SHUTTER_SPEED_MAX_NUM      (1000000)
#define HPVT_Config_CAMERA_SHUTTER_SPEED_DEFAULT      (0)

#define HPVT_Config_CAMERA_ANALOG_GAIN_MIN_NUM        (0)
#define HPVT_Config_CAMERA_ANALOG_GAIN_MAX_NUM        (64000)
#define HPVT_Config_CAMERA_ANALOG_GAIN_DEFAULT        (0)

#define HPVT_Config_CAMERA_EV_COMPENSATION_MIN_NUM    (-8000)
#define HPVT_Config_CAMERA_EV_COMPENSATION_MAX_NUM    (8000)
#define HPVT_Config_CAMERA_EV_COMPENSATION_DEFAULT    (0)

#define HPVT_Config_VIDEO_BITRATE_MIN_NUM             (20)
#define HPVT_Config_VIDEO_BITRATE_MAX_NUM             (20000)
#define HPVT_Config_VIDEO_BITRATE_DEFAULT             (10000)

#define HPVT_Config_VIDEO_FRAMERATE_MIN_NUM           (1)
#define HPVT_Config_VIDEO_FRAMERATE_MAX_NUM           (60)
#define HPVT_Config_VIDEO_FRAMERATE_DEFAULT           (30)

#define HPVT_Config_VIDEO_IDR_PERIOD_MIN_NUM          (1)
#define HPVT_Config_VIDEO_IDR_PERIOD_MAX_NUM          (3600)
#define HPVT_Config_VIDEO_IDR_PERIOD_DEFAULT          (30)

typedef int16_t HPVT_Config_VIDEO_PROFILE_TYPE;
#define HPVT_Config_VIDEO_PROFILE_NONE				(0x00)
#define HPVT_Config_VIDEO_PROFILE_BASELINE			(0x01)
#define HPVT_Config_VIDEO_PROFILE_BASELINE_NAME		"baseline"
#define HPVT_Config_VIDEO_PROFILE_MAIN				(0x02)
#define HPVT_Config_VIDEO_PROFILE_MAIN_NAME			"main"
#define HPVT_Config_VIDEO_PROFILE_HIGH				(0x03)
#define HPVT_Config_VIDEO_PROFILE_HIGH_NAME			"high"
#define HPVT_Config_VIDEO_PROFILE_DEFAULT          HPVT_Config_VIDEO_PROFILE_HIGH

typedef int16_t HPVT_Config_CAMERA_FLIP;
#define HPVT_Config_CAMERA_FLIP_NONE				(0x00)
#define HPVT_Config_CAMERA_FLIP_OFF_NAME			"off"
#define HPVT_Config_CAMERA_FLIP_VERTICAL			(0x01)
#define HPVT_Config_CAMERA_FLIP_VERTICAL_NAME		"vertical"
#define HPVT_Config_CAMERA_FLIP_HORIZONTAL			(0x02)
#define HPVT_Config_CAMERA_FLIP_HORIZONTAL_NAME		"horizontal"
#define HPVT_Config_CAMERA_FLIP_BOTH				(0x03)
#define HPVT_Config_CAMERA_FLIP_BOTH_NAME			"both"
#define HPVT_Config_CAMERA_FLIP_DEFAULT         	 HPVT_Config_CAMERA_FLIP_NONE

typedef int16_t HPVT_Config_CAMERA_WHITE_BALANCE_MODE;
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_AUTO				(0x00)
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_AUTO_NAME			"auto"
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_INCANDESCENT		(0x01)
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_INCANDESCENT_NAME	"incandescent"
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_TUNGSTEN			(0x02)
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_TUNGSTEN_NAME		"tungsten"
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_FLUORESCENT		(0x03)
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_FLUORESCENT_NAME	"fluorescent"
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_INDOOR			(0x04)
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_INDOOR_NAME		"indoor"
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_DAYLIGHT			(0x05)
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_DAYLIGHT_NAME		"daylight"
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_CLOUDY			(0x06)
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_CLOUDY_NAME		"cloudy"
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_CUSTOM			(0x07)
#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_CUSTOM_NAME		"custom"

#define HPVT_Config_CAMERA_WHITE_BALANCE_MODE_DEFAULT			HPVT_Config_CAMERA_WHITE_BALANCE_MODE_AUTO

#define HPVT_Config_CAMERA_WHITE_BALANCE_GAIN_MIN_NUM		(0)
#define HPVT_Config_CAMERA_WHITE_BALANCE_GAIN_MAX_NUM		(32000)
#define HPVT_Config_CAMERA_WHITE_BALANCE_RED_GAIN_DEFAULT	(0)
#define HPVT_Config_CAMERA_WHITE_BALANCE_BLUE_GAIN_DEFAULT	(0)

typedef int16_t HPVT_Config_CAMERA_EXPOSURE_MODE;
#define HPVT_Config_CAMERA_EXPOSURE_MODE_NORMAL			(0x00)
#define HPVT_Config_CAMERA_EXPOSURE_MODE_NORMAL_NAME	"normal"
#define HPVT_Config_CAMERA_EXPOSURE_MODE_SHORT			(0x01)
#define HPVT_Config_CAMERA_EXPOSURE_MODE_SHORT_NAME		"short"
#define HPVT_Config_CAMERA_EXPOSURE_MODE_LONG			(0x02)
#define HPVT_Config_CAMERA_EXPOSURE_MODE_LONG_NAME		"long"

#define HPVT_Config_CAMERA_EXPOSURE_MODE_DEFAULT			HPVT_Config_CAMERA_EXPOSURE_MODE_NORMAL

typedef int16_t HPVT_Config_CAMERA_METERING_MODE;
#define HPVT_Config_CAMERA_METERING_MODE_CENTRE				(0x00)
#define HPVT_Config_CAMERA_METERING_MODE_CENTRE_NAME		"centre"
#define HPVT_Config_CAMERA_METERING_MODE_SPOT				(0x01)
#define HPVT_Config_CAMERA_METERING_MODE_SPOT_NAME			"spot"
#define HPVT_Config_CAMERA_METERING_MODE_MATRIX				(0x02)
#define HPVT_Config_CAMERA_METERING_MODE_MATRIX_NAME		"matrix"

#define HPVT_Config_CAMERA_METERING_MODE_DEFAULT			HPVT_Config_CAMERA_METERING_MODE_CENTRE

#define HPVT_Config_VIDEO_BUFFERING_DELAY_MIN_NUM      (50)
#define HPVT_Config_VIDEO_BUFFERING_DELAY_MAX_NUM      (5000)
#define HPVT_Config_VIDEO_BUFFERING_DELAY_DEFAULT      (300)

#define HPVT_Config_AC_MEASUREMENT_DURATION_MIN_NUM			(5)
#define HPVT_Config_AC_MEASUREMENT_DURATION_MAX_NUM			(60)
#define HPVT_Config_AC_MEASUREMENT_DURATION_DEFAULT			(15)

#define HPVT_Config_AC_CHANGE_LIMIT_AMOUNT_MIN_NUM			(2)
#define HPVT_Config_AC_CHANGE_LIMIT_AMOUNT_MAX_NUM			(8)
#define HPVT_Config_AC_CHANGE_LIMIT_AMOUNT_DEFAULT			(2)

#define HPVT_Config_AC_INTERVAL_MIN_NUM						(30)
#define HPVT_Config_AC_INTERVAL_MAX_NUM						(600)
#define HPVT_Config_AC_INTERVAL_DEFAULT						(60)

#define HPVT_Config_AC_LOSS_RATE_MIN_NUM					(1)
#define HPVT_Config_AC_LOSS_RATE_MAX_NUM					(500)
#define HPVT_Config_AC_LOSS_RATE_DEFAULT					(5)

#define HPVT_Config_AC_MIN_BITRATE_MIN_NUM					(50)
#define HPVT_Config_AC_MIN_BITRATE_MAX_NUM					(10000)
#define HPVT_Config_AC_MIN_BITRATE_DEFAULT					(50)

#define HPVT_CONFIG_KEY_LOG_FACILITY				"LogFacility"
#define HPVT_CONFIG_KEY_LOG_LEVEL					"LogLevel"

#define HPVT_CONFIG_KEY_TYPE						"Type"
#define HPVT_CONFIG_KEY_CONNECTION_MODE				"ConnectionMode"
#define HPVT_CONFIG_KEY_CONNECTION_IP				"ConnectionIP"
#define HPVT_CONFIG_KEY_CONNECTION_PORT				"ConnectionPort"
#define HPVT_CONFIG_KEY_LISTEN_PORT					"ListenPort"
#define HPVT_CONFIG_KEY_TIMEOUT						"Timeout"
#define HPVT_CONFIG_KEY_RETRY_WAIT					"RetryWait"
#define HPVT_CONFIG_KEY_PACE_CONTROL				"PaceControl"

#define HPVT_CONFIG_KEY_VIDEO_CAMERA_ID				"VideoCameraID"

#define HPVT_CONFIG_KEY_CAMERA_CAPTURE_WIDTH			"CameraCaptureWidth"
#define HPVT_CONFIG_KEY_CAMERA_CAPTURE_HEIGHT			"CameraCaptureHeight"
#define HPVT_CONFIG_KEY_CAMERA_CAPTURE_FRAMERATE		"CameraCaptureFramerate"
#define HPVT_CONFIG_KEY_CAMERA_FLIP						"CameraFlip"
#define HPVT_CONFIG_KEY_CAMERA_ROTATION					"CameraRotation"
#define HPVT_CONFIG_KEY_CAMERA_SHARPNESS				"CameraSharpness"
#define HPVT_CONFIG_KEY_CAMERA_CONTRAST					"CameraContrast"
#define HPVT_CONFIG_KEY_CAMERA_BRIGHTNESS				"CameraBrightness"
#define HPVT_CONFIG_KEY_CAMERA_SATURATION				"CameraSaturation"
#define HPVT_CONFIG_KEY_CAMERA_SHUTTER_SPEED			"CameraShutterSpeed"
#define HPVT_CONFIG_KEY_CAMERA_ANALOG_GAIN				"CameraAnalogGain"
#define HPVT_CONFIG_KEY_CAMERA_EXPOSURE_MODE			"CameraExposureMode"
#define HPVT_CONFIG_KEY_CAMERA_EV_COMPENSATAION			"CameraEVCompensation"
#define HPVT_CONFIG_KEY_CAMERA_METERING_MODE			"CameraMeteringMode"
#define HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_MODE		"CameraWhiteBalanceMode"
#define HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_RED_GAIN	"CameraWhiteBalanceRedGain"
#define HPVT_CONFIG_KEY_CAMERA_WHITE_BALANCE_BLUE_GAIN	"CameraWhiteBalanceBlueGain"

#define HPVT_CONFIG_KEY_VIDEO_FRAMERATE					"VideoFramerate"
#define HPVT_CONFIG_KEY_VIDEO_BITRATE					"VideoBitrate"
#define HPVT_CONFIG_KEY_VIDEO_I_FRAME_INTERVAL			"VideoIFrameInterval"
#define HPVT_CONFIG_KEY_VIDEO_PREVIEW					"VideoPreview"
#define HPVT_CONFIG_KEY_VIDEO_PROFILE					"VideoProfile"

#define HPVT_CONFIG_KEY_PREVIEW_FULL_SCREEN				"PreviewFullScreen"

#define HPVT_CONFIG_KEY_MTU_SIZE						"MTUSize"
#define HPVT_CONFIG_KEY_PACKET_SIZE_DETERMINATION_MODE	"PacketSizeDeterminationMode"
#define HPVT_CONFIG_KEY_PATH_MTU_DISCOVERY_MIN_SIZE		"PathMTUDiscoveryMinSize"
#define HPVT_CONFIG_KEY_PATH_MTU_DISCOVERY_MAX_SIZE		"PathMTUDiscoveryMaxSize"
#define HPVT_CONFIG_KEY_FEC_LEVEL						"FECLevel"
#define HPVT_CONFIG_KEY_BUFFERING_DELAY					"BufferingDelay"
#define HPVT_CONFIG_KEY_FEEDBACK_CONTROL				"FeedbackControl"
#define HPVT_CONFIG_KEY_FEEDBACK_INTERVAL				"FeedbackInterval"
#define HPVT_CONFIG_KEY_ADAPTIVE_CONTROL				"AdaptiveControl"
#define HPVT_CONFIG_KEY_AC_MEASUREMENT_INTERVAL			"ACMeasurementInterval"
#define HPVT_CONFIG_KEY_AC_LIMIT_OF_VARIATION			"ACLimitOfVariation"
#define HPVT_CONFIG_KEY_AC_INTERVAL_THRESHOLD			"ACBitrateIncreaseInterval"
#define HPVT_CONFIG_KEY_AC_UNRECEIVED_RATE_THRESHOLD	"ACBitrateIncreaseUnreceivedRate"
#define HPVT_CONFIG_KEY_AC_MIN_BITRATE					"ACMinBitrate"


typedef struct HPVT_Config_s HPVT_Config;
struct HPVT_Config_s {

	struct {
		LOG_FACILITY facility;
		LOG_LEVEL level;
	} log;
	char * config_file;

	struct {
		HPVT_Config_TYPE type;
		HPVT_Config_CONNECTION_MODE connection_mode;

		struct {
			uint16_t timeout;
			uint16_t retry_wait;
		} server;

		uint32_t host_ip;
		uint16_t host_port;
		uint16_t listen_port;

	} connection;

	struct {
		uint16_t mtu_size;
		HPVT_Config_PACKET_SIZE_DETERMINATION_MODE packet_size_determination_mode;
		uint16_t path_mtu_discovery_min_size;
		uint16_t path_mtu_discovery_max_size;

		HPVT_Config_CAMERA_ID camera_id;

		HPVT_Config_FUNCTION pace_control;
		HPVT_Config_FUNCTION feedback_control;
		uint16_t feedback_interval;

		HPVT_Config_FUNCTION fec_enabled;
		uint16_t fec_level;

		uint32_t bitrate;
		uint32_t framerate;
		uint32_t idr_period;
		uint16_t profile;

		HPVT_Config_FUNCTION preview_enabled;

		struct {
			uint16_t measurement_interval;
			uint16_t limit_amount;
			uint16_t interval_threshold;
			uint16_t unreceived_rate_threshold;
			uint32_t min_bitrate;
		} adaptive_control_param;

		struct {
			uint16_t width;
			uint16_t height;
			uint32_t framerate;
			uint16_t rotation;
			uint16_t flip;
			uint16_t sharpness;
			uint16_t contrast;
			int32_t brightness;
			uint16_t saturation;
			uint32_t shutter_speed;
			uint16_t analog_gain;
			uint16_t exposure_mode;
			int32_t exposure_compensation;
			uint16_t metering_mode;

			struct {
				uint16_t mode;
				uint16_t red_gain;
				uint16_t blue_gain;
			} white_balance;
		} camera;
	} video_transmitter;

	struct {
		uint32_t buffering_delay;
	} video_receiver;

	HPVT_Config_FUNCTION video_adaptive_control;

	struct {
		HPVT_Config_FUNCTION full_screen;
	} preview;

};

#endif /* CONFIG_CONFIG_S_H_ */
