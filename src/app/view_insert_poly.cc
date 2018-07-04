#include <algorithm>
#include "opengl.h"
#include "view_insert_poly.h"
#include "app.h"
#include "vport.h"

ViewInsertPoly::ViewInsertPoly(Controller &controller, Model &model, int x, int y)
	: View(controller, model)
{
	type = VIEW_INSERT;
	start_mouse_pos[0] = curr_mouse_pos[0] = x;
	start_mouse_pos[1] = curr_mouse_pos[1] = y;
}

ViewInsertPoly::~ViewInsertPoly() {
}

void ViewInsertPoly::render()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(view_mat[0]);

	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);

	Vec2 rect[] = {
		scr_to_view(start_mouse_pos[0], start_mouse_pos[1]),
		scr_to_view(curr_mouse_pos[0], curr_mouse_pos[1])
	};

	static const float col[][4] = {
		{0.5, 0.5, 0.5, 1},
		{1, 0.8, 0.2, 1}
	};

	glLineWidth(3.0);
	for(int i=0; i<2; i++) {
		if(i == 0) {
			glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
		} else {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		glBegin(i == 0 ? GL_QUADS : GL_LINE_LOOP);
		glColor4fv(col[i]);
		glVertex2f(rect[0].x, rect[0].y);
		glVertex2f(rect[1].x, rect[0].y);
		glVertex2f(rect[1].x, rect[1].y);
		glVertex2f(rect[0].x, rect[1].y);
		glEnd();
	}

	glPopAttrib();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void ViewInsertPoly::mouse_button(int bn, bool pressed, int x, int y)
{
	curr_mouse_pos[0] = x;
	curr_mouse_pos[1] = y;
	app_redraw();

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
	app_redraw();
}

void ViewInsertPoly::insert_poly() {
	int base_index = (int)model.clip.verts.size();

	Vec2 rect[] = {
		scr_to_view(start_mouse_pos[0], start_mouse_pos[1]),
		scr_to_view(curr_mouse_pos[0], curr_mouse_pos[1])
	};

	float min[2] = { std::min(rect[0].x, rect[1].x), std::min(rect[0].y, rect[1].y) };
	float max[2] = { std::max(rect[0].x, rect[1].x), std::max(rect[0].y, rect[1].y) };

	// insert verts
	ClipVertex cv;
	cv.pos = Vec2(min[0], max[1]);
	model.clip.verts.push_back(cv);
	cv.pos = Vec2(min[0], min[1]);
	model.clip.verts.push_back(cv);
	cv.pos = Vec2(max[0], min[1]);
	model.clip.verts.push_back(cv);
	cv.pos = Vec2(max[0], max[1]);
	model.clip.verts.push_back(cv);

	// insert poly
	ClipPoly poly;
	for (int i = 0; i < 4; i++) {
		poly.push_back(base_index++);
	}

	poly.cache(model.clip);
	model.clip.polys.push_back(poly);
}
