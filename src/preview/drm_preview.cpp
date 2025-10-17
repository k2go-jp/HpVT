#define FILE_NUMBER "P104"

#include "preview.hpp"

#include <drm.h>
#include <drm_fourcc.h>
#include <drm_mode.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <map>
#include <string>

#include "../network/context.h"

extern HPVT_Context *g_context;

class DrmPreview: public Preview {
public:
	DrmPreview();
	~DrmPreview();
	virtual void Show(int fd, size_t size, StreamInfo &info) override;
	virtual void Reset() override;
	virtual void MaxImageSize(unsigned int &w, unsigned int &h) const override
	{
		w = max_image_width_;
		h = max_image_height_;
	}

private:
	struct Buffer {
		Buffer() :
				fd(-1), size(0), bo_handle(0), fb_handle(0) {
		}
		int fd;
		size_t size;
		StreamInfo info;
		uint32_t bo_handle;
		unsigned int fb_handle;
	};
	void makeBuffer(int fd, size_t size, StreamInfo const &info, Buffer &buffer);
	void findCrtc();
	void findPlane();
	int drmfd_;
	int conId_;
	uint32_t crtcId_;
	int crtcIdx_;
	uint32_t planeId_;
	unsigned int out_fourcc_;
	unsigned int x_;
	unsigned int y_;
	unsigned int width_;
	unsigned int height_;
	unsigned int screen_width_;
	unsigned int screen_height_;
	std::map<int, Buffer> buffers_;
	int last_fd_;
	unsigned int max_image_width_;
	unsigned int max_image_height_;
	bool first_time_;
};

#define ERRSTR strerror(errno)

void DrmPreview::findCrtc() {
	int i;
	drmModeRes *res = drmModeGetResources(drmfd_);
	if (!res) {
		HPVT_logging(LOG_LEVEL_ERROR, "drmModeGetResources failed");
		throw std::runtime_error("drmModeGetResources failed: " + std::string(ERRSTR));
	}

	if (res->count_crtcs <= 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "drm: no crts");
		throw std::runtime_error("drm: no crts");
	}

	max_image_width_ = res->max_width;
	max_image_height_ = res->max_height;

	if (!conId_) {
		for (i = 0; i < res->count_connectors; i++) {
			drmModeConnector *con = drmModeGetConnector(drmfd_, res->connectors[i]);
			drmModeEncoder *enc = NULL;
			drmModeCrtc *crtc = NULL;

			if (con->encoder_id) {
				enc = drmModeGetEncoder(drmfd_, con->encoder_id);
				if (enc->crtc_id) {
					crtc = drmModeGetCrtc(drmfd_, enc->crtc_id);
				}
			}

			if (!conId_ && crtc) {
				conId_ = con->connector_id;
				crtcId_ = crtc->crtc_id;
			}

			if (crtc) {
				screen_width_ = crtc->width;
				screen_height_ = crtc->height;
			}
		}

		if (!conId_) {
			HPVT_logging(LOG_LEVEL_ERROR, "No suitable enabled connector found");
			throw std::runtime_error("No suitable enabled connector found");
		}
	}

	crtcIdx_ = -1;

	for (i = 0; i < res->count_crtcs; ++i) {
		if (crtcId_ == res->crtcs[i]) {
			crtcIdx_ = i;
			break;
		}
	}

	if (crtcIdx_ == -1) {
		drmModeFreeResources(res);
		HPVT_logging(LOG_LEVEL_ERROR, "drm: CRTC %d not found", crtcId_);
		throw std::runtime_error("drm: CRTC " + std::to_string(crtcId_) + " not found");
	}

	if (res->count_connectors <= 0) {
		drmModeFreeResources(res);
		HPVT_logging(LOG_LEVEL_ERROR, "drm: no connectors");
		throw std::runtime_error("drm: no connectors");
	}

	drmModeConnector *c;
	c = drmModeGetConnector(drmfd_, conId_);
	if (!c) {
		drmModeFreeResources(res);
		HPVT_logging(LOG_LEVEL_ERROR, "drmModeGetConnector failed");
		throw std::runtime_error("drmModeGetConnector failed: " + std::string(ERRSTR));
	}

	if (!c->count_modes) {
		drmModeFreeConnector(c);
		drmModeFreeResources(res);
		HPVT_logging(LOG_LEVEL_ERROR, "connector supports no mode");
		throw std::runtime_error("connector supports no mode");
	}

	if (width_ == 0 || height_ == 0) {
		drmModeCrtc *crtc = drmModeGetCrtc(drmfd_, crtcId_);
		x_ = crtc->x;
		y_ = crtc->y;
		width_ = crtc->width;
		height_ = crtc->height;
		drmModeFreeCrtc(crtc);
	}
}
void DrmPreview::findPlane() {
	drmModePlaneResPtr planes;
	drmModePlanePtr plane;
	unsigned int i;
	unsigned int j;

	planes = drmModeGetPlaneResources(drmfd_);
	if (!planes) {
		HPVT_logging(LOG_LEVEL_ERROR, "drmModeGetPlaneResources failed");
		throw std::runtime_error("drmModeGetPlaneResources failed: " + std::string(ERRSTR));
	}

	try {
		for (i = 0; i < planes->count_planes; ++i) {
			plane = drmModeGetPlane(drmfd_, planes->planes[i]);
			if (!planes) {
				HPVT_logging(LOG_LEVEL_ERROR, "drmModeGetPlane failed");
				throw std::runtime_error("drmModeGetPlane failed: " + std::string(ERRSTR));
			}

			if (!(plane->possible_crtcs & (1 << crtcIdx_))) {
				drmModeFreePlane(plane);
				continue;
			}

			for (j = 0; j < plane->count_formats; ++j) {
				if (plane->formats[j] == out_fourcc_) {
					break;
				}
			}

			if (j == plane->count_formats) {
				drmModeFreePlane(plane);
				continue;
			}

			planeId_ = plane->plane_id;

			drmModeFreePlane(plane);
			break;
		}
	}
	catch (std::exception const &e) {
		drmModeFreePlaneResources(planes);
		throw;
	}

	drmModeFreePlaneResources(planes);
}
DrmPreview::DrmPreview() :
		Preview(), last_fd_(-1), first_time_(true) {
	drmfd_ = drmOpen("vc4", NULL);
	if (drmfd_ < 0) {
		HPVT_logging(LOG_LEVEL_ERROR, "drmOpen failed");
		throw std::runtime_error("drmOpen failed: " + std::string(ERRSTR));
	}

	x_ = 0;
	y_ = 0;
	if (HPVT_Context_is_transmitter(g_context) == true) {
		width_ = g_context->settings.video.camera.width;
		height_ = g_context->settings.video.camera.height;
	}
	else {
		width_ = g_context->connection.current_resolution_width;
		height_ = g_context->connection.current_resolution_height;
	}
	screen_width_ = 0;
	screen_height_ = 0;

	try {
		if (!drmIsMaster(drmfd_)) {
			HPVT_logging(LOG_LEVEL_ERROR, "DRM preview unavailable - not master");
			throw std::runtime_error("DRM preview unavailable - not master");
		}

		conId_ = 0;
		findCrtc();
		out_fourcc_ = DRM_FORMAT_YUV420;
		findPlane();
	}
	catch (std::exception const &e) {
		close(drmfd_);
		throw;
	}

	if (g_context->settings.video.preview.full_screen == true || //
			width_ == 0 || height_ == 0 || x_ + width_ > screen_width_ || y_ + height_ > screen_height_) {
		x_ = y_ = 0;
		width_ = screen_width_;
		height_ = screen_height_;
	}

}
DrmPreview::~DrmPreview() {
	close(drmfd_);
}
static int drm_set_property(int fd, int plane_id, char const *name, char const *val) {
	drmModeObjectPropertiesPtr properties = nullptr;
	drmModePropertyPtr prop = nullptr;
	int ret = -1;
	properties = drmModeObjectGetProperties(fd, plane_id, DRM_MODE_OBJECT_PLANE);

	for (unsigned int i = 0; i < properties->count_props; i++) {
		int prop_id = properties->props[i];
		prop = drmModeGetProperty(fd, prop_id);
		if (!prop) {
			continue;
		}

		if (!drm_property_type_is(prop, DRM_MODE_PROP_ENUM) || !strstr(prop->name, name)) {
			drmModeFreeProperty(prop);
			prop = nullptr;
			continue;
		}

		for (int j = 0; j < prop->count_enums; j++) {
			if (!strstr(prop->enums[j].name, val)) {
				continue;
			}

			ret = drmModeObjectSetProperty(fd, plane_id, DRM_MODE_OBJECT_PLANE, prop_id, prop->enums[j].value);
			if (ret < 0) {
				HPVT_logging(LOG_LEVEL_ERROR, "DrmPreview: failed to set value %s for property %s", val, name);
			}
			goto done;
		}

		HPVT_logging(LOG_LEVEL_ERROR, "DrmPreview: failed to find value %s for property %s\n", val, name);
		goto done;
	}

	HPVT_logging(LOG_LEVEL_ERROR, "DrmPreview: failed to find property %s\n", name);
	done: if (prop) {
		drmModeFreeProperty(prop);
	}
	if (properties) {
		drmModeFreeObjectProperties(properties);
	}
	return ret;
}
void DrmPreview::makeBuffer(int fd, size_t size, StreamInfo const &info, Buffer &buffer) {
	if (first_time_) {
		first_time_ = false;
		drm_set_property(drmfd_, planeId_, "COLOR_ENCODING", "709");
		drm_set_property(drmfd_, planeId_, "COLOR_RANGE", "limited");
	}

	buffer.fd = fd;
	buffer.size = size;
	buffer.info = info;

	if (drmPrimeFDToHandle(drmfd_, fd, &buffer.bo_handle)) {
		HPVT_logging(LOG_LEVEL_ERROR, "rmPrimeFDToHandle failed for fd %d", fd);
		throw std::runtime_error("drmPrimeFDToHandle failed for fd " + std::to_string(fd));
	}

	uint32_t offsets[4] = {
			0, info.stride * info.height_encoded, info.stride * info.height_encoded + (info.stride / 2) * (info.height_encoded / 2) };
	uint32_t pitches[4] = {
			info.stride, info.stride / 2, info.stride / 2 };
	uint32_t bo_handles[4] = {
			buffer.bo_handle, buffer.bo_handle, buffer.bo_handle };

	if (drmModeAddFB2(drmfd_, info.width, info.height, out_fourcc_, bo_handles, pitches, offsets, &buffer.fb_handle, 0)) {
		HPVT_logging(LOG_LEVEL_ERROR, "drmModeAddFB2 failed");
		throw std::runtime_error("drmModeAddFB2 failed: " + std::string(ERRSTR));
	}
}
void DrmPreview::Show(int fd, size_t size, StreamInfo &info) {
	Buffer &buffer = buffers_[fd];
	if (buffer.fd == -1) {
		makeBuffer(fd, size, info, buffer);
	}

	unsigned int x_off = 0, y_off = 0;
	unsigned int w = width_, h = height_;
	if (info.width * height_ > width_ * info.height) {
		h = width_ * info.height / info.width, y_off = (height_ - h) / 2;
	}
	else {
		w = height_ * info.width / info.height, x_off = (width_ - w) / 2;
	}

	if (drmModeSetPlane(drmfd_, planeId_, crtcId_, buffer.fb_handle, 0, //
			x_off + x_, y_off + y_, w, h, 0, 0, buffer.info.width << 16, buffer.info.height << 16)) {
		HPVT_logging(LOG_LEVEL_ERROR, "drmModeSetPlane failed");
		throw std::runtime_error("drmModeSetPlane failed: " + std::string(ERRSTR));
	}

	last_fd_ = fd;
}
void DrmPreview::Reset() {
	for (auto &it : buffers_) {
		drmModeRmFB(drmfd_, it.second.fb_handle);
	}
	buffers_.clear();
	last_fd_ = -1;
	first_time_ = true;
}
Preview *make_drm_preview() {
	return new DrmPreview();
}
