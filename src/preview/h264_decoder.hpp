#pragma once

#include <fcntl.h>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <sys/mman.h>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "preview.hpp"

class H264Decoder {
public:
	H264Decoder();
	~H264Decoder();
	void DecodeBuffer(void *mem, size_t size);
	void SetResolution(unsigned int width, unsigned int height);
	void Start();
	void Stop();
	boolean IsFormatted();

private:
	std::unique_ptr<Preview> preview;
	static const int NUM_OUTPUT_BUFFERS = 6;
	static const int NUM_CAPTURE_BUFFERS = 6;

	void mapOutputBuffers();
	void mapCaptureBuffers();
	void unmapOutputBuffers();
	void unmapCaptureBuffers();
	void startOutputStream();
	void stopOutputStream();
	void startCaptureStream();
	void stopCaptureStream();
	void rotate_input_index();
	void reset_input_index();
	void outputThread();

	boolean is_formatted_;
	unsigned int width_;
	unsigned int height_;
	unsigned int stride_;
	unsigned int height_encoded_;
	unsigned int input_index;

	boolean abortOutput_;
	int fd_;
	struct BufferDescription {
		void *mem;
		size_t size;
		struct v4l2_buffer inner;
		struct v4l2_plane plane;
		int dmafd;
	};
	BufferDescription encoded_buffers_[NUM_OUTPUT_BUFFERS];
	BufferDescription decoded_buffers_[NUM_CAPTURE_BUFFERS];

	struct OutputItem {
		void *mem;
		size_t bytes_used;
		size_t length;
		unsigned int index;
	};
	std::queue<OutputItem> output_queue_;
	std::mutex output_mutex_;
	std::condition_variable output_cond_var_;
	std::thread output_thread_;
};
