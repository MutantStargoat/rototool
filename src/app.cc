#include <assert.h>
#include "app.h"
#include "opengl.h"
#include "videotex.h"
#include "filters.h"

// updated by main_*.cc
int win_width, win_height;
float win_aspect;

// pass tex=0 to glGenTextures a new one
static unsigned int create_tex(unsigned int tex, int xsz, int ysz, unsigned int pixfmt, void *pixels);

static VideoTexture *vtex;

static float pan_x, pan_y;
static float zoom = 1.0f;

static unsigned int dftex;
static int dftex_width, dftex_height;

#define NULL_TEX_SZ	128
static unsigned int null_tex;

static bool bnstate[8];
static int prev_mx, prev_my;

static bool dbg_show_filt;


bool app_init(int argc, char **argv)
{
	if(init_opengl() == -1) {
		return false;
	}

	if(init_filters() == -1) {
		return false;
	}
	assert(glGetError() == GL_NO_ERROR);

	if(argc >= 2) {
		vtex = new VideoTexture;
		if(!vtex->open(argv[1])) {
			delete vtex;
			return false;
		}
	}

	unsigned char *pixels = new unsigned char[NULL_TEX_SZ * NULL_TEX_SZ * 3];
	unsigned char *pptr = pixels;
	for(int i=0; i<NULL_TEX_SZ; i++) {
		for(int j=0; j<NULL_TEX_SZ; j++) {
			int x = i ^ j;
			*pptr++ = x << 1;
			*pptr++ = x << 2;
			*pptr++ = x << 3;
		}
	}
	null_tex = create_tex(0, NULL_TEX_SZ, NULL_TEX_SZ, GL_RGB, pixels);
	delete [] pixels;

	glEnable(GL_TEXTURE_2D);

	return true;
}

void app_shutdown()
{
	delete vtex;
	glDeleteTextures(1, &null_tex);
}

void app_display()
{
	float img_aspect;

	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	if(vtex) {
		vtex->bind();	// this must be called first to update texture sizes if necessary
		glMatrixMode(GL_TEXTURE);
		vtex->load_tex_scale();

		int width = vtex->get_width();
		int height = vtex->get_height();
		img_aspect = (float)width / (float)height;

		int texw = vtex->get_tex_width();
		int texh = vtex->get_tex_height();
		if(!dftex || dftex_width != texw || dftex_height != texh) {
			dftex = create_tex(dftex, texw, texh, GL_RGB16F, 0);
		}

		// apply sobel filter
		edge_detect(dftex, vtex->get_texture(), width, height);
		gauss_blur(dftex, dftex, width, height, 5.0);
		assert(glGetError() == GL_NO_ERROR);

		if(dbg_show_filt) {
			glBindTexture(GL_TEXTURE_2D, dftex);	// DBG
		} else {
			vtex->bind();
		}

	} else {
		glBindTexture(GL_TEXTURE_2D, null_tex);
		img_aspect = 1.0f;
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(img_aspect * zoom, zoom, zoom);
	glTranslatef(pan_x, pan_y, 0);

	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	glTexCoord2f(0, 1);
	glVertex2f(-0.5f, -0.5f);
	glTexCoord2f(1, 1);
	glVertex2f(0.5f, -0.5f);
	glTexCoord2f(1, 0);
	glVertex2f(0.5f, 0.5f);
	glTexCoord2f(0, 0);
	glVertex2f(-0.5f, 0.5f);
	glEnd();

	assert(glGetError() == GL_NO_ERROR);
}

void app_reshape(int x, int y)
{
	glViewport(0, 0, x, y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(win_aspect >= 1.0f) {
		glScalef(2.0f / win_aspect, 2.0f, 2.0f);
	} else {
		glScalef(2.0f, 2.0f * win_aspect, 2.0f);
	}
}

void app_keyboard(int key, bool pressed)
{
	if(pressed) {
		switch(key) {
		case KEY_ESC:
			app_quit();
			break;

		case KEY_RIGHT:
			if(vtex) {
				vtex->seek_frame_rel(1);
				app_redraw();
			}
			break;

		case KEY_LEFT:
			if(vtex) {
				vtex->seek_frame_rel(-1);
				app_redraw();
			}
			break;

		case KEY_UP:
			if(vtex) {
				vtex->seek_frame_rel(30);
				app_redraw();
			}
			break;

		case KEY_DOWN:
			if(vtex) {
				vtex->seek_frame_rel(-30);
				app_redraw();
			}
			break;

		case KEY_HOME:
			if(vtex) {
				vtex->rewind();
				app_redraw();
			}
			break;

		case KEY_DEL:
			pan_x = pan_y = 0;
			zoom = 1;
			app_redraw();
			break;

		case KEY_F5:
			dbg_show_filt = !dbg_show_filt;
			app_redraw();
			break;

		default:
			break;
		}
	}
}

void app_mouse_button(int bn, bool pressed, int x, int y)
{
	bnstate[bn] = pressed;
	prev_mx = x;
	prev_my = y;
}

void app_mouse_motion(int x, int y)
{
	int dx = x - prev_mx;
	int dy = y - prev_my;
	prev_mx = x;
	prev_my = y;

	if(!dx && !dy) return;

	if(bnstate[1]) {
		float pan_scale = 1.0 / (win_height * zoom);
		pan_x += (float)dx * pan_scale;
		pan_y -= (float)dy * pan_scale;
		app_redraw();
	}
}

void app_mouse_wheel(int delta)
{
	zoom += delta * 0.1;
	app_redraw();
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

static unsigned int create_tex(unsigned int tex, int xsz, int ysz, unsigned int pixfmt, void *pixels)
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
