#ifndef _VIEW_H_
#define _VIEW_H_

#include <app.h>
#include "model.h"
#include "controller.h"

class View {
	friend class Controller;
protected:
	View(Controller &controller, Model &model);
	virtual ~View();

	virtual bool init() { return true; }
	virtual void shutdown() {}

	virtual void render() {}

	virtual void keyboard(int key, bool pressed) {}
	virtual void mouse_button(int bn, bool pressed, int x, int y) {}
	virtual void mouse_motion(int x, int y, int dx, int dy) {}
	virtual void passive_mouse_motion(int x, int y, int dx, int dy) {}
	virtual void mouse_wheel(int delta) {}

	bool stacked_input_enabled() const;

protected:
	void enable_stacked_input(bool enable);

protected:
	Controller &controller;
	Model &model;

private:
	bool stacked_input;
};

#endif // _VIEW_H_