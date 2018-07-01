#include <opengl.h>

#include "view.h"

View::View(Controller &c, Model &m) : controller(c), model(m) {
	stacked_input = false;
}

View::~View() {
	
}

void View::enable_stacked_input(bool enable) {
	stacked_input = enable;
}

bool View::stacked_input_enabled() const {
	return stacked_input;
}

