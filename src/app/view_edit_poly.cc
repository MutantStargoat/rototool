#include <algorithm>
#include "opengl.h"
#include "view_edit_poly.h"
#include "app.h"

ViewEditPoly::ViewEditPoly(Controller &controller, Model &model, ClipPoly &poly_edit)
	: View(controller, model), poly(poly_edit) {

	highlight_vertex = -1;
	mode = Mode::NONE;
	ivert_edge_a = ivert_edge_b = -1;
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

	const float sz = 4;

	if (mode == Mode::NONE || mode == Mode::MOVE) {
		if (highlight_vertex >= 0 && highlight_vertex < (int)poly.verts.size()) {
			const Vec2 &v = poly.verts[highlight_vertex];

			glBegin(GL_QUADS);
			glColor3f(1, 1, 1);
			glVertex2f(v.x - sz, v.y - sz);
			glVertex2f(v.x - sz, v.y + sz);
			glVertex2f(v.x + sz, v.y + sz);
			glVertex2f(v.x + sz, v.y - sz);
			glEnd();
		}
	}

	if (mode == Mode::INSERT) {
		if (ivert_edge_a >= 0) {
			glBegin(GL_QUADS);
			glColor3f(0, 0, 1);
			glVertex2f(ivert.x - sz, ivert.y - sz);
			glVertex2f(ivert.x - sz, ivert.y + sz);
			glVertex2f(ivert.x + sz, ivert.y + sz);
			glVertex2f(ivert.x + sz, ivert.y - sz);
			glEnd();
		}
	}

	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void ViewEditPoly::keyboard(int key, bool pressed) {
	if (mode == Mode::NONE) {
		if (app_get_modifiers() & MODKEY_CTRL) {
			mode = Mode::INSERT;
			update_ivert(Vec2(app_mouse_x(), app_mouse_y()));
			app_redraw();
			return;
		}
	}

	if (mode == Mode::INSERT) {
		if (!(app_get_modifiers() & MODKEY_CTRL)) {
			mode = Mode::NONE;
			return;
		}
	}
}

void ViewEditPoly::mouse_button(int bn, bool pressed, int x, int y) {
	if (mode == Mode::NONE) {
		if (bn == 0 && pressed) {
			mode = Mode::MOVE;
			move_highlight_vertex(x, y);
			return;
		}

		if (bn == 2 && pressed) {
			controller.pop_view();
			return;
		}
	}

	if (mode == Mode::MOVE) {
		if (bn == 0 && !pressed) {
			mode = Mode::NONE;
			return;
		}
	}

	if (mode == Mode::INSERT) {
		if (bn == 0 && pressed) {
			// Let there be vertex
			highlight_vertex = insert_ivert();
			mode = Mode::MOVE;
			return;
		}
	}
}

void ViewEditPoly::mouse_motion(int x, int y, int dx, int dy) {
	if (mode == Mode::MOVE) {
		move_highlight_vertex(x, y);
	}
}

void ViewEditPoly::passive_mouse_motion(int x, int y, int dx, int dy) {
	if (mode == Mode::NONE) {

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

	if (mode == Mode::INSERT) {
		update_ivert(Vec2(x, y));
		app_redraw();
	}
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

void ViewEditPoly::update_ivert(const Vec2 &m) {
	ivert = poly.closest_point(m, &ivert_edge_a, &ivert_edge_b);
}

int ViewEditPoly::insert_ivert() {
	if (ivert_edge_a < 0 || ivert_edge_b < 0) {
		return -1;
	}

	int nindex = (int)model.clip.verts.size();
	ClipVertex cv;
	cv.pos = ivert;
	model.clip.verts.push_back(cv);

	auto it = poly.begin() + ivert_edge_b;
	poly.insert(it, nindex);
	poly.cache(model.clip);

	return ivert_edge_b;
}

