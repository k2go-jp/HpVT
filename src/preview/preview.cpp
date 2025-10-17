#define FILE_NUMBER "P101"

#include "preview.hpp"

Preview *make_null_preview();
Preview *make_egl_preview();
Preview *make_drm_preview();

Preview *make_preview() {

	try {
		printf("X/EGL preview\n");
		Preview *p = make_egl_preview();
		if (p) {
			printf("Made X/EGL preview window\n");
		}
		return p;
	}
	catch (std::exception const &e) {
		try {
			printf("DRM preview\n");
			Preview *p = make_drm_preview();
			if (p) {
				printf("Made DRM preview window\n");
			}
			return p;

		}
		catch (std::exception const &e) {
			printf("Preview window unavailable\n");
			return make_null_preview();
		}
	}

	return nullptr;
}
