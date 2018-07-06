#include <assert.h>
#include "app.h"
#include "opengl.h"
#include "videotex.h"
#include "filters.h"
#include "app/controller.h"
#include "vport.h"
#include "ui.h"
#include "pal.h"

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
	glEnable(GL_MULTISAMPLE);

	if(init_filters() == -1) {
		return false;
	}
	assert(glGetError() == GL_NO_ERROR);

	if(!init_palette(32)) {
		return false;
	}

	char *vidfile = 0;
	char *clipfile = 0;

	if(argc > 1) {
		vidfile = argv[1];
		clipfile = (char*)alloca(strlen(vidfile) + strlen(".clip.txt") + 1);
		sprintf(clipfile, "%s.clip.txt", vidfile);
	}

	if (!controller.init(vidfile, clipfile)) {
		return false;
	}

	if(!init_ui()) {
		return false;
	}

	update_view();
	return true;
}

void app_shutdown()
{
	destroy_ui();
	controller.shutdown();
	destroy_palette();
}

void app_display()
{
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	controller.render();

	draw_ui();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, win_width, 0, win_height, -1, 1);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	assert(glGetError() == GL_NO_ERROR);
}

void app_reshape(int x, int y)
{
	glViewport(0, 0, x, y);

	update_proj();

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj_mat[0]);

	ui_reshape(x, y);
}

#define PAN_SCALE	(1.0f / (win_width * view_zoom))

void app_keyboard(int key, bool pressed)
{
	if(pressed) {
		switch(key) {
		case 'q':
			if(app_get_modifiers() & MODKEY_CTRL) {
				app_quit();
			}
			break;

		case KEY_DEL:
			view_pan_x = view_pan_y = 0;
			view_zoom = 1;
			update_view();
			app_redraw();
			break;

		case KEY_LEFT:
			if(app_get_modifiers() & MODKEY_CTRL) {
				view_pan_x += PAN_SCALE * 10;
				update_view();
				app_redraw();
				return;
			}
			break;

		case KEY_RIGHT:
			if(app_get_modifiers() & MODKEY_CTRL) {
				view_pan_x -= PAN_SCALE * 10;
				update_view();
				app_redraw();
				return;
			}
			break;

		case KEY_UP:
			if(app_get_modifiers() & MODKEY_CTRL) {
				view_pan_y -= PAN_SCALE * 10;
				update_view();
				app_redraw();
				return;
			}
			break;

		case KEY_DOWN:
			if(app_get_modifiers() & MODKEY_CTRL) {
				view_pan_y += PAN_SCALE * 10;
				update_view();
				app_redraw();
				return;
			}
			break;

		default:
			break;
		}
	}

	controller.keyboard(key, pressed);

	ui_keyboard(key, pressed);
}

void app_mouse_button(int bn, bool pressed, int x, int y)
{
	bnstate[bn] = pressed;
	prev_mx = x;
	prev_my = y;

	if(ui_mouse_button(bn, pressed, x, y)) {
		return;
	}

	controller.mouse_button(bn, pressed, x, y);
}

void app_mouse_motion(int x, int y)
{
	int dx = x - prev_mx;
	int dy = y - prev_my;
	prev_mx = x;
	prev_my = y;

	if(!dx && !dy) return;

	if(ui_mouse_motion(x, y)) {
		return;
	}

	if(bnstate[1]) {
		view_pan_x += (float)dx * PAN_SCALE;
		view_pan_y -= (float)dy * PAN_SCALE;
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

	if(ui_mouse_motion(x, y)) {
		return;
	}

	controller.passive_mouse_motion(x, y, dx, dy);
}

void app_mouse_wheel(int delta)
{
	if(app_get_modifiers()) {
		controller.mouse_wheel(delta);
	} else {
		// wheel without modifiers is zoom
		view_zoom += delta * 0.1;
		if(view_zoom < 0.0) view_zoom = 0.0;
		update_view();
		app_redraw();
	}
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
