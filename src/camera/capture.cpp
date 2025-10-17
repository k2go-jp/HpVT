#define FILE_NUMBER "C201"

#include "../config/config.h"
#include "../network/context_s.h"
#include "../network/queue/queue.h"
#include "../preview/preview.hpp"
#include "h264_encoder.hpp"
#include "stream_info.hpp"
#include "camera.h"

#include <sys/mman.h>
#include <libcamera/libcamera.h>

using namespace libcamera;

extern HPVT_Context *g_context;

static std::shared_ptr<Camera> camera;
static std::unique_ptr<Preview> preview;
static std::unique_ptr<H264Encoder> encoder;
static pthread_mutex_t request_ptr_locked;
static Request *request_ptr = nullptr;

static boolean is_activated = false;
static uint32_t frame_width = 0;
static uint32_t frame_height = 0;
static uint32_t frame_stride = 0;
static uint32_t frame_height_encoded = 0;

static void _HPVT_init_request_ptr_mutex(void) {

	static boolean init_flag = false;

	if (init_flag == false) {
		if (pthread_mutex_init(&request_ptr_locked, 0)) {
			HPVT_log_UNEXPECTED_ERROR();
		}
		init_flag = true;
	}
}
static void createdFrameBufferCallback(void* mem, size_t size, int64_t timestamp, bool keyframe) {

	HPVT_enqueue_video_frame_encoded(mem, size, keyframe, false);
}
static void previewDoneCallback(int fd) {
}
static void requestComplete(Request *request) {
	if (request->status() == Request::RequestCancelled) {
		return;
	}

	const Request::BufferMap &buffers = request->buffers();
	for (auto bufferPair : buffers) {
		FrameBuffer *buffer = bufferPair.second;
		const FrameMetadata &metadata = buffer->metadata();
		int buffer_fd = buffer->planes()[0].fd.get();
		size_t total_size = metadata.planes()[0].bytesused + metadata.planes()[1].bytesused + metadata.planes()[2].bytesused;
		void *buffer_ptr = mmap(nullptr, total_size, PROT_READ, MAP_SHARED, buffer_fd, 0);

		if (buffer_ptr == MAP_FAILED) {
			HPVT_logging(LOG_LEVEL_ERROR, "camera buffer mmap failed");
			return;
		}

		if (g_context->settings.video.preview_enabled == true) {
			StreamInfo info = { };
			info.width = frame_width;
			info.height = frame_height;
			info.stride = frame_stride;
			info.height_encoded = frame_height_encoded;
			info.pixel_format = formats::YUV420;
			info.colour_space = libcamera::ColorSpace::Rec709;
			preview->Show(buffer_fd, total_size, info);
		}

		encoder->EncodeBuffer(buffer_fd, total_size, buffer_ptr, metadata.timestamp / 1000);

		if (munmap(buffer_ptr, total_size) < 0) {
			HPVT_logging(LOG_LEVEL_ERROR, "camera buffer munmap failed");
		}
	}

	request->reuse(Request::ReuseBuffers);
	camera->queueRequest(request);

	HPVT_Lock(request_ptr_locked);
	request_ptr = request;
	HPVT_Unlock(request_ptr_locked);

}
static void _HPVT_set_camera_transform(libcamera::Transform& tf, int camera_id) {

	uint32_t rotation = g_context->camera_params[camera_id].rotation;
	uint32_t flip = g_context->camera_params[camera_id].flip;

	if (rotation == 0) {
		tf = Transform::Rot0 * tf;
	}
	else if (rotation == 180) {
		tf = Transform::Rot180 * tf;
	}

	if (flip == HPVT_Config_CAMERA_FLIP_HORIZONTAL) {
		tf = Transform::HFlip * tf;
	}
	else if (flip == HPVT_Config_CAMERA_FLIP_VERTICAL) {
		tf = Transform::VFlip * tf;
	}
	else if (flip == HPVT_Config_CAMERA_FLIP_BOTH) {
		tf = Transform::HVFlip * tf;
	}

}
static void _HPVT_set_camera_parameters(ControlList& ctrl_list, int camera_id) {

	int64_t frame_duration = (1000 * 1000 * 1000) / (int) (g_context->camera_params[camera_id].framerate * 1000.0);

	HPVT_logging(LOG_LEVEL_NOTICE, "libcam set framerate = %.2f FrameDurationLimits = %lld usec\n", //
			g_context->camera_params[camera_id].framerate, frame_duration);
	ctrl_list.set(controls::FrameDurationLimits, libcamera::Span<const int64_t, 2>( {
			frame_duration, frame_duration }));

	HPVT_logging(LOG_LEVEL_NOTICE, "libcam set Sharpness = %f\n", g_context->camera_params[camera_id].sharpness);
	ctrl_list.set(controls::Sharpness, g_context->camera_params[camera_id].sharpness);
	//
	HPVT_logging(LOG_LEVEL_NOTICE, "libcam set Contrast = %f\n", g_context->camera_params[camera_id].contrast);
	ctrl_list.set(controls::Contrast, g_context->camera_params[camera_id].contrast);
	//
	HPVT_logging(LOG_LEVEL_NOTICE, "libcam set Brightness = %f\n", g_context->camera_params[camera_id].brightness);
	ctrl_list.set(controls::Brightness, g_context->camera_params[camera_id].brightness);
	//
	HPVT_logging(LOG_LEVEL_NOTICE, "libcam set Saturation = %f\n", g_context->camera_params[camera_id].saturation);
	ctrl_list.set(controls::Saturation, g_context->camera_params[camera_id].saturation);
	//
	int32_t shutter_speed = (int32_t) g_context->camera_params[camera_id].shutter_speed;
	HPVT_logging(LOG_LEVEL_NOTICE, "libcam set shutter_speed = %d\n", shutter_speed);
	ctrl_list.set(controls::ExposureTime, (int32_t) g_context->camera_params[camera_id].shutter_speed);
	//
	HPVT_logging(LOG_LEVEL_NOTICE, "libcam set AnalogueGain = %f\n", g_context->camera_params[camera_id].analog_gain);
	ctrl_list.set(controls::AnalogueGain, g_context->camera_params[camera_id].analog_gain);
	//
	int32_t exposure_mode = g_context->camera_params[camera_id].exposure_mode;
	HPVT_logging(LOG_LEVEL_NOTICE, "libcam set AeExposureMode = %d\n", exposure_mode);
	ctrl_list.set(controls::AeExposureMode, exposure_mode);
	//
	HPVT_logging(LOG_LEVEL_NOTICE, "libcam set ExposureValue = %f\n", g_context->camera_params[camera_id].exposure_compensation);
	ctrl_list.set(controls::ExposureValue, g_context->camera_params[camera_id].exposure_compensation);
	//
	int32_t awb_mode = g_context->camera_params[camera_id].awb_mode;
	HPVT_logging(LOG_LEVEL_NOTICE, "libcam set AwbMode = %d\n", awb_mode);
	ctrl_list.set(controls::AwbMode, awb_mode);
	//

	if (awb_mode == HPVT_Config_CAMERA_WHITE_BALANCE_MODE_CUSTOM) {
		HPVT_logging(LOG_LEVEL_NOTICE, "libcam set AwbEnable false\n");
		HPVT_logging(LOG_LEVEL_NOTICE, "libcam set ColourGains awb_red_gain = %f awb_blue_gain = %f\n", //
				g_context->camera_params[camera_id].awb_red_gain, g_context->camera_params[camera_id].awb_blue_gain);
		ctrl_list.set(controls::AwbEnable, false);
		ctrl_list.set(controls::ColourGains, libcamera::Span<const float, 2>( {
				g_context->camera_params[camera_id].awb_red_gain, g_context->camera_params[camera_id].awb_blue_gain }));
	}
	else {
		HPVT_logging(LOG_LEVEL_NOTICE, "libcam set AwbEnable true\n");
		ctrl_list.set(controls::AwbEnable, true);
	}
	//
	int32_t metering_mode = g_context->camera_params[camera_id].metering_mode;
	HPVT_logging(LOG_LEVEL_NOTICE, "libcam set AeMeteringMode = %d\n", metering_mode);
	ctrl_list.set(controls::AeMeteringMode, metering_mode);
}
void HPVT_start_standard_camera_encoding(HPVT_Config_CAMERA_ID camera_id) {

	while (is_activated == false) {
		usleep(100 * 1000);
		printf("wait\n");
	}

	printf("camera capturing is started\n");
	HPVT_logging(LOG_LEVEL_NOTICE, "CSI2 camera capturing is stopped");

	frame_width = g_context->settings.video.camera.width;
	frame_height = g_context->settings.video.camera.height;
	frame_stride = HpVT_round_value(frame_width, 64);
	frame_height_encoded = HpVT_round_value(frame_height, 16);
	frame_height_encoded = frame_height;

	int ret = 0;

	std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
	ret = cm->start();

	if (ret < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "Could not start CameraManager");
		return;
	}

	if (cm->cameras().empty()) {
		printf("Could not find a CSI2 camera\n");
		HPVT_logging(LOG_LEVEL_ERROR, "Could not find a CSI2 camera");
		cm->stop();
		return;
	}

	std::string cameraId = cm->cameras()[0]->id();
	camera = cm->get(cameraId);

	if (camera == nullptr) {
		HPVT_logging(LOG_LEVEL_ERROR, "Could not get camera (cameraId=%s)", cameraId.c_str());
		cm->stop();
		return;
	}

	ret = camera->acquire();
	if (ret < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "Could not acquire camera");
		camera.reset();
		cm->stop();
		return;
	}

	std::unique_ptr<CameraConfiguration> camera_cfg = camera->generateConfiguration( {
			StreamRole::VideoRecording });

	StreamConfiguration &stream_cfg = camera_cfg->at(0);
	stream_cfg.size.width = frame_width;
	stream_cfg.size.height = frame_height;
	stream_cfg.pixelFormat = formats::YUV420;
	stream_cfg.bufferCount = 6;

	printf("Default viewfinder configuration is: %s\n", stream_cfg.toString().c_str());

	CameraConfiguration::Status camera_cfg_status = camera_cfg->validate();
	printf("CameraConfiguration Status : %d\n", camera_cfg_status);
	printf("Validated viewfinder configuration is: %s\n", stream_cfg.toString().c_str());

	ControlList cam_controls;
	_HPVT_set_camera_parameters(cam_controls, camera_id);

	libcamera::Transform transform = Transform::Identity;
	_HPVT_set_camera_transform(transform, camera_id);
	camera_cfg->transform = transform;

	ret = camera->configure(camera_cfg.get());

	if (ret < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "Could not configure camera");
		camera.reset();
		cm->stop();
		return;
	}

	boolean allocator_failed = false;
	FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);
	Stream *stream = stream_cfg.stream();

	for (StreamConfiguration &stream_cfg : *camera_cfg) {
		if (allocator->allocate(stream) < 0) {
			printf("Can't allocate buffers\n");
			allocator_failed = true;
			break;
		}
		size_t allocated = allocator->buffers(stream).size();
		printf("Allocated %d buffers for stream\n", allocated);
		HPVT_logging(LOG_LEVEL_NOTICE, "Allocated %d buffers for stream", allocated);
	}

	const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
	std::vector<std::unique_ptr<Request>> requests;

	for (int i = 0; i < buffers.size(); ++i) {
		std::unique_ptr<Request> request = camera->createRequest();
		if (!request) {
			printf("Could not create request\n");
			HPVT_logging(LOG_LEVEL_NOTICE, "Could not create request");
			allocator_failed = true;
			break;
		}

		const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
		if (request->addBuffer(stream, buffer.get()) < 0) {
			printf("Could not  set buffer for request\n");
			HPVT_logging(LOG_LEVEL_NOTICE, "Could not set buffer for request");
			allocator_failed = true;
			break;
		}

		requests.push_back(move(request));
	}

	if (allocator_failed) {
		goto label_FUNCTION_END;
	}

	camera->requestCompleted.connect(requestComplete);

	encoder = std::unique_ptr < H264Encoder > (new H264Encoder());
	encoder->SetOutputReadyCallback(&createdFrameBufferCallback);

	if (g_context->settings.video.preview_enabled == true) {
		preview = std::unique_ptr < Preview > (make_preview());
		preview->SetDoneCallback(&previewDoneCallback);
		Size max_size;
		preview->MaxImageSize(max_size.width, max_size.height);
		HPVT_logging(LOG_LEVEL_NOTICE, "Preview MaxImageSize %d x %d", max_size.width, max_size.height);
	}
	else {
		HPVT_logging(LOG_LEVEL_NOTICE, "Preview is disabled");
	}

	_HPVT_init_request_ptr_mutex();

	ret = camera->start(&cam_controls);
	if (ret < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "Cound not start camera");
		goto label_FUNCTION_END;
	}

	for (std::unique_ptr<Request> &request : requests) {
		ret = camera->queueRequest(request.get());
		if (ret < 0) {
			HPVT_logging(LOG_LEVEL_ERROR, "failed to queue request");
			goto label_FUNCTION_END;
		}
	}

	while (is_activated) {
		usleep(100 * 1000);
		if (g_context->termination == true) {
			break;
		}
	}

	label_FUNCTION_END: ;
	camera->stop();
	if (allocator) {
		allocator->free(stream);
		delete allocator;
	}
	camera->release();
	camera.reset();
	cm->stop();
	cm.reset();

	encoder.reset();

	if (preview) {
		preview.reset();
	}

	printf("camera capturing is stopped\n");
	HPVT_logging(LOG_LEVEL_NOTICE, "CSI2 camera capturing is stopped");

	return;
}
boolean HPVT_activate_standard_camera_capture(void) {

	is_activated = true;

	return true;
}
boolean HPVT_inactivate_standard_camera_capture(void) {

	is_activated = false;
	usleep(150000);

	return true;

}
static boolean HpVT_libcam_set_float_parameter(ControlList &ctrl_list, const Control<float>& ctrl_id, float value) {

	ctrl_list.set(ctrl_id, value);

	return true;
}
static boolean HpVT_libcam_set_int32_parameter(ControlList &ctrl_list, const Control<int32_t>& ctrl_id, int32_t value) {

	ctrl_list.set(ctrl_id, value);

	return true;
}
static boolean HpVT_libcam_set_bool_parameter(ControlList &ctrl_list, const Control<bool>& ctrl_id, bool value) {

	ctrl_list.set(ctrl_id, value);

	return true;
}
boolean HpVT_libcam_set_framerate(float framerate) {

	boolean ret;
	int64_t tmp_frame_duration = (1000 * 1000 * 1000) / (int) (framerate * 1000.0);

	HPVT_Lock(request_ptr_locked);
	if (request_ptr == nullptr) {
		HPVT_Unlock(request_ptr_locked);
		return false;
	}

	ControlList &ctrl_list = request_ptr->controls();
	ctrl_list.set(controls::FrameDurationLimits, libcamera::Span<const int64_t, 2>( {
			tmp_frame_duration, tmp_frame_duration }));

	HPVT_Unlock(request_ptr_locked);

	return true;
}
boolean HpVT_libcam_set_brightness(float brightness) {

	boolean ret;

	HPVT_Lock(request_ptr_locked);
	if (request_ptr == nullptr) {
		HPVT_Unlock(request_ptr_locked);
		return false;
	}
	ret = HpVT_libcam_set_float_parameter(request_ptr->controls(), controls::Brightness, brightness);
	HPVT_Unlock(request_ptr_locked);

	return ret;
}
boolean HpVT_libcam_set_saturation(float saturation) {

	boolean ret;

	HPVT_Lock(request_ptr_locked);
	if (request_ptr == nullptr) {
		HPVT_Unlock(request_ptr_locked);
		return false;
	}
	ret = HpVT_libcam_set_float_parameter(request_ptr->controls(), controls::Saturation, saturation);
	HPVT_Unlock(request_ptr_locked);

	return ret;
}
boolean HpVT_libcam_set_sharpness(float sharpness) {

	boolean ret;

	HPVT_Lock(request_ptr_locked);
	if (request_ptr == nullptr) {
		HPVT_Unlock(request_ptr_locked);
		return false;
	}
	ret = HpVT_libcam_set_float_parameter(request_ptr->controls(), controls::Sharpness, sharpness);
	HPVT_Unlock(request_ptr_locked);

	return ret;
}
boolean HpVT_libcam_set_contrast(float contrast) {

	boolean ret;

	HPVT_Lock(request_ptr_locked);
	if (request_ptr == nullptr) {
		HPVT_Unlock(request_ptr_locked);
		return false;
	}
	ret = HpVT_libcam_set_float_parameter(request_ptr->controls(), controls::Contrast, contrast);
	HPVT_Unlock(request_ptr_locked);

	return ret;
}
boolean HpVT_libcam_set_shutter_speed(int shutter_speed) {

	boolean ret;

	HPVT_Lock(request_ptr_locked);
	if (request_ptr == nullptr) {
		HPVT_Unlock(request_ptr_locked);
		return false;
	}
	ret = HpVT_libcam_set_int32_parameter(request_ptr->controls(), controls::ExposureTime, (int32_t) shutter_speed);
	HPVT_Unlock(request_ptr_locked);

	return ret;
}
boolean HpVT_libcam_set_analog_gain(float gain) {

	boolean ret;

	HPVT_Lock(request_ptr_locked);
	if (request_ptr == nullptr) {
		HPVT_Unlock(request_ptr_locked);
		return false;
	}
	ret = HpVT_libcam_set_float_parameter(request_ptr->controls(), controls::AnalogueGain, gain);
	HPVT_Unlock(request_ptr_locked);

	return ret;
}
boolean HpVT_libcam_set_awb_mode(int awb_mode) {

	boolean ret;

	HPVT_Lock(request_ptr_locked);
	if (request_ptr == nullptr) {
		HPVT_Unlock(request_ptr_locked);
		return false;
	}
	if (awb_mode == HPVT_Config_CAMERA_WHITE_BALANCE_MODE_CUSTOM) {
		ret = HpVT_libcam_set_bool_parameter(request_ptr->controls(), controls::AwbEnable, false);
	}
	else {
		ret = HpVT_libcam_set_bool_parameter(request_ptr->controls(), controls::AwbEnable, true);
	}

	if (ret == true) {
		ret = HpVT_libcam_set_int32_parameter(request_ptr->controls(), controls::AwbMode, (int32_t) awb_mode);
	}
	HPVT_Unlock(request_ptr_locked);

	return ret;
}
boolean HpVT_libcam_set_awb_gains(float awb_red_gain, float awb_blue_gain) {

	boolean ret;

	HPVT_Lock(request_ptr_locked);
	if (request_ptr == nullptr) {
		HPVT_Unlock(request_ptr_locked);
		return false;
	}
	ControlList &ctrl_list = request_ptr->controls();
	ctrl_list.set(controls::ColourGains, libcamera::Span<const float, 2>( {
			awb_red_gain, awb_blue_gain }));


	HPVT_Unlock(request_ptr_locked);

	return true;
}
boolean HpVT_libcam_set_exposure_mode(int exposure_mode) {

	boolean ret;

	HPVT_Lock(request_ptr_locked);
	if (request_ptr == nullptr) {
		HPVT_Unlock(request_ptr_locked);
		return false;
	}
	ret = HpVT_libcam_set_int32_parameter(request_ptr->controls(), controls::AeExposureMode, (int32_t) exposure_mode);
	HPVT_Unlock(request_ptr_locked);

	return ret;
}
boolean HpVT_libcam_set_ev_compensation(float ev_compensation) {

	boolean ret;

	HPVT_Lock(request_ptr_locked);
	if (request_ptr == nullptr) {
		HPVT_Unlock(request_ptr_locked);
		return false;
	}
	ret = HpVT_libcam_set_float_parameter(request_ptr->controls(), controls::ExposureValue, ev_compensation);
	HPVT_Unlock(request_ptr_locked);

	return ret;
}
boolean HpVT_libcam_set_metering_mode(int metering_mode) {

	boolean ret;
	HPVT_Lock(request_ptr_locked);
	if (request_ptr == nullptr) {
		HPVT_Unlock(request_ptr_locked);
		return false;
	}
	ret = HpVT_libcam_set_int32_parameter(request_ptr->controls(), controls::AeMeteringMode, (int32_t) metering_mode);

	HPVT_Unlock(request_ptr_locked);

	return ret;
}
boolean HpVT_libcam_set_video_bitrate(int bitrate) {
	return encoder->setBitrate(bitrate);
}
boolean HpVT_libcam_set_video_period(int interval) {
	return encoder->setIPeriod(interval);
}
boolean HpVT_libcam_set_video_profile(HPVT_Config_VIDEO_PROFILE_TYPE profile) {
	return encoder->setProfile(profile);
}
boolean HpVT_libcam_set_video_framerate(float framerate) {
	return encoder->setFramerate(framerate);
}
uint32_t HpVT_round_value(uint32_t value, uint32_t multiplier) {
	return (value + multiplier - 1) & ~(multiplier - 1);
}
