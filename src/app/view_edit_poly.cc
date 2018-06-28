#include <algorithm>
#include "opengl.h"
#include "view_edit_poly.h"
#include "app.h"

ViewEditPoly::ViewEditPoly(Controller &controller, Model &model, ClipPoly &poly_edit)
	: View(controller, model), poly(poly_edit) {

	highlight_vertex = -1;
	moving = false;
}

ViewEditPoly::~ViewEditPoly() {
}

void ViewEditPoly::render() const {
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

	if (highlight_vertex >= 0 && highlight_vertex < (int)poly.verts.size()) {
		const Vec2 &v = poly.verts[highlight_vertex];

		float sz = 4;

		glBegin(GL_QUADS);
		glColor3f(1, 1, 1);
		glVertex2f(v.x - sz, v.y - sz);
		glVertex2f(v.x - sz, v.y + sz);
		glVertex2f(v.x + sz, v.y + sz);
		glVertex2f(v.x + sz, v.y - sz);
		glEnd();
	}

	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void ViewEditPoly::mouse_button(int bn, bool pressed, int x, int y) {
	if (bn == 0 && pressed) {
		move_highlight_vertex(x, y);
		moving = true;
	}

	if (bn == 0 && !pressed) {
		moving = false;
	}

	if (bn == 2 && pressed) {
		controller.pop_view();
	}
}

void ViewEditPoly::mouse_motion(int x, int y, int dx, int dy) {
	if (moving) {
		move_highlight_vertex(x, y);
	}
}

void ViewEditPoly::passive_mouse_motion(int x, int y, int dx, int dy) {
	Vec2 m(x, y);

	// find closest vertex
	if (poly.verts.size() == 0) {
		return;
	}
	int new_hv = -1;
	float min_dist;
	for (int i = 0; i < (int)poly.verts.size(); i++) {
		const Vec2 &v = poly.verts[i];
		float dist = distance(m, v);
		if (new_hv < 0 || dist < min_dist) {
			new_hv = i;
			min_dist = dist;
		}
	}

	if (new_hv != highlight_vertex) {
		app_redraw();
	}
	highlight_vertex = new_hv;
}

void ViewEditPoly::move_highlight_vertex(float x, float y) {
	if (highlight_vertex < 0 || highlight_vertex >= (int)poly.verts.size()) {
		return;
	}

	Vec2 &v = poly.verts[highlight_vertex];
	v.x = x;
	v.y = y;
	poly.apply(model.clip);
	poly.cache(model.clip); // to update bb
	app_redraw();
}
