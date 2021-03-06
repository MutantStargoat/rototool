#include <assert.h>
#include "view_video.h"
#include "opengl.h"
#include "filters.h"
#include "vport.h"
#include "vidfilter.h"
#include "view_vidfilter.h"
#include "model.h"
#include "controller.h"
#include "app.h"

ViewVideo::ViewVideo(Controller *controller, Model *model)
	: View(controller, model)
{
	type = VIEW_VIDEO;
	zoom = 1.0f;
	pan_x = pan_y = 0;
	dftex = 0;
	vtex = 0;

	enable_stacked_input(true);
}

ViewVideo::~ViewVideo() {

}

bool ViewVideo::init()
{
	vfchain.clear();

	vtex = new VideoTexture;

	VFVideoSource *vsrc = new VFVideoSource;
	if(model->video.is_open()) {
		vsrc->set_source(&model->video);
	}
	vfchain.add(vsrc);

	vfchain.set_tap(VF_COLOR_TAP, vsrc);	// get color frames from the source
	//vfchain.set_tap(VF_EDGES_TAP, sobel);	// get edge-detected frames from sobel

	if(!vfui_init()) {
		return false;
	}

	return true;
}

void ViewVideo::shutdown() {
	delete vtex;
	vtex = nullptr;
}

void ViewVideo::render()
{
	float img_aspect;

	if(!vtex) return;

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);

	vtex->bind(model->get_cur_video_frame());	// this must be called first to update texture sizes if necessary
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	vtex->load_tex_scale();

	int width = vtex->get_width();
	int height = vtex->get_height();
	img_aspect = (float)width / (float)height;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
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

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}

void ViewVideo::keyboard(int key, bool pressed)
{
	if (pressed) {
		switch (key) {
		case KEY_RIGHT:
			if (vtex) {
				controller->seek_video(model->get_cur_video_frame() + 1);
				app_redraw();
			}
			break;

		case KEY_LEFT:
			if (vtex) {
				controller->seek_video(model->get_cur_video_frame() - 1);
				app_redraw();
			}
			break;

		case KEY_UP:
			if (vtex) {
				controller->seek_video(model->get_cur_video_frame() + 30);
				app_redraw();
			}
			break;

		case KEY_DOWN:
			if (vtex) {
				controller->seek_video(model->get_cur_video_frame() - 30);
				app_redraw();
			}
			break;

		case KEY_HOME:
			if (vtex) {
				controller->seek_video(0);
				app_redraw();
			}
			break;

		case KEY_F5:
			if(vtex->current_tap() == VF_COLOR_TAP) {
				vtex->use_tap(VF_EDGES_TAP);
			} else {
				vtex->use_tap(VF_COLOR_TAP);
			}
			vtex->invalidate();
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
