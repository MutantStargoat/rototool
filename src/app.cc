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

static int dbgx, dbgy;

bool app_init(int argc, char **argv)
{
	if(init_opengl() == -1) {
		return false;
	}

	if(init_filters() == -1) {
		return false;
	}
	assert(glGetError() == GL_NO_ERROR);

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

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	Vec2 vp = scr_to_view(dbgx, dbgy);
	printf("DBG: [%d %d] -> (%g %g)", dbgx, dbgy, vp.x, vp.y);
	if(controller.model && controller.model->video.is_open()) {
		vp = scr_to_vid(&controller.model->video, dbgx, dbgy);
		printf(" {%g %g}\n", vp.x, vp.y);
	} else {
		putchar('\n');
	}

	assert(glGetError() == GL_NO_ERROR);
}

void app_reshape(int x, int y)
{
	glViewport(0, 0, x, y);

	update_proj();

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj_mat[0]);
}

#define PAN_SCALE	(1.0f / (win_width * view_zoom))

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
}

void app_mouse_button(int bn, bool pressed, int x, int y)
{
	bnstate[bn] = pressed;
	prev_mx = x;
	prev_my = y;

	if(bn == 0 && pressed) {
		dbgx = x;
		dbgy = y;
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
