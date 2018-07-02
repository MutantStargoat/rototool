#include "opengl.h"
#include "view_clip.h"
#include "view_edit_poly.h"
#include "view_insert_poly.h"
#include "app.h"
#include "vport.h"

ViewClip::ViewClip(Controller &controller, Model &model) : View(controller, model) {
	highlight_poly = -1;
}

ViewClip::~ViewClip() {

}

void ViewClip::render()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(view_mat[0]);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);

	for (const ClipPoly &poly : model.clip.polys) {
		if (poly.triangles.size() < 3) {
			continue;
		}

		glBegin(GL_TRIANGLES);
		glColor3f(0, 1, 0);
		for (const int i : poly.triangles) {
			const Vec2 &v = poly.verts[i];
			glVertex2f(v.x, v.y);
		}
		glEnd();
	}

	if (highlight_poly >= 0 && highlight_poly < (int)model.clip.polys.size()) {
		const ClipPoly &poly = model.clip.polys[highlight_poly];

		glBegin(GL_TRIANGLES);
		glColor3f(1, 0, 0);
		for (const int i : poly.triangles) {
			const Vec2 &v = poly.verts[i];
			glVertex2f(v.x, v.y);
		}
		glEnd();
	}

	glPopAttrib();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void ViewClip::mouse_button(int bn, bool pressed, int x, int y) {
	if (bn == 0 && pressed) {
		// insert new polygon
		controller.push_view(new ViewInsertPoly(controller, model, x, y));
		app_redraw();
	}

	if (bn == 2 && pressed && highlight_poly >= 0 && highlight_poly < (int) model.clip.polys.size()) {
		// edit polygon
		controller.push_view(new ViewEditPoly(controller, model, model.clip.polys[highlight_poly]));
		app_redraw();
	}
}

void ViewClip::mouse_motion(int x, int y, int dx, int dy) {

}

void ViewClip::passive_mouse_motion(int x, int y, int dx, int dy) {
	Vec2 m = scr_to_view(x, y);
	int new_hp = -1;
	for (int i = 0; i < (int)model.clip.polys.size(); i++) {
		if (model.clip.polys[i].contains(m)) {
			new_hp = i;
			break;
		}
	}
	if (new_hp != highlight_poly) {
		app_redraw();
	}

	highlight_poly = new_hp;
}
