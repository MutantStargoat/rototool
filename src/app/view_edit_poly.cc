#include <algorithm>
#include "opengl.h"
#include "view_edit_poly.h"
#include "app.h"
#include <vport.h>

ViewEditPoly::ViewEditPoly(Controller &controller, Model &model, ClipPoly &poly_edit)
	: View(controller, model), poly(poly_edit)
{
	printf("foo!\n");
	type = VIEW_EDIT;
	highlight_vertex = -1;
	mode = Mode::NONE;
	ivert_edge_a = ivert_edge_b = -1;
}

ViewEditPoly::~ViewEditPoly() {
}

ClipPoly *ViewEditPoly::get_poly() const
{
	return &poly;
}

void ViewEditPoly::render() {

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(view_mat[0]);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);

	const float sz = std::abs(scr_to_view(4, 0).x - scr_to_view(0, 0).x);

	glDisable(GL_CULL_FACE);

	if (mode == Mode::NONE || mode == Mode::MOVE || mode == Mode::DELETE) {
		if (highlight_vertex >= 0 && highlight_vertex < (int)poly.verts.size()) {
			const Vec2 &v = poly.verts[highlight_vertex];

			glBegin(GL_QUADS);
			glColor3f(1, 1, 1);
			if (mode == Mode::DELETE) {
				glColor3f(1, 0, 0);
			}
			glVertex2f(v.x + sz, v.y - sz);
			glVertex2f(v.x + sz, v.y + sz);
			glVertex2f(v.x - sz, v.y + sz);
			glVertex2f(v.x - sz, v.y - sz);

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

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void ViewEditPoly::keyboard(int key, bool pressed) {

	if (mode == Mode::NONE) {
		if (key == KEY_CTRL && pressed) {
			mode = Mode::INSERT;
			update_ivert(Vec2(app_mouse_x(), app_mouse_y()));
			app_redraw();
			return;
		}

		if (key == KEY_ALT && pressed && poly.size() > 3) {
			mode = Mode::DELETE;
			app_redraw();
			return;
		}
	}

	if (mode == Mode::INSERT) {
		if (key == KEY_CTRL && (!pressed)) {
			mode = Mode::NONE;
			return;
		}
	}

	if (mode == Mode::DELETE) {
		if (key == KEY_ALT && (!pressed)) {
			mode = Mode::NONE;
			return;
		}
	}
}

void ViewEditPoly::mouse_button(int bn, bool pressed, int x, int y) {
	Vec2 m = scr_to_view(x, y);

	if (mode == Mode::NONE) {
		if (bn == 0 && pressed) {
			mode = Mode::MOVE;
			move_highlight_vertex(m);
			return;
		}

		if (bn == 2 && pressed) {
			//auto_color();
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

	if (mode == Mode::DELETE) {
		if (bn == 0 && pressed) {
			delete_highlight_vertex();
			app_redraw();
			return;
		}
	}
}

void ViewEditPoly::mouse_motion(int x, int y, int dx, int dy) {
	if (mode == Mode::MOVE) {
		move_highlight_vertex(scr_to_view(x, y));
	}
}

void ViewEditPoly::passive_mouse_motion(int x, int y, int dx, int dy) {
	Vec2 m = scr_to_view(x, y);

	if (mode == Mode::NONE || mode == Mode::DELETE) {

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
		update_ivert(m);
		app_redraw();
	}
}

void ViewEditPoly::move_highlight_vertex(const Vec2 &m) {
	if (highlight_vertex < 0 || highlight_vertex >= (int)poly.verts.size()) {
		return;
	}

	Vec2 &v = poly.verts[highlight_vertex];
	v = m;
	poly.apply(model.clip);
	poly.cache(model.clip); // to update bb
	app_redraw();
}

void ViewEditPoly::delete_highlight_vertex() {
	if (highlight_vertex < 0 || highlight_vertex >= (int)poly.size()) {
		return;
	}

	poly.erase(poly.begin() + highlight_vertex);
	poly.cache(model.clip);
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


void ViewEditPoly::auto_color() {
	if(!model.video.is_open()) return;

	Vec2 bb_min_vid = view_to_vid(&model.video, poly.bb_min.x, poly.bb_min.y);
	Vec2 bb_max_vid = view_to_vid(&model.video, poly.bb_max.x, poly.bb_max.y);

	int i0 = (int)std::min(bb_min_vid.x, bb_max_vid.x);
	int i1 = (int)std::max(bb_min_vid.x, bb_max_vid.x);
	int j0 = (int)std::min(bb_min_vid.y, bb_max_vid.y);
	int j1 = (int)std::max(bb_min_vid.y, bb_max_vid.y);

	int vw = model.video.GetWidth();
	int vh = model.video.GetHeight();

	unsigned char *pixels;
	model.video.GetFrame(model.get_cur_video_frame(), &pixels);

	Vec3 avg_color = Vec3(0, 0, 0);
	std::vector<Vec3> colors;

	// examine all video pixels inside the bb
	for (int j = j0; j <= j1; j++) {
		if (j < 0 || j >= vh) continue;
		for (int i = i0; i <= i1; i++) {
			if (i < 0 || i >= vw) continue;

			// check if we are inside the polygon
			Vec2 v = vid_to_view(&model.video, i, j);
			if (!poly.contains(v)) continue;

			Vec3 c;
			c.x = pixels[4 * (vw * j + i) + 2];
			c.y = pixels[4 * (vw * j + i) + 1];
			c.z = pixels[4 * (vw * j + i) + 0];
			colors.push_back(c);

			avg_color += c;
		}
	}

	Vec3 best_color(0, 0, 0);
	if (colors.size()) {
		avg_color /= (int)colors.size();
		float min_dist;

		for (int i = 0; i < (int)colors.size(); i++) {
			const Vec3 &c = colors[i];
			float dist = distance(c, avg_color);
			if (i == 0 || dist < min_dist) {
				min_dist = dist;
				best_color = c;
			}
		}
	}

	best_color *= 1.0f / 255.0f;

	poly.color = best_color;
}


