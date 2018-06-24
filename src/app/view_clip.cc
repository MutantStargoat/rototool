#include <GL/glut.h>

#include "view_clip.h"

#include "view_insert_poly.h"

ViewClip::ViewClip(Controller &controller, Model &model) : View(controller, model) {

}

ViewClip::~ViewClip() {

}

void ViewClip::render() const {
	
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