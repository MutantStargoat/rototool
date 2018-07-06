#include "opengl.h"
#include "view_clip.h"
#include "view_edit_poly.h"
#include "view_insert_poly.h"
#include "app.h"
#include "vport.h"
#include "pal.h"

ViewClip::ViewClip(Controller &controller, Model &model)
	: View(controller, model)
{
	type = VIEW_CLIP;
	highlight_poly = -1;
}

ViewClip::~ViewClip() {

}

void ViewClip::render()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(view_mat[0]);

	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);

	glLineWidth(3);

	for (const ClipPoly &poly : model.clip.polys) {
		if (poly.triangles.size() < 3) {
			continue;
		}

		const Vec3 &col = poly.palcol == -1 ? poly.color : palette[poly.palcol];

		glBegin(GL_TRIANGLES);
		glColor3f(col.x, col.y, col.z);
		for (const int i : poly.triangles) {
			const Vec2 &v = poly.verts[i];
			glVertex2f(v.x, v.y);
		}
		glEnd();
	}

	if (highlight_poly >= 0 && highlight_poly < (int)model.clip.polys.size()) {
		const ClipPoly &poly = model.clip.polys[highlight_poly];

		glBegin(GL_LINE_LOOP);
		glColor3f(1, 0.7, 0.1);
		int nverts = poly.verts.size();
		for(int i=0; i<nverts; i++) {
			glVertex2f(poly.verts[i].x, poly.verts[i].y);
		}
		glEnd();
	}

	glPopAttrib();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void ViewClip::mouse_button(int bn, bool pressed, int x, int y) {
	if (highlight_poly >= 0 && highlight_poly < (int)model.clip.polys.size()) {

		if (bn == 0 && pressed) {
			// edit polygon
			controller.push_view(new ViewEditPoly(controller, model, model.clip.polys[highlight_poly]));
			app_redraw();
		}
	}
	else {
		if (bn == 0 && pressed) {
			// insert new polygon
			controller.push_view(new ViewInsertPoly(controller, model, x, y));
			app_redraw();
		}
	}
}

void ViewClip::mouse_motion(int x, int y, int dx, int dy) {

}

void ViewClip::passive_mouse_motion(int x, int y, int dx, int dy)
{
	update_hover(x, y);
}

void ViewClip::update_hover(int x, int y)
{
	if(x == -1) x = app_mouse_x();
	if(y == -1) y = app_mouse_y();

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
