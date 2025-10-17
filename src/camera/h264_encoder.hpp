#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

#include "../config/config_s.h"

typedef std::function<void(void *, size_t, int64_t, bool)> OutputReadyCallback;

class H264Encoder
{
public:
	H264Encoder();
	~H264Encoder();
	void EncodeBuffer(int fd, size_t size, void *mem,  int64_t timestamp_us);
	void SetOutputReadyCallback(OutputReadyCallback callback) { output_ready_callback_ = callback; }
	boolean setBitrate(int bitrate);
	boolean setProfile(int profile);
	boolean setIPeriod(int intra);
	boolean setFramerate(int framerate);

protected:
	OutputReadyCallback output_ready_callback_;

private:
	static const int NUM_OUTPUT_BUFFERS = 6;
	static const int NUM_CAPTURE_BUFFERS = 12;

	void pollThread();
	void outputThread();

	bool abortPoll_;
	bool abortOutput_;
	int fd_;
	struct BufferDescription
	{
		void *mem;
		size_t size;
	};
	BufferDescription buffers_[NUM_CAPTURE_BUFFERS];
	int num_capture_buffers_;
	std::thread poll_thread_;
	std::mutex input_buffers_available_mutex_;
	std::queue<int> input_buffers_available_;
	struct OutputItem
	{
		void *mem;
		size_t bytes_used;
		size_t length;
		unsigned int index;
		bool keyframe;
		int64_t timestamp_us;
	};
	std::queue<OutputItem> output_queue_;
	std::mutex output_mutex_;
	std::condition_variable output_cond_var_;
	std::thread output_thread_;
};
