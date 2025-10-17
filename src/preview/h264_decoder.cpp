#define FILE_NUMBER "P102"

#include <libcamera/libcamera.h>

#include "../camera/camera.h"
#include "h264_decoder.hpp"

using namespace libcamera;

static void previewDoneCallback(int fd) {
}

void H264Decoder::SetResolution(unsigned int width, unsigned int height) {

	width_ = width;
	height_ = height;
	stride_ = HpVT_round_value(width, 64);
	height_encoded_ = HpVT_round_value(height, 16);

	HPVT_logging(LOG_LEVEL_NOTICE, "H.264 Decoder Set width=%d height=%d stride=%d, height_encoded=%d", //
			width_, height_, stride_, height_encoded_);
}
void H264Decoder::Start() {

	mapOutputBuffers();
	mapCaptureBuffers();
	startOutputStream();
	startCaptureStream();

	preview = std::unique_ptr < Preview > (make_preview());
	preview->SetDoneCallback((&previewDoneCallback));

	is_formatted_ = true;
	output_thread_ = std::thread(&H264Decoder::outputThread, this);
}
void H264Decoder::Stop() {

	if (is_formatted_) {
		abortOutput_ = true;
		output_thread_.join();
		abortOutput_ = false;

		stopOutputStream();
		stopCaptureStream();
		unmapOutputBuffers();
		unmapCaptureBuffers();

		preview->Quit();
		preview->Reset();
		is_formatted_ = false;
	}
}
boolean H264Decoder::IsFormatted() {

	return is_formatted_;
}
void H264Decoder::rotate_input_index(void) {
	input_index += 1;
	if (input_index == NUM_OUTPUT_BUFFERS) {
		input_index = 0;
	}
}
void H264Decoder::reset_input_index(void) {
	input_index = 0;
}
void H264Decoder::mapOutputBuffers() {

	v4l2_format fmt = { };
	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	fmt.fmt.pix_mp.width = width_;
	fmt.fmt.pix_mp.height = height_encoded_;
	fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
	fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
	fmt.fmt.pix_mp.num_planes = 1;
	if (v4l2_ioctl(fd_, VIDIOC_S_FMT, &fmt) < 0) {
		throw std::runtime_error("failed to set output format");
	}

	if (v4l2_ioctl(fd_, VIDIOC_G_FMT, &fmt) < 0) {
		throw std::runtime_error("failed to get output format");
	}

	v4l2_requestbuffers reqbufs = { };
	reqbufs.count = NUM_OUTPUT_BUFFERS;
	reqbufs.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	reqbufs.memory = V4L2_MEMORY_MMAP;
	if (v4l2_ioctl(fd_, VIDIOC_REQBUFS, &reqbufs) < 0) {
		throw std::runtime_error("request for output buffers failed");
	}

	for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++) {

		struct v4l2_buffer *buffer = &encoded_buffers_[i].inner;
		memset(buffer, 0, sizeof(struct v4l2_buffer));
		buffer->type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
		buffer->memory = V4L2_MEMORY_MMAP;
		buffer->index = i;
		buffer->length = 1;
		buffer->m.planes = &(encoded_buffers_[i].plane);

		if (v4l2_ioctl(fd_, VIDIOC_QUERYBUF, buffer) < 0) {
			throw std::runtime_error("failed to output query buffer " + std::to_string(i));
		}
		encoded_buffers_[i].mem = v4l2_mmap(0, buffer->m.planes[0].length, //
				PROT_READ | PROT_WRITE, MAP_SHARED, fd_, buffer->m.planes[0].m.mem_offset);
		if (encoded_buffers_[i].mem == MAP_FAILED) {
			throw std::runtime_error("failed to mmap output buffer " + std::to_string(i));
		}
		encoded_buffers_[i].size = buffer->m.planes[0].length;
		if (v4l2_ioctl(fd_, VIDIOC_QBUF, buffer) < 0) {
			throw std::runtime_error("failed to queue output buffer " + std::to_string(i));
		}
	}

}
void H264Decoder::mapCaptureBuffers() {

	v4l2_format fmt = { };
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	fmt.fmt.pix_mp.width = width_;
	fmt.fmt.pix_mp.height = height_encoded_;
	fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_YUV420;
	fmt.fmt.pix_mp.plane_fmt[0].bytesperline = stride_;
	fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
	fmt.fmt.pix_mp.num_planes = 1;
	if (v4l2_ioctl(fd_, VIDIOC_S_FMT, &fmt) < 0) {
		throw std::runtime_error("failed to set capture format");
	}

	if (v4l2_ioctl(fd_, VIDIOC_G_FMT, &fmt) < 0) {
		throw std::runtime_error("failed to get capture format");
	}

	v4l2_requestbuffers reqbufs = { };
	reqbufs.count = NUM_CAPTURE_BUFFERS;
	reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	reqbufs.memory = V4L2_MEMORY_MMAP;
	if (v4l2_ioctl(fd_, VIDIOC_REQBUFS, &reqbufs) < 0) {
		throw std::runtime_error("request for capture buffers failed");
	}

	for (int i = 0; i < NUM_CAPTURE_BUFFERS; i++) {
		struct v4l2_buffer *buffer = &decoded_buffers_[i].inner;
		memset(buffer, 0, sizeof(struct v4l2_buffer));
		buffer->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buffer->memory = V4L2_MEMORY_MMAP;
		buffer->index = i;
		buffer->length = 1;
		buffer->m.planes = &(decoded_buffers_[i].plane);

		if (v4l2_ioctl(fd_, VIDIOC_QUERYBUF, buffer) < 0) {
			throw std::runtime_error("failed to capture query buffer " + std::to_string(i));
		}

		decoded_buffers_[i].mem = v4l2_mmap(0, buffer->m.planes[0].length,
		PROT_READ | PROT_WRITE, MAP_SHARED, fd_, buffer->m.planes[0].m.mem_offset);
		if (decoded_buffers_[i].mem == MAP_FAILED) {
			throw std::runtime_error("failed to mmap capture buffer " + std::to_string(i));
		}
		decoded_buffers_[i].size = buffer->m.planes[0].length;

		if (v4l2_ioctl(fd_, VIDIOC_QBUF, buffer) < 0) {
			throw std::runtime_error("failed to queue capture buffer " + std::to_string(i));
		}
	}

	for (int i = 0; i < NUM_CAPTURE_BUFFERS; i++) {
		struct v4l2_exportbuffer exbuf = { };
		exbuf.index = i;
		exbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		exbuf.plane = 0;

		if (v4l2_ioctl(fd_, VIDIOC_EXPBUF, &exbuf) < 0) {
			throw std::runtime_error("failed to queue export buffer " + std::to_string(i));
		}
		decoded_buffers_[i].dmafd = exbuf.fd;
	}
}
void H264Decoder::unmapOutputBuffers() {

	v4l2_requestbuffers reqbufs = { };
	reqbufs.count = 0;
	reqbufs.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	reqbufs.memory = V4L2_MEMORY_MMAP;
	if (v4l2_ioctl(fd_, VIDIOC_REQBUFS, &reqbufs) < 0) {
		printf("Request to free output buffers failed\n");
	}

	for (int i = 0; i < NUM_OUTPUT_BUFFERS; i++) {
		if (munmap(encoded_buffers_[i].mem, encoded_buffers_[i].size) < 0) {
			printf("Failed to unmap buffer\n");
		}
	}

}
void H264Decoder::unmapCaptureBuffers() {

	v4l2_requestbuffers reqbufs = { };
	reqbufs.count = 0;
	reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	reqbufs.memory = V4L2_MEMORY_MMAP;
	if (v4l2_ioctl(fd_, VIDIOC_REQBUFS, &reqbufs) < 0) {
		printf("Request to free capture buffers failed\n");
	}

	for (int i = 0; i < NUM_CAPTURE_BUFFERS; i++) {
		if (munmap(decoded_buffers_[i].mem, decoded_buffers_[i].size) < 0) {
			printf("Failed to unmap buffer\n");
		}
	}

	for (int i = 0; i < NUM_CAPTURE_BUFFERS; i++) {
		close(decoded_buffers_[i].dmafd);
	}

}
void H264Decoder::startOutputStream() {

	v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	if (v4l2_ioctl(fd_, VIDIOC_STREAMON, &type) < 0) {
		throw std::runtime_error("failed to start output streaming");
	}
}
void H264Decoder::stopOutputStream() {

	v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	if (v4l2_ioctl(fd_, VIDIOC_STREAMOFF, &type) < 0) {
		throw std::runtime_error("failed to stop output streaming");
	}
}
void H264Decoder::startCaptureStream() {

	v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	if (v4l2_ioctl(fd_, VIDIOC_STREAMON, &type) < 0) {
		throw std::runtime_error("failed to start capture streaming");
	}
}
void H264Decoder::stopCaptureStream() {

	v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	if (v4l2_ioctl(fd_, VIDIOC_STREAMOFF, &type) < 0) {
		throw std::runtime_error("failed to stop capture streaming");
	}
}
H264Decoder::H264Decoder() :
		abortOutput_(false) {

	is_formatted_ = false;
	width_ = 0;
	height_ = 0;
	stride_ = 0;
	height_encoded_ = 0;
	input_index = 0;

	const char device_name[] = "/dev/video10";
	fd_ = v4l2_open(device_name, O_RDWR | O_NONBLOCK);
	if (fd_ < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "H264Decoder failed to open");
		throw std::runtime_error("failed to open V4L2 H264 decoder");
	}
	HPVT_logging(LOG_LEVEL_NOTICE, "H264Decoder fd = %d", fd_);

	if (v4l2_fd_open(fd_, V4L2_DISABLE_CONVERSION) != fd_) {
		close(fd_);
		HPVT_logging(LOG_LEVEL_ERROR, "H264Decoder failed to fd_open");
		throw std::runtime_error("failed to fd_open V4L2 H264 decoder");
	}
}
H264Decoder::~H264Decoder() {

	abortOutput_ = true;
	output_thread_.join();

	stopOutputStream();
	stopCaptureStream();

	unmapOutputBuffers();
	unmapCaptureBuffers();

	close(fd_);

	preview.reset();

	HPVT_logging(LOG_LEVEL_NOTICE, "H264Decoder closed");
}
void H264Decoder::DecodeBuffer(void *mem, size_t size) {

	if (abortOutput_) {
		return;
	}

	if (is_formatted_ == false) {
		return;
	}

	if (input_index < 0 || input_index >= NUM_OUTPUT_BUFFERS) {
		HPVT_logging(LOG_LEVEL_ERROR, "Invalid index output buffers (%d)", input_index);
		return;
	}

	v4l2_buffer buf = { };
	v4l2_plane planes = { };

	buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	buf.index = input_index;
	buf.field = V4L2_FIELD_NONE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.length = 1;
	buf.m.planes = &planes;

	if (v4l2_ioctl(fd_, VIDIOC_DQBUF, &buf) < 0) {
		rotate_input_index();
		return;
	}

	memcpy(encoded_buffers_[buf.index].mem, mem, size);
	encoded_buffers_[buf.index].inner.m.planes[0].bytesused = size;

	if (v4l2_ioctl(fd_, VIDIOC_QBUF, &encoded_buffers_[buf.index].inner) < 0) {
		throw std::runtime_error("failed to queue input to decoder");
	}

	buf = {};
	planes = {};

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.length = 1;
	buf.m.planes = &planes;

	if (v4l2_ioctl(fd_, VIDIOC_DQBUF, &buf) < 0) {
		if (errno == EPIPE) {
			is_formatted_ = false;
			stopCaptureStream();
			unmapCaptureBuffers();
			mapCaptureBuffers();
			startCaptureStream();

			reset_input_index();
			is_formatted_ = true;
		}
		else {
			rotate_input_index();
		}
		return;
	}

	OutputItem item = {
			decoded_buffers_[buf.index].mem, buf.m.planes[0].bytesused, buf.m.planes[0].length, buf.index };
	std::lock_guard < std::mutex > lock(output_mutex_);
	output_queue_.push(item);
	output_cond_var_.notify_one();

	rotate_input_index();

}
void H264Decoder::outputThread() {

	OutputItem item;
	while (true) {
		std::unique_lock < std::mutex > lock(output_mutex_);

		while (true) {
			if (abortOutput_ && output_queue_.empty()) {
				return;
			}

			if (!output_queue_.empty()) {
				item = output_queue_.front();
				output_queue_.pop();
				break;
			}
			else {
				output_cond_var_.wait_for(lock, std::chrono::milliseconds(100));
			}
		}

		if (is_formatted_) {
			StreamInfo info = { };
			info.width = width_;
			info.height = height_;
			info.stride = stride_;
			info.height_encoded = height_encoded_;
			info.pixel_format = libcamera::formats::YUV420;
			info.colour_space = libcamera::ColorSpace::Rec709;
			preview->Show(decoded_buffers_[item.index].dmafd, decoded_buffers_[item.index].plane.bytesused, info);
		}

		decoded_buffers_[item.index].plane.bytesused = 0;
		if (v4l2_ioctl(fd_, VIDIOC_QBUF, &decoded_buffers_[item.index].inner) < 0) {
			HPVT_logging(LOG_LEVEL_ERROR, "failed to queue buffer");
		}
	}
}
