#include <assert.h>
#include "view_video.h"
#include "opengl.h"
#include "filters.h"
#include "vport.h"

#define NULL_TEX_SZ	128

static unsigned int null_tex;

ViewVideo::ViewVideo(Controller &controller, Model &model)
	: View(controller, model)
{
	zoom = 1.0f;
	pan_x = pan_y = 0;
	dftex = 0;
	dbg_show_filt = false;
	vtex = 0;

	enable_stacked_input(true);
}

ViewVideo::~ViewVideo() {

}

bool ViewVideo::init()
{
	if(model.video.is_open()) {
		vtex = new VideoTexture(model.video);
	}

	if(!null_tex) {
		// create null texture
		unsigned char *pixels = new unsigned char[NULL_TEX_SZ * NULL_TEX_SZ * 3];
		unsigned char *pptr = pixels;
		for (int i = 0; i<NULL_TEX_SZ; i++) {
			for (int j = 0; j<NULL_TEX_SZ; j++) {
				int x = i ^ j;
				*pptr++ = x << 1;
				*pptr++ = x << 2;
				*pptr++ = x << 3;
			}
		}
		null_tex = create_tex(0, NULL_TEX_SZ, NULL_TEX_SZ, GL_RGB, pixels);
		delete [] pixels;
	}

	return true;
}

void ViewVideo::shutdown() {
	delete vtex;
	vtex = nullptr;
}

void ViewVideo::render() {
	float img_aspect;

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);

	if (vtex) {
		vtex->bind();	// this must be called first to update texture sizes if necessary
		glMatrixMode(GL_TEXTURE);
		vtex->load_tex_scale();

		int width = vtex->get_width();
		int height = vtex->get_height();
		img_aspect = (float)width / (float)height;

		int texw = vtex->get_tex_width();
		int texh = vtex->get_tex_height();
		if (!dftex || dftex_width != texw || dftex_height != texh) {
			dftex = create_tex(dftex, texw, texh, GL_RGB16F, 0);
		}

		// apply sobel filter
		edge_detect(dftex, vtex->get_texture(), width, height);
		gauss_blur(dftex, dftex, width, height, 5.0);
		assert(glGetError() == GL_NO_ERROR);

		if (dbg_show_filt) {
			glBindTexture(GL_TEXTURE_2D, dftex);	// DBG
		}
		else {
			vtex->bind();
		}

	} else {
		glBindTexture(GL_TEXTURE_2D, null_tex);
		img_aspect = 1.0f;
	}

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	/*glLoadIdentity();
	glScalef(img_aspect * vport_zoom, vport_zoom, vport_zoom);
	glTranslatef(vport_pan_x, vport_pan_y, 0);
	*/
	glLoadMatrixf(view_mat[0]);
	glScalef(img_aspect, 1, 1);

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

	glPopMatrix();
	glPopAttrib();
}

void ViewVideo::keyboard(int key, bool pressed)
{
	if (pressed) {
		switch (key) {
		case KEY_RIGHT:
			if (vtex) {
				vtex->seek_frame_rel(1);
				app_redraw();
			}
			break;

		case KEY_LEFT:
			if (vtex) {
				vtex->seek_frame_rel(-1);
				app_redraw();
			}
			break;

		case KEY_UP:
			if (vtex) {
				vtex->seek_frame_rel(30);
				app_redraw();
			}
			break;

		case KEY_DOWN:
			if (vtex) {
				vtex->seek_frame_rel(-30);
				app_redraw();
			}
			break;

		case KEY_HOME:
			if (vtex) {
				vtex->rewind();
				app_redraw();
			}
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

void ViewVideo::mouse_button(int bn, bool pressed, int x, int y) {

}

void ViewVideo::mouse_motion(int x, int y, int dx, int dy)
{
}

void ViewVideo::passive_mouse_motion(int x, int y, int dx, int dy)
{
}

void ViewVideo::mouse_wheel(int delta)
{
}
