#pragma once

#include <optional>

#include <libcamera/color_space.h>
#include <libcamera/pixel_format.h>

struct StreamInfo
{
	StreamInfo() : width(0), height(0), stride(0), height_encoded(0) {}
	unsigned int width;
	unsigned int height;
	unsigned int stride;
	unsigned int height_encoded;
	libcamera::PixelFormat pixel_format;
	std::optional<libcamera::ColorSpace> colour_space;
};
