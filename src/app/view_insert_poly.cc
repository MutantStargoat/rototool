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
	curr_mouse_pos[0] = x;
	curr_mouse_pos[1] = y;
	glutPostRedisplay();

	if (bn == 0 && !pressed) {
		if (curr_mouse_pos[0] != start_mouse_pos[0] &&
			curr_mouse_pos[1] != start_mouse_pos[1]) {
			insert_poly();
		}

		// this will delete us
		controller.pop_view();
	}
}

void ViewInsertPoly::mouse_motion(int x, int y, int dx, int dy) {
	curr_mouse_pos[0] = x;
	curr_mouse_pos[1] = y;
	glutPostRedisplay();
}

void ViewInsertPoly::insert_poly() {
	int base_index = (int)model.clip.verts.size();

	int min[2] = { std::min(start_mouse_pos[0], curr_mouse_pos[0]), std::min(start_mouse_pos[1], curr_mouse_pos[1]) };
	int max[2] = { std::max(start_mouse_pos[0], curr_mouse_pos[0]), std::max(start_mouse_pos[1], curr_mouse_pos[1]) };

	// insert verts
	ClipVertex cv;
	cv.pos = Vec2(min[0], min[1]);
	model.clip.verts.push_back(cv);
	cv.pos = Vec2(min[0], max[1]);
	model.clip.verts.push_back(cv);
	cv.pos = Vec2(max[0], max[1]);
	model.clip.verts.push_back(cv);
	cv.pos = Vec2(max[0], min[1]);
	model.clip.verts.push_back(cv);

	// insert poly
	ClipPoly poly;
	for (int i = 0; i < 4; i++) {
		poly.push_back(base_index++);
	}
	model.clip.polys.push_back(poly);
}