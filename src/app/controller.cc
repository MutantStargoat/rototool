#include "model.h"
#include "view.h"
#include "controller.h"
#include "../clip/clip_io.h"

#include "view_clip.h"
#include "view_video.h"

Controller::Controller()
	: model(nullptr), view(nullptr)
{
}

Controller::~Controller() {

}


bool Controller::init(const char *vidfile, const char *clipfile)
{
	// create a new model
	model = new Model;

	// load video
	if (!model->video.open(vidfile)) {
		fprintf(stderr, "Failed to load video: %s\n", vidfile);
		return false;
	}
	video_file = std::string(vidfile);

	// load clip
	ClipIO io;
	if (!io.load(clipfile, &model->clip)) {
		fprintf(stderr, "Failed to load clip file: %s. A new clip will be created\n", clipfile);
	}
	clip_file = std::string(clipfile);

	// create view stack
	view = new ViewVideo(*this, *model);
	push_view(new ViewClip(*this, *model));

	// initialize views
	if (!view->init()) {
		return false;
	}
	for (View *v : view_stack) {
		if (!v->init()) {
			return false;
		}
	}

	return true;
}

void Controller::update() {

}

void Controller::render() {
	for (View *v : view_stack) {
		v->render();
	}
	if (view) view->render();
}

void Controller::shutdown() {
	model->video.close();

	if (view) {
		view->shutdown();
	}
	for (View *v : view_stack) {
		v->shutdown();
	}
	view_stack.clear();

	delete model;
	model = nullptr;
	delete view;
	view = nullptr;
}

void Controller::keyboard(int key, bool pressed) {

	if (key == 's' && pressed) {
		ClipIO io;
		if (io.save(clip_file.c_str(), model->clip)) {
			printf("Saved to %s\n", clip_file.c_str());
		}
		return;
	}

	for (View *v : view_stack) {
		if (v->stacked_input_enabled()) {
			v->keyboard(key, pressed);
		}
	}

	if (view) {
		view->keyboard(key, pressed);
	}
}

void Controller::mouse_button(int bn, bool pressed, int x, int y) {
	for (View *v : view_stack) {
		if (v->stacked_input_enabled()) {
			v->mouse_button(bn, pressed, x, y);
		}
	}
	if (view) {
		view->mouse_button(bn, pressed, x, y);
	}
}

void Controller::mouse_motion(int x, int y, int dx, int dy) {
	mouse_pos[0] = x;
	mouse_pos[1] = y;

	for (View *v : view_stack) {
		if (v->stacked_input_enabled()) {
			v->mouse_motion(x, y, dx, dy);
		}
	}
	if (view) {
		view->mouse_motion(x, y, dx, dy);
	}
}

void Controller::passive_mouse_motion(int x, int y, int dx, int dy) {
	mouse_pos[0] = x;
	mouse_pos[1] = y;

	for (View *v : view_stack) {
		if (v->stacked_input_enabled()) {
			v->passive_mouse_motion(x, y, dx, dy);
		}
	}
	if (view) {
		view->passive_mouse_motion(x, y, dx, dy);
	}
}

void Controller::mouse_wheel(int delta) {
	for (View *v : view_stack) {
		if (v->stacked_input_enabled()) {
			v->mouse_wheel(delta);
		}
	}
	if (view) {
		view->mouse_wheel(delta);
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
