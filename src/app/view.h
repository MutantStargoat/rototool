#ifndef _VIEW_H_
#define _VIEW_H_

#include "model.h"
#include "controller.h"

class View {
	friend class Controller;
	protected:
	View(Controller &controller, Model &model);
	virtual ~View();

	virtual void render() const {}

	virtual void keyboard(int key, bool pressed) {}
	virtual void mouse_button(int bn, bool pressed, int x, int y) {}
	virtual void mouse_motion(int x, int y, int dx, int dy) {}
	virtual void passive_mouse_motion(int x, int y, int dx, int dy) {}

protected:
	Controller &controller;
	Model &model;
};

#endif // _VIEW_H_