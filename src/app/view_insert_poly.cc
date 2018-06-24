#include <algorithm>

#include <GL/glut.h>

#include "view_insert_poly.h"

ViewInsertPoly::ViewInsertPoly(Controller &controller, Model &model, int x, int y) : View(controller, model) {
	start_mouse_pos[0] = curr_mouse_pos[0] = x;
	start_mouse_pos[1] = curr_mouse_pos[1] =  y;
}

ViewInsertPoly::~ViewInsertPoly() {
}

void ViewInsertPoly::render() const {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH)-1, glutGet(GLUT_WINDOW_HEIGHT)-1, 0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);

	glBegin(GL_LINES);
	glColor3f(1, 1, 1);
	glVertex2iv(start_mouse_pos);
	glVertex2iv(curr_mouse_pos);
	glEnd();

	int min[2] = {std::min(start_mouse_pos[0], curr_mouse_pos[0]), std::min(start_mouse_pos[1], curr_mouse_pos[1])};
	int max[2] = { std::max(start_mouse_pos[0], curr_mouse_pos[0]), std::max(start_mouse_pos[1], curr_mouse_pos[1]) };

	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	glVertex2i(min[0], min[1]);
	glVertex2i(min[0], max[1]);
	glVertex2i(max[0], max[1]);
	glVertex2i(max[0], min[1]);
	glEnd();

	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void ViewInsertPoly::mouse_button(int bn, bool pressed, int x, int y) {
	if (bn == 0 && !pressed) {
		// this will delete us
		controller.pop_view();
	}
}

void ViewInsertPoly::mouse_motion(int x, int y, int dx, int dy) {
	curr_mouse_pos[0] = x;
	curr_mouse_pos[1] = y;
	glutPostRedisplay();
}