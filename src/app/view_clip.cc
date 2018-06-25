#include "opengl.h"
#include "view_clip.h"
#include "view_insert_poly.h"
#include "app.h"

ViewClip::ViewClip(Controller &controller, Model &model) : View(controller, model) {

}

ViewClip::~ViewClip() {

}

void ViewClip::render() const {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, win_width, win_height, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);

	for (ClipPoly poly : model.clip.polys) {
		if (!poly.size()) continue;
		poly.push_back(poly.front());

		glBegin(GL_LINE_STRIP);
		glColor3f(0, 1, 0);
		for (int i : poly) {
			const ClipVertex &cv = model.clip.verts[i];
			glVertex2f(cv.pos.x, cv.pos.y);
		}
		glEnd();
	}

	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void ViewClip::mouse_button(int bn, bool pressed, int x, int y) {
	if (bn == 0 && pressed) {
		// insert new polygon
		controller.push_view(new ViewInsertPoly(controller, model, x, y));
		app_redraw();
	}
}

void ViewClip::mouse_motion(int x, int y, int dx, int dy) {

}
