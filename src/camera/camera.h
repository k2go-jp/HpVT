#ifndef CAMERA_CAMERA_H_
#define CAMERA_CAMERA_H_

#include "../common_s.h"
#include "../config/config_s.h"

uint32_t HpVT_round_value(uint32_t value, uint32_t multiplier);
void HPVT_start_standard_camera_encoding(HPVT_Config_CAMERA_ID camera_id);
boolean HPVT_activate_standard_camera_capture(void);
boolean HPVT_inactivate_standard_camera_capture(void);
boolean HpVT_libcam_set_framerate(float framerate);
boolean HpVT_libcam_set_brightness(float brightness);
boolean HpVT_libcam_set_saturation(float saturation);
boolean HpVT_libcam_set_sharpness(float sharpness);
boolean HpVT_libcam_set_contrast(float contrast);
boolean HpVT_libcam_set_shutter_speed(int shutter_speed);
boolean HpVT_libcam_set_analog_gain(float gain);
boolean HpVT_libcam_set_awb_mode(int awb_mode);
boolean HpVT_libcam_set_awb_gains(float awb_red_gain, float awb_blue_gain);
boolean HpVT_libcam_set_exposure_mode(int exposure_mode);
boolean HpVT_libcam_set_ev_compensation(float ev_compensation);
boolean HpVT_libcam_set_metering_mode(int metering_mode);
boolean HpVT_libcam_set_video_bitrate(int bitrate);
boolean HpVT_libcam_set_video_period(int interval);
boolean HpVT_libcam_set_video_profile(HPVT_Config_VIDEO_PROFILE_TYPE profile);
boolean HpVT_libcam_set_video_framerate(float framerate);

#endif /* CAMERA_CAMERA_H_ */
