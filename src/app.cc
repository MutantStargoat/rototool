#include <assert.h>
#include "app.h"
#include "opengl.h"
#include "videotex.h"
#include "filters.h"
#include "app/controller.h"
#include "vport.h"

#ifdef WIN32
#include <malloc.h>
#else
#include <alloca.h>
#endif

Controller controller;

// updated by main_*.cc
int win_width, win_height;
float win_aspect;

static bool bnstate[8];
static int prev_mx, prev_my;

bool app_init(int argc, char **argv)
{
	if(init_opengl() == -1) {
		return false;
	}

	if(init_filters() == -1) {
		return false;
	}
	assert(glGetError() == GL_NO_ERROR);

	if (argc < 2) {
		printf("Usage: rototool video_file\n");
		return false;
	}

	char *clipfile = (char*)alloca(strlen(argv[1]) + strlen(".clip.txt") + 1);
	sprintf(clipfile, "%s.clip.txt", argv[1]);

	if (!controller.init(argv[1], clipfile)) {
		return false;
	}

	update_view();
	return true;
}

void app_shutdown()
{
	controller.shutdown();
}

void app_display()
{
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	controller.render();

	assert(glGetError() == GL_NO_ERROR);
}

void app_reshape(int x, int y)
{
	glViewport(0, 0, x, y);

	update_proj();

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj_mat[0]);
}

void app_keyboard(int key, bool pressed)
{
	if(pressed) {
		switch(key) {
		case KEY_ESC:
			app_quit();
			break;

		case KEY_DEL:
			view_pan_x = view_pan_y = 0;
			view_zoom = 1;
			update_view();
			app_redraw();
			break;

		default:
			break;
		}
	}

	controller.keyboard(key, pressed);
}

void app_mouse_button(int bn, bool pressed, int x, int y)
{
	bnstate[bn] = pressed;
	prev_mx = x;
	prev_my = y;

	controller.mouse_button(bn, pressed, x, y);
}

void app_mouse_motion(int x, int y)
{
	int dx = x - prev_mx;
	int dy = y - prev_my;
	prev_mx = x;
	prev_my = y;

	if(!dx && !dy) return;

	if(bnstate[1]) {
		float pan_scale = 1.0 / (win_height * view_zoom);
		view_pan_x += (float)dx * pan_scale;
		view_pan_y -= (float)dy * pan_scale;
		update_view();
		app_redraw();
		return;
	}

	controller.mouse_motion(x, y, dx, dy);
}

void app_passive_mouse_motion(int x, int y)
{
	int dx = x - prev_mx;
	int dy = y - prev_my;
	prev_mx = x;
	prev_my = y;

	if (!dx && !dy) return;

	controller.passive_mouse_motion(x, y, dx, dy);
}

void app_mouse_wheel(int delta)
{
	view_zoom += delta * 0.1;
	if(view_zoom < 0.0) view_zoom = 0.0;
	update_view();
	app_redraw();
	//controller.mouse_wheel(delta);
}

static unsigned int intfmt2fmt(unsigned int intfmt)
{
	switch(intfmt) {
	case 3:
	case GL_RGB:
	case GL_RGB16F:
	case GL_RGB32F:
	case GL_SRGB:
		return GL_RGB;

	case 4:
	case GL_RGBA:
	case GL_RGBA16F:
	case GL_RGBA32F:
	case GL_SRGB_ALPHA:
		return GL_RGBA;

	default:
		break;
	}
	return intfmt;
}

static unsigned int intfmt2type(unsigned int intfmt)
{
	switch(intfmt) {
	case 3:
	case 4:
	case GL_RGB:
	case GL_SRGB:
	case GL_RGBA:
	case GL_SRGB_ALPHA:
		return GL_UNSIGNED_BYTE;

	case GL_RGB16F:
	case GL_RGB32F:
	case GL_RGBA16F:
	case GL_RGBA32F:
		return GL_FLOAT;

	default:
		break;
	}
	return GL_UNSIGNED_BYTE;
}

unsigned int create_tex(unsigned int tex, int xsz, int ysz, unsigned int pixfmt, void *pixels)
{
	if(!tex) {
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	} else {
		glBindTexture(GL_TEXTURE_2D, tex);
	}
	glTexImage2D(GL_TEXTURE_2D, 0, pixfmt, xsz, ysz, 0, intfmt2fmt(pixfmt),
			intfmt2type(pixfmt), pixels);

	return tex;
}
