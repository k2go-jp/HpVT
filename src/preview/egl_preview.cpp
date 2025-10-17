#define FILE_NUMBER "P105"

#include "preview.hpp"

#include <libdrm/drm_fourcc.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <epoxy/egl.h>
#include <epoxy/gl.h>
#include <map>
#include <string>

#include "../network/context.h"

extern HPVT_Context *g_context;

class EglPreview: public Preview {
public:
	EglPreview();
	~EglPreview();
	virtual void Show(int fd, size_t size, StreamInfo &info) override;
	virtual void Reset() override;
	virtual bool Quit() override;
	virtual void MaxImageSize(unsigned int &w, unsigned int &h) const override
	{
		w = max_image_width_;
		h = max_image_height_;
	}

private:
	struct Buffer {
		Buffer() :
				fd(-1), size(0), texture(0) {
		}
		int fd;
		size_t size;
		StreamInfo info;
		GLuint texture;
	};
	void makeWindow(char const *name);
	void makeBuffer(int fd, size_t size, StreamInfo const &info, Buffer &buffer);
	::Display *display_;
	EGLDisplay egl_display_;
	Window window_;
	EGLContext egl_context_;
	EGLSurface egl_surface_;
	std::map<int, Buffer> buffers_;bool first_time_;
	int last_fd_;
	Atom wm_delete_window_;
	int x_;
	int y_;
	int width_;
	int height_;
	unsigned int max_image_width_;
	unsigned int max_image_height_;
};
static GLint compile_shader(GLenum target, const char *source) {
	GLuint s = glCreateShader(target);
	glShaderSource(s, 1, (const GLchar **) &source, NULL);
	glCompileShader(s);

	GLint ok;
	glGetShaderiv(s, GL_COMPILE_STATUS, &ok);

	if (!ok) {
		GLchar *info;
		GLint size;

		glGetShaderiv(s, GL_INFO_LOG_LENGTH, &size);
		info = (GLchar *) malloc(size);

		glGetShaderInfoLog(s, size, NULL, info);
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		throw std::runtime_error("failed to compile shader: " + std::string(info) + "\nsource:\n" + std::string(source));
	}

	return s;
}
static GLint link_program(GLint vs, GLint fs) {
	GLint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);

	GLint ok;
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (!ok) {
		GLint size;
		GLchar *info = NULL;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
		if (size > 1) {
			info = (GLchar *) malloc(size);
			glGetProgramInfoLog(prog, size, NULL, info);
		}
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		throw std::runtime_error("failed to link: " + std::string(info ? info : "<empty log>"));
	}

	return prog;
}
static void gl_setup(int width, int height, int window_width, int window_height) {
	float w_factor = width / (float) window_width;
	float h_factor = height / (float) window_height;
	float max_dimension = std::max(w_factor, h_factor);
	w_factor /= max_dimension;
	h_factor /= max_dimension;
	char vs[256];
	snprintf(vs, sizeof(vs), "attribute vec4 pos;\n"
			"varying vec2 texcoord;\n"
			"\n"
			"void main() {\n"
			"  gl_Position = pos;\n"
			"  texcoord.x = pos.x / %f + 0.5;\n"
			"  texcoord.y = 0.5 - pos.y / %f;\n"
			"}\n", 2.0 * w_factor, 2.0 * h_factor);
	vs[sizeof(vs) - 1] = 0;
	GLint vs_s = compile_shader(GL_VERTEX_SHADER, vs);
	const char *fs = "#extension GL_OES_EGL_image_external : enable\n"
			"precision mediump float;\n"
			"uniform samplerExternalOES s;\n"
			"varying vec2 texcoord;\n"
			"void main() {\n"
			"  gl_FragColor = texture2D(s, texcoord);\n"
			"}\n";
	GLint fs_s = compile_shader(GL_FRAGMENT_SHADER, fs);
	GLint prog = link_program(vs_s, fs_s);

	glUseProgram(prog);

	static const float verts[] = {
			-w_factor, -h_factor, w_factor, -h_factor, w_factor, h_factor, -w_factor, h_factor };
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, verts);
	glEnableVertexAttribArray(0);

}
EglPreview::EglPreview() :
		Preview(), last_fd_(-1), first_time_(true) {
	display_ = XOpenDisplay(NULL);

	if (!display_) {
		HPVT_logging(LOG_LEVEL_NOTICE, "Couldn't open X display");
		throw std::runtime_error("Couldn't open X display");
	}

	egl_display_ = eglGetDisplay(display_);
	if (!egl_display_) {
		HPVT_logging(LOG_LEVEL_NOTICE, "eglGetDisplay failed");
		throw std::runtime_error("eglGetDisplay() failed");
	}

	EGLint egl_major, egl_minor;

	if (!eglInitialize(egl_display_, &egl_major, &egl_minor)) {
		HPVT_logging(LOG_LEVEL_NOTICE, "eglInitialize failed");
		throw std::runtime_error("eglInitialize() failed");
	}

	x_ = 0;
	y_ = 0;
	width_ = 0;
	height_ = 0;

	makeWindow("Preview");

}
EglPreview::~EglPreview() {

	if (egl_surface_) {
		eglDestroySurface(egl_display_, egl_surface_);
		egl_surface_ = nullptr;
	}
	if (egl_context_) {
		eglDestroyContext(egl_display_, egl_context_);
		egl_context_ = nullptr;
	}
	if (egl_display_) {
		eglTerminate(egl_display_);
		egl_display_ = nullptr;
	}
	eglReleaseThread();
	XDestroyWindow(display_, window_);
	XCloseDisplay(display_);
}
static void no_border(Display *display, Window window) {
	static const unsigned MWM_HINTS_DECORATIONS = (1 << 1);
	static const int PROP_MOTIF_WM_HINTS_ELEMENTS = 5;

	typedef struct {
		unsigned long flags;
		unsigned long functions;
		unsigned long decorations;
		long inputMode;
		unsigned long status;
	} PropMotifWmHints;

	PropMotifWmHints motif_hints;
	Atom prop, proptype;
	unsigned long flags = 0;

	motif_hints.flags = MWM_HINTS_DECORATIONS;
	motif_hints.decorations = flags;

	prop = XInternAtom(display, "_MOTIF_WM_HINTS", True);
	if (!prop) {
		return;
	}

	proptype = prop;

	XChangeProperty(display, window, prop, proptype, 32, PropModeReplace, (unsigned char*) &motif_hints, PROP_MOTIF_WM_HINTS_ELEMENTS);
}
void EglPreview::makeWindow(char const *name) {
	int screen_num = DefaultScreen(display_);
	XSetWindowAttributes attr;
	unsigned long mask;
	Window root = RootWindow(display_, screen_num);
	int screen_width = DisplayWidth(display_, screen_num);
	int screen_height = DisplayHeight(display_, screen_num);

	if (HPVT_Context_is_transmitter(g_context) == true) {
		width_ = g_context->settings.video.camera.width;
		height_ = g_context->settings.video.camera.height;
	}
	else {
		width_ = g_context->connection.current_resolution_width;
		height_ = g_context->connection.current_resolution_height;
	}

	if (g_context->settings.video.preview.full_screen == true || x_ + width_ > screen_width || y_ + height_ > screen_height) {
		x_ = y_ = 0;
		width_ = screen_width;
		height_ = screen_height;
	}

	static const EGLint attribs[] = {
			EGL_RED_SIZE, 1, EGL_GREEN_SIZE, 1, EGL_BLUE_SIZE, 1, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE };
	EGLConfig config;
	EGLint num_configs;
	if (!eglChooseConfig(egl_display_, attribs, &config, 1, &num_configs)) {
		HPVT_logging(LOG_LEVEL_ERROR, "couldn't get an EGL visual config");
		throw std::runtime_error("couldn't get an EGL visual config");
	}

	EGLint vid;
	if (!eglGetConfigAttrib(egl_display_, config, EGL_NATIVE_VISUAL_ID, &vid)) {
		HPVT_logging(LOG_LEVEL_ERROR, "eglGetConfigAttrib failed");
		throw std::runtime_error("eglGetConfigAttrib() failed\n");
	}

	XVisualInfo visTemplate = { };
	visTemplate.visualid = (VisualID) vid;
	int num_visuals;
	XVisualInfo *visinfo = XGetVisualInfo(display_, VisualIDMask, &visTemplate, &num_visuals);

	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap(display_, root, visinfo->visual, AllocNone);
	attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	window_ = XCreateWindow(display_, root, x_, y_, width_, height_, 0, visinfo->depth, InputOutput, visinfo->visual, mask, &attr);

	if (g_context->settings.video.preview.full_screen == true) {
		no_border(display_, window_);
	}

	XSizeHints sizehints;
	sizehints.x = x_;
	sizehints.y = y_;
	sizehints.width = width_;
	sizehints.height = height_;
	sizehints.flags = USSize | USPosition;
	XSetNormalHints(display_, window_, &sizehints);
	XSetStandardProperties(display_, window_, name, name, None, (char **) NULL, 0, &sizehints);

	eglBindAPI(EGL_OPENGL_ES_API);

	static const EGLint ctx_attribs[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	egl_context_ = eglCreateContext(egl_display_, config, EGL_NO_CONTEXT, ctx_attribs);
	if (!egl_context_) {
		HPVT_logging(LOG_LEVEL_ERROR, "eglCreateContext failed");
		throw std::runtime_error("eglCreateContext failed");
	}

	XFree(visinfo);

	XMapWindow(display_, window_);

	wm_delete_window_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display_, window_, &wm_delete_window_, 1);

	egl_surface_ = eglCreateWindowSurface(egl_display_, config, reinterpret_cast<EGLNativeWindowType>(window_), NULL);
	if (!egl_surface_) {
		HPVT_logging(LOG_LEVEL_ERROR, "eglCreateWindowSurface failed");
		throw std::runtime_error("eglCreateWindowSurface failed");
	}

	eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, egl_context_);
	int max_texture_size = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

	max_image_width_ = max_image_height_ = max_texture_size;
	eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}
void EglPreview::makeBuffer(int fd, size_t size, StreamInfo const &info, Buffer &buffer) {
	if (first_time_) {
		if (!eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_)) {
			HPVT_logging(LOG_LEVEL_ERROR, "eglMakeCurrent failed");
			throw std::runtime_error("eglMakeCurrent failed");
		}
		gl_setup(info.width, info.height, width_, height_);
		first_time_ = false;
	}

	buffer.fd = fd;
	buffer.size = size;
	buffer.info = info;

	HPVT_logging(LOG_LEVEL_NOTICE, "fd=%d size =%d info.width = %d, height = %d, stride = %d, height_encoded = %d", //
			fd, size, info.width, info.height, info.stride, info.height_encoded);

	EGLint encoding = EGL_ITU_REC601_EXT;
	EGLint range = EGL_YUV_NARROW_RANGE_EXT;

	EGLint attribs[] = {
			EGL_WIDTH, static_cast<EGLint>(info.width), EGL_HEIGHT, static_cast<EGLint>(info.height), //
			EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_YUV420, //
			EGL_DMA_BUF_PLANE0_FD_EXT, fd, //
			EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0, EGL_DMA_BUF_PLANE0_PITCH_EXT, static_cast<EGLint>(info.stride), //
			EGL_DMA_BUF_PLANE1_FD_EXT, fd, //
			EGL_DMA_BUF_PLANE1_OFFSET_EXT, static_cast<EGLint>(info.stride * info.height_encoded), //
			EGL_DMA_BUF_PLANE1_PITCH_EXT, static_cast<EGLint>(info.stride / 2), //
			EGL_DMA_BUF_PLANE2_FD_EXT, fd, //
			EGL_DMA_BUF_PLANE2_OFFSET_EXT, static_cast<EGLint>(info.stride * info.height_encoded + (info.stride / 2) * (info.height_encoded / 2)), //
			EGL_DMA_BUF_PLANE2_PITCH_EXT, static_cast<EGLint>(info.stride / 2), //
			EGL_YUV_COLOR_SPACE_HINT_EXT, encoding, EGL_SAMPLE_RANGE_HINT_EXT, range, EGL_NONE };

	EGLImage image = eglCreateImageKHR(egl_display_, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attribs);
	if (!image) {
		HPVT_logging(LOG_LEVEL_ERROR, "failed to import fd %d", fd);
		return;
	}

	glGenTextures(1, &buffer.texture);
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, buffer.texture);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);

	eglDestroyImageKHR(egl_display_, image);
	HPVT_logging(LOG_LEVEL_NOTICE, "EglPreview makeBuffer");
}
void EglPreview::Show(int fd, size_t size, StreamInfo &info) {
	Buffer &buffer = buffers_[fd];
	if (buffer.fd == -1) {
		makeBuffer(fd, size, info, buffer);
	}

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_EXTERNAL_OES, buffer.texture);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	EGLBoolean success [[maybe_unused]] = eglSwapBuffers(egl_display_, egl_surface_);
	last_fd_ = fd;
}
void EglPreview::Reset() {
	for (auto &it : buffers_) {
		glDeleteTextures(1, &it.second.texture);
	}
	buffers_.clear();
	last_fd_ = -1;
	eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	first_time_ = true;
}
bool EglPreview::Quit() {
	XEvent event;
	while (XCheckTypedWindowEvent(display_, window_, ClientMessage, &event)) {
		if (static_cast<Atom>(event.xclient.data.l[0]) == wm_delete_window_)
			return true;
	}
	return false;
}
Preview *make_egl_preview() {
	return new EglPreview();
}
