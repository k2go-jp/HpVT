#ifndef NETWORK_CONTEXT_S_H_
#define NETWORK_CONTEXT_S_H_

#include "../common_s.h"
#include "../logger/logger_s.h"

/** Camera ID definitions */
#define HPVT_CAMERA_ID    int
#define HPVT_CAMERA_ID_0  (0)   /* first camera */
#define HPVT_CAMERA_ID_1  (1)   /* second camera */

/** Rotation angle definitions */
#define HPVT_CAMERA_ROTATION         int
#define HPVT_CAMERA_ROTATION_NONE    (0)     /* Not rotate */
#define HPVT_CAMERA_ROTATION_90      (90)    /* Rotate 90 degrees */
#define HPVT_CAMERA_ROTATION_180     (180)   /* Rotate 180 degrees */
#define HPVT_CAMERA_ROTATION_270     (270)   /* Rotate 270 degrees */

/** Flip type definitions */
#define HPVT_CAMERA_FLIP             int
#define HPVT_CAMERA_FLIP_NONE        (0)   /* Not flip */
#define HPVT_CAMERA_FLIP_VERTICAL    (1)   /* vertical flip */
#define HPVT_CAMERA_FLIP_HORIZONTAL  (2)   /* horizontal flip */
#define HPVT_CAMERA_FLIP_BOTH        (3)   /* vertical and horizontal flip */

/** Auto white balance mode definitions */
#define HPVT_CAMERA_AWB_MODE               int
#define HPVT_CAMERA_AWB_MODE_AUTO          (0)  /* automatic mode */
#define HPVT_CAMERA_AWB_MODE_INCANDESCENT  (1)  /* incandescent lighting mode */
#define HPVT_CAMERA_AWB_MODE_TUNGSTEN      (2)  /* tungsten lighting mode */
#define HPVT_CAMERA_AWB_MODE_FLUORESCENT   (3)  /* fluorescent lighting mode */
#define HPVT_CAMERA_AWB_MODE_INDOOR        (4)  /* indoor mode */
#define HPVT_CAMERA_AWB_MODE_SUNLIGHT      (5)  /* sunny mode */
#define HPVT_CAMERA_AWB_MODE_CLOUDY        (6)  /* cloudy mode */
#define HPVT_CAMERA_AWB_MODE_CUSTOM        (7)  /* custom white balance */

/** Metering mode definitions */
#define HPVT_CAMERA_METERING_MODE          int
#define HPVT_CAMERA_METERING_MODE_CENTRE   (0)  /* centre metering */
#define HPVT_CAMERA_METERING_MODE_SPOT     (1)  /* spot metering */
#define HPVT_CAMERA_METERING_MODE_MATRIX   (2)  /* matrix metering */

/** Exposure mode definitions */
#define HPVT_CAMERA_EXPOSURE_MODE             int
#define HPVT_CAMERA_EXPOSURE_MODE_NORMAL      (0)    /* use automatic exposure mode */
#define HPVT_CAMERA_EXPOSURE_MODE_SHORT       (1)    /* select setting for sports (fast shutter etc.) */
#define HPVT_CAMERA_EXPOSURE_MODE_LONG        (2)   /* select setting for long exposures */

/** Camera shutter speed definitions */
#define HPVT_CAMERA_SHUTTER_SPEED_AUTO  (0)  /* Automatic */

/** H.264/AVC profile definitions*/
#define HPVT_H264_AVC_PROFILE_TYPE          int
#define HPVT_H264_AVC_PROFILE_BASELINE      (1)
#define HPVT_H264_AVC_PROFILE_MAIN          (2)
#define HPVT_H264_AVC_PROFILE_HIGH          (8)

/** Video transmission session state definitions */
#define HPVT_VIDEO_CONNECTION_STATE               int
#define HPVT_VIDEO_CONNECTION_STATE_CLOSED        (0)   /* disabled connection attempt */
#define HPVT_VIDEO_CONNECTION_STATE_DISCONNECTED  (1)   /* disconnected the connection */
#define HPVT_VIDEO_CONNECTION_STATE_CONNECTING    (2)   /* trying to connect to other host */
#define HPVT_VIDEO_CONNECTION_STATE_WAITING       (3)   /* waiting for a connection from other host */
#define HPVT_VIDEO_CONNECTION_STATE_CONNECTED     (8)   /* connected with other host */

/** Definition of camera setting parameters */
typedef struct hpvt_camera_parameters_s {
	int width;
	int height;
	float framerate;
	float sharpness;
	float contrast;
	float brightness;
	float saturation;
	int shutter_speed;
	float analog_gain;
	HPVT_CAMERA_EXPOSURE_MODE exposure_mode;
	float exposure_compensation;
	HPVT_CAMERA_AWB_MODE awb_mode;
	float awb_red_gain;
	float awb_blue_gain;
	HPVT_CAMERA_METERING_MODE metering_mode;
	HPVT_CAMERA_ROTATION rotation;
	HPVT_CAMERA_FLIP flip;
} HPVT_CAMERA_PARAMETERS;

/** Definition of network traffic information */
typedef struct HPVT_NETWORK_TRAFFIC_INFO {
	struct {
		struct {
			uint64_t packets;
			uint64_t bytes;
		} control;
		struct {
			uint64_t packets;
			uint64_t bytes;
		} payload;
		struct {
			uint64_t packets;
			uint64_t bytes;
		} redundancy;
		struct {
			uint64_t packets;
			uint64_t bytes;
		} dropped;
		struct {
			uint64_t packets;
			uint64_t bytes;
		} cancel;
	} sent;
	struct {
		struct {
			uint64_t packets;
			uint64_t bytes;
		} control;
		struct {
			uint64_t packets;
			uint64_t bytes;
		} payload;
		struct {
			uint64_t packets;
			uint64_t bytes;
		} redundancy;
		struct {
			uint64_t packets;
		} unexpected;
		struct {
			uint64_t packets;
		} reordering;
		struct {
			struct {
				uint64_t packets;
				uint64_t bytes;
			} total;
			struct {
				uint64_t packets;
				uint64_t bytes;
			} duplicate;
			struct {
				uint64_t packets;
				uint64_t bytes;
			} overrun;
			struct {
				uint64_t packets;
				uint64_t bytes;
			} outofrange;
			struct {
				uint64_t packets;
				uint64_t bytes;
			} invalid;
			struct {
				uint64_t packets;
				uint64_t bytes;
			} broken;
		} dropped;
	} received;
} HPVT_NETWORK_TRAFFIC_INFO;

typedef struct HPVT_global_context {

	struct {

		pthread_mutex_t settings_locked;

		struct {
			int mtu_size;
			char packet_size_determination_mode;
			int path_mtu_discovery_min_size;
			int path_mtu_discovery_max_size;

			char adaptive_control;
			struct {
				uint16_t measurement_interval;
				uint16_t limit_amount;
				uint16_t interval_threshold;
				uint16_t unreceived_rate_threshold;
				uint32_t min_bitrate;
				int max_bitrate;
			} adaptive_control_param;

			char feedback_control;
			int feedback_interval;

			char pace_control;
			boolean fec_enabled;
			int fec_level;
			int bitrate;
			int encode_framerate;
			int i_frame_interval;
			int profile;

			int timeout_threshold;
			int timeout_retry_wait;

			int buffering_delay;
			boolean preview_enabled;

			struct {
				uint16_t width;
				uint16_t height;
				float framerate;
			} camera;

			struct {
				boolean full_screen;
			} preview;
		} video;

		struct {
			char type;
			char connection_mode;
		} system;

	} settings;

	struct {

		pthread_mutex_t status_locked;

		int state;
		int timeout_count;
		int no_callback_count;

		boolean flag_timeout;
		boolean flag_send_wait;
		boolean flag_reset_resolution;
		uint16_t current_resolution_width;
		uint16_t current_resolution_height;
		uint32_t current_framerate;

		struct {
			int receivable_size;
			int try_maximum_size;
			boolean flag_start_discovery;
		} path_mtu_discovery;

		struct {
			HPVT_Queue_FRAME_SEQNO seqno;
			uint64_t captured_time;

			struct {
				uint16_t bitrate;
			} adaptive;

			uint8_t flags;
		} internal_info;

		uint64_t frame_generated_time;

		HPVT_Queue_FRAME_SEQNO processing_seqno;
		HPVT_Queue_FRAME_SEQNO send_recv_seqno;

		struct {
			uint32_t round_trip_time;
			uint32_t loss_rate;
			uint64_t send_throughput;
			uint64_t receive_throughput;
		} feedback;

		struct {
			uint32_t round_trip_time;
			uint32_t loss_rate;
			uint64_t send_throughput;
			uint64_t receive_throughput;
		} self;

		boolean flag_change_framerate;
		boolean reconnection_attempt_on;

		struct {
			uint32_t addr;
			uint16_t port;
			uint8_t version_major;
			uint8_t version_minor;
			uint8_t version_rivision;
		} peer;

	} connection;

	int ready;
	HPVT_CAMERA_PARAMETERS camera_params[2];

	char *config_path;
	boolean termination;

} HPVT_Context;

#endif /* NETWORK_CONTEXT_S_H_ */
