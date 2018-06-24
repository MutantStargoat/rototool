#include <GL/glut.h>

#include "view_clip.h"

#include "view_insert_poly.h"

ViewClip::ViewClip(Controller &controller, Model &model) : View(controller, model) {

}

ViewClip::~ViewClip() {

}

void ViewClip::render() const {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH) - 1, glutGet(GLUT_WINDOW_HEIGHT) - 1, 0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH);
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
	
	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void ViewClip::mouse_button(int bn, bool pressed, int x, int y) {
	if (bn == 0 && pressed) {
		// insert new polygon
		controller.push_view(new ViewInsertPoly(controller, model, x, y));
		glutPostRedisplay();
	}
}

void ViewClip::mouse_motion(int x, int y, int dx, int dy) {

}