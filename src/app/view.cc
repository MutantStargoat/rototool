#include <opengl.h>

#include "view.h"

View::View(Controller &c, Model &m)
	: controller(c), model(m)
{
	stacked_input = false;
	type = VIEW_UNKNOWN;
}

View::~View() {
}

bool View::init()
{
	return true;
}

void View::shutdown()
{
}

void View::render()
{
}

void View::keyboard(int key, bool pressed) {}
void View::mouse_button(int bn, bool pressed, int x, int y) {}
void View::mouse_motion(int x, int y, int dx, int dy) {}
void View::passive_mouse_motion(int x, int y, int dx, int dy) {}
void View::mouse_wheel(int delta) {}

void View::enable_stacked_input(bool enable) {
	stacked_input = enable;
}

bool View::stacked_input_enabled() const {
	return stacked_input;
}

