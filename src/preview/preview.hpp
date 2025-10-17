#pragma once

#include <functional>
#include <string>

#include "../camera/stream_info.hpp"
#include "../common_s.h"

class Preview
{
public:
	typedef std::function<void(int fd)> DoneCallback;

	Preview(){}
	virtual ~Preview() {}
	void SetDoneCallback(DoneCallback callback) { done_callback_ = callback; }
	virtual void Show(int fd, size_t size, StreamInfo &info) = 0;
	virtual void Reset() = 0;
	virtual bool Quit() { return false; }
	virtual void MaxImageSize(unsigned int &w, unsigned int &h) const = 0;

protected:
	DoneCallback done_callback_;
};

Preview *make_preview();
