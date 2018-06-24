#include "model.h"
#include "view.h"
#include "controller.h"

#include "view_clip.h"

Controller::Controller() : model(nullptr), view(nullptr) {

}

Controller::~Controller() {
	
}


bool Controller::init() {
	delete model;
	model = nullptr;
	delete view;
	view = nullptr;

	// create a new model and view
	model = new Model();
	view = new ViewClip(*this, *model);

	return true;
}

void Controller::update() {

}

void Controller::render() {
	for (const View *v : view_stack) {
		v->render();
	}
	if (view) view->render();
}

void Controller::shutdown() {
	delete model;
	model = nullptr;
	delete view;
	view = nullptr;
}

void Controller::mouse_button(int bn, bool pressed, int x, int y) {
	if (view) {
		view->mouse_button(bn, pressed, x, y);
	}
}

void Controller::mouse_motion(int x, int y, int dx, int dy) {
	mouse_pos[0] = x;
	mouse_pos[1] = y;

	if (view) {
		view->mouse_motion(x, y, dx, dy);
	}
}

void Controller::push_view(View *v) {
	view_stack.push_back(view);
	view = v;
}

void Controller::pop_view() {
	delete view;
	view = nullptr;
	if (view_stack.size()) {
		view = view_stack.back();
		view_stack.pop_back();
	}
}

int Controller::mouse_x() const {
	return mouse_pos[0];
}

int Controller::mouse_y() const {
	return mouse_pos[1];
}
