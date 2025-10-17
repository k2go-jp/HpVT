#define FILE_NUMBER "C202"

#include <fcntl.h>
#include <poll.h>
#include <libv4l2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#include "../network/context_s.h"
#include "../camera/camera.h"
#include "h264_encoder.hpp"

extern HPVT_Context *g_context;

static int xioctl(int fd, unsigned long ctl, void *arg) {
	int ret, num_tries = 10;
	do {
		ret = v4l2_ioctl(fd, ctl, arg);
	} while (ret == -1 && errno == EINTR && num_tries-- > 0);
	return ret;
}
static int _HPVT_convert_libcam_profile(int value) {

	switch (value) {
		case HPVT_Config_VIDEO_PROFILE_BASELINE:
			return V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE;
		case HPVT_Config_VIDEO_PROFILE_MAIN:
			return V4L2_MPEG_VIDEO_H264_PROFILE_MAIN;
		case HPVT_Config_VIDEO_PROFILE_HIGH:
			return V4L2_MPEG_VIDEO_H264_PROFILE_HIGH;
		default:
			return V4L2_MPEG_VIDEO_H264_PROFILE_HIGH;
	}

}
H264Encoder::H264Encoder() :
		abortPoll_(false), abortOutput_(false) {

	const char device_name[] = "/dev/video11";
	fd_ = v4l2_open(device_name, O_RDWR, 0);
	if (fd_ < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "failed to open V4L2 H264 encoder");
		throw std::runtime_error("failed to open V4L2 H264 encoder");
	}

	HPVT_logging(LOG_LEVEL_NOTICE, "H264Encoder fd = %d", fd_);

	v4l2_control ctrl;
	ctrl.id = V4L2_CID_MPEG_VIDEO_REPEAT_SEQ_HEADER;
	ctrl.value = 1;
	if (xioctl(fd_, VIDIOC_S_CTRL, &ctrl) < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "H264Encoder:failed to set inline headers");
		throw std::runtime_error("failed to set inline headers");
	}

	ctrl.id = V4L2_CID_MPEG_VIDEO_H264_LEVEL;
	ctrl.value = V4L2_MPEG_VIDEO_H264_LEVEL_4_2;
	if (xioctl(fd_, VIDIOC_S_CTRL, &ctrl) < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "H264Encoder:failed to set level");
		throw std::runtime_error("failed to set level");
	}

	this->setProfile(g_context->settings.video.profile);
	this->setBitrate(g_context->settings.video.bitrate);
	this->setIPeriod(g_context->settings.video.i_frame_interval);
	this->setFramerate(g_context->settings.video.encode_framerate);

	v4l2_format fmt;
	fmt = {};
	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	fmt.fmt.pix_mp.width = g_context->settings.video.camera.width;
	fmt.fmt.pix_mp.height = g_context->settings.video.camera.height;
	fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_YUV420;
	fmt.fmt.pix_mp.plane_fmt[0].bytesperline = HpVT_round_value(fmt.fmt.pix_mp.width, 64);
	fmt.fmt.pix_mp.field = V4L2_FIELD_ANY;
	fmt.fmt.pix_mp.colorspace = V4L2_COLORSPACE_REC709;
	fmt.fmt.pix_mp.num_planes = 1;
	if (xioctl(fd_, VIDIOC_S_FMT, &fmt) < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "failed to set output format");
		throw std::runtime_error("failed to set output format");
	}

	fmt = {};
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	fmt.fmt.pix_mp.width = g_context->settings.video.camera.width;
	fmt.fmt.pix_mp.height = g_context->settings.video.camera.height;
	fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
	fmt.fmt.pix_mp.field = V4L2_FIELD_ANY;
	fmt.fmt.pix_mp.colorspace = V4L2_COLORSPACE_DEFAULT;
	fmt.fmt.pix_mp.num_planes = 1;
	fmt.fmt.pix_mp.plane_fmt[0].bytesperline = 0;
	fmt.fmt.pix_mp.plane_fmt[0].sizeimage = 512 << 10;
	if (xioctl(fd_, VIDIOC_S_FMT, &fmt) < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "failed to set capture format");
		throw std::runtime_error("failed to set capture format");
	}

	v4l2_requestbuffers reqbufs;
	reqbufs = {};
	reqbufs.count = NUM_OUTPUT_BUFFERS;
	reqbufs.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	reqbufs.memory = V4L2_MEMORY_DMABUF;
	if (xioctl(fd_, VIDIOC_REQBUFS, &reqbufs) < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "request for output buffers failed");
		throw std::runtime_error("request for output buffers failed");
	}

	for (unsigned int i = 0; i < reqbufs.count; i++) {
		input_buffers_available_.push(i);
	}

	reqbufs = {};
	reqbufs.count = NUM_CAPTURE_BUFFERS;
	reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	reqbufs.memory = V4L2_MEMORY_MMAP;
	if (xioctl(fd_, VIDIOC_REQBUFS, &reqbufs) < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "request for capture buffers failed");
		throw std::runtime_error("request for capture buffers failed");
	}
	num_capture_buffers_ = reqbufs.count;

	for (unsigned int i = 0; i < reqbufs.count; i++) {
		v4l2_plane planes;
		v4l2_buffer buffer = { };
		buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = i;
		buffer.length = 1;
		buffer.m.planes = &planes;
		if (xioctl(fd_, VIDIOC_QUERYBUF, &buffer) < 0) {
			HPVT_logging(LOG_LEVEL_ERROR, "failed to capture query buffer %d", i);
			throw std::runtime_error("failed to capture query buffer " + std::to_string(i));
		}
		buffers_[i].mem = mmap(0, buffer.m.planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, buffer.m.planes[0].m.mem_offset);
		if (buffers_[i].mem == MAP_FAILED) {
			HPVT_logging(LOG_LEVEL_ERROR, "failed to mmap capture buffer %d", i);
			throw std::runtime_error("failed to mmap capture buffer " + std::to_string(i));
		}
		buffers_[i].size = buffer.m.planes[0].length;
		if (xioctl(fd_, VIDIOC_QBUF, &buffer) < 0) {
			HPVT_logging(LOG_LEVEL_ERROR, "failed to queue capture buffer %d", i);
			throw std::runtime_error("failed to queue capture buffer " + std::to_string(i));
		}
	}

	v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	if (xioctl(fd_, VIDIOC_STREAMON, &type) < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "failed to start output streaming");
		throw std::runtime_error("failed to start output streaming");
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	if (xioctl(fd_, VIDIOC_STREAMON, &type) < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "failed to start capture streaming");
		throw std::runtime_error("failed to start capture streaming");
	}
	output_thread_ = std::thread(&H264Encoder::outputThread, this);
	poll_thread_ = std::thread(&H264Encoder::pollThread, this);

	printf("H264Encoder started\n");
	HPVT_logging(LOG_LEVEL_NOTICE, "H264Encoder started");
}
H264Encoder::~H264Encoder() {

	abortPoll_ = true;
	poll_thread_.join();
	abortOutput_ = true;
	output_thread_.join();

	v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	if (xioctl(fd_, VIDIOC_STREAMOFF, &type) < 0) {
		printf("failed to stop output streaming\n");
		HPVT_logging(LOG_LEVEL_ERROR, "failed to stop output streaming");
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	if (xioctl(fd_, VIDIOC_STREAMOFF, &type) < 0) {
		printf("failed to stop capture streaming\n");
		HPVT_logging(LOG_LEVEL_ERROR, "failed to stop capture streaming");
	}

	v4l2_requestbuffers reqbufs;
	reqbufs = {};
	reqbufs.count = 0;
	reqbufs.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	reqbufs.memory = V4L2_MEMORY_DMABUF;
	if (xioctl(fd_, VIDIOC_REQBUFS, &reqbufs) < 0) {
		printf("failed to request to free output buffers\n");
		HPVT_logging(LOG_LEVEL_ERROR, "failed to request to free output buffers");
	}

	for (int i = 0; i < num_capture_buffers_; i++) {
		if (munmap(buffers_[i].mem, buffers_[i].size) < 0) {
			printf("failed to unmap capture buffer\n");
			HPVT_logging(LOG_LEVEL_ERROR, "failed to unmap capture buffer");
		}
	}
	reqbufs = {};
	reqbufs.count = 0;
	reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	reqbufs.memory = V4L2_MEMORY_MMAP;
	if (xioctl(fd_, VIDIOC_REQBUFS, &reqbufs) < 0) {
		printf("failed to request to free capture buffers\n");
		HPVT_logging(LOG_LEVEL_ERROR, "failed to request to free capture buffers");
	}

	close(fd_);
	printf("H264Encoder stopped\n");
	HPVT_logging(LOG_LEVEL_NOTICE, "H264Encoder stopped");
}
void H264Encoder::EncodeBuffer(int fd, size_t size, void *mem, int64_t timestamp_us) {

	int index;
	std::lock_guard < std::mutex > lock(input_buffers_available_mutex_);
	if (input_buffers_available_.empty()) {
		HPVT_logging(LOG_LEVEL_ERROR, "no buffers available to queue codec input");
		throw std::runtime_error("no buffers available to queue codec input");
	}
	index = input_buffers_available_.front();
	input_buffers_available_.pop();

	v4l2_buffer buf = { };
	v4l2_plane planes = { };
	buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	buf.index = index;
	buf.field = V4L2_FIELD_NONE;
	buf.memory = V4L2_MEMORY_DMABUF;
	buf.length = 1;
	buf.timestamp.tv_sec = timestamp_us / 1000000;
	buf.timestamp.tv_usec = timestamp_us % 1000000;
	buf.m.planes = &planes;
	buf.m.planes[0].m.fd = fd;
	buf.m.planes[0].bytesused = size;
	buf.m.planes[0].length = size;
	if (xioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "failed to queue input to codec");
		throw std::runtime_error("failed to queue input to codec");
	}

}
void H264Encoder::pollThread() {

	while (true) {
		pollfd p = {
				fd_, POLLIN, 0 };
		int ret = poll(&p, 1, 200);
		{
			std::lock_guard < std::mutex > lock(input_buffers_available_mutex_);
			if (abortPoll_ && input_buffers_available_.size() == NUM_OUTPUT_BUFFERS) {
				break;
			}
		}
		if (ret == -1) {
			if (errno == EINTR) {
				continue;
			}
			throw std::runtime_error("unexpected errno " + std::to_string(errno) + " from poll");
		}
		if (p.revents & POLLIN) {
			v4l2_buffer buf = { };
			v4l2_plane planes = { };
			buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
			buf.memory = V4L2_MEMORY_DMABUF;
			buf.length = 1;
			buf.m.planes = &planes;
			int ret = xioctl(fd_, VIDIOC_DQBUF, &buf);
			if (ret == 0) {
				std::lock_guard < std::mutex > lock(input_buffers_available_mutex_);
				input_buffers_available_.push(buf.index);
			}

			buf = {};
			planes= {};
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.length = 1;
			buf.m.planes = &planes;
			ret = xioctl(fd_, VIDIOC_DQBUF, &buf);
			if (ret == 0) {
				int64_t timestamp_us = (buf.timestamp.tv_sec * (int64_t) 1000000) + buf.timestamp.tv_usec;
				OutputItem item = {
						buffers_[buf.index].mem, //
						buf.m.planes[0].bytesused, //
						buf.m.planes[0].length, //
						buf.index, //
						!!(buf.flags & V4L2_BUF_FLAG_KEYFRAME), //
						timestamp_us };
				std::lock_guard < std::mutex > lock(output_mutex_);
				output_queue_.push(item);
				output_cond_var_.notify_one();
			}
		}
	}
}
void H264Encoder::outputThread() {

	OutputItem item;
	while (true) {
		{
			std::unique_lock < std::mutex > lock(output_mutex_);
			while (true) {
				if (abortOutput_ && output_queue_.empty())
					return;

				if (!output_queue_.empty()) {
					item = output_queue_.front();
					output_queue_.pop();
					break;
				}
				else {
					output_cond_var_.wait_for(lock, std::chrono::milliseconds(100));
				}
			}
		}

		output_ready_callback_(item.mem, item.bytes_used, item.timestamp_us, item.keyframe);
		v4l2_buffer buf = { };
		v4l2_plane planes = { };
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = item.index;
		buf.length = 1;
		buf.m.planes = &planes;
		buf.m.planes[0].bytesused = 0;
		buf.m.planes[0].length = item.length;
		if (xioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
			HPVT_logging(LOG_LEVEL_ERROR, "failed to re-queue encoded buffer");
			throw std::runtime_error("failed to re-queue encoded buffer");
		}
	}
}
boolean H264Encoder::setBitrate(int bitrate) {

	v4l2_control ctrl = { };
	ctrl.id = V4L2_CID_MPEG_VIDEO_BITRATE;
	ctrl.value = bitrate;

	HPVT_logging(LOG_LEVEL_NOTICE, "H264Encoder:set bitrate %d bps", bitrate);

	if (xioctl(fd_, VIDIOC_S_CTRL, &ctrl) < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "H264Encoder:failed to set bitrate");
		return false;
	}

	HPVT_Lock(g_context->settings.settings_locked);
	g_context->settings.video.bitrate = bitrate;
	HPVT_Unlock(g_context->settings.settings_locked);
	HPVT_logging(LOG_LEVEL_NOTICE, "H264Encoder:succeeded to set bitrate");

	return true;
}
boolean H264Encoder::setProfile(int profile) {

	int tmp_profile = _HPVT_convert_libcam_profile(profile);

	v4l2_control ctrl = { };
	ctrl.id = V4L2_CID_MPEG_VIDEO_H264_PROFILE;
	ctrl.value = _HPVT_convert_libcam_profile(tmp_profile);

	HPVT_logging(LOG_LEVEL_NOTICE, "H264Encoder:set profile %d (V4L2 %d)", profile, tmp_profile);

	if (xioctl(fd_, VIDIOC_S_CTRL, &ctrl) < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "H264Encoder:failed to set profile");
		return false;
	}

	HPVT_Lock(g_context->settings.settings_locked);
	g_context->settings.video.profile = profile;
	HPVT_Unlock(g_context->settings.settings_locked);
	HPVT_logging(LOG_LEVEL_NOTICE, "H264Encoder:succeeded to set profile");

	return true;
}
boolean H264Encoder::setIPeriod(int intra) {

	v4l2_control ctrl = { };
	ctrl.id = V4L2_CID_MPEG_VIDEO_H264_I_PERIOD;
	ctrl.value = intra;

	HPVT_logging(LOG_LEVEL_NOTICE, "H264Encoder:set I Period %d", intra);

	if (xioctl(fd_, VIDIOC_S_CTRL, &ctrl) < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "H264Encoder:failed to set I period");
		return false;
	}

	HPVT_Lock(g_context->settings.settings_locked);
	g_context->settings.video.i_frame_interval = intra;
	HPVT_Unlock(g_context->settings.settings_locked);
	HPVT_logging(LOG_LEVEL_NOTICE, "H264Encoder:succeeded to set I period");

	return true;
}
boolean H264Encoder::setFramerate(int framerate) {

	struct v4l2_streamparm streamparm;
	streamparm = {};
	streamparm.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	streamparm.parm.output.capability = V4L2_CAP_TIMEPERFRAME;
	streamparm.parm.output.timeperframe.denominator = framerate;
	streamparm.parm.output.timeperframe.numerator = 1;
	HPVT_logging(LOG_LEVEL_NOTICE, "H264Encoder:set framerate %d fps", framerate);

	if (xioctl(fd_, VIDIOC_S_PARM, &streamparm) < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "failed to set output streamparm timeperframe");
		throw std::runtime_error("failed to set output streamparm timeperframe");
	}

	HPVT_Lock(g_context->settings.settings_locked);
	g_context->settings.video.encode_framerate = framerate;
	HPVT_Unlock(g_context->settings.settings_locked);

	HPVT_logging(LOG_LEVEL_NOTICE, "H264Encoder:succeeded to set framerate %d/%d", //
			streamparm.parm.output.timeperframe.numerator, streamparm.parm.output.timeperframe.denominator);

	return true;
}
