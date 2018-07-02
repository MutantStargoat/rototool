#ifndef _VIEW_H_
#define _VIEW_H_

#include <app.h>
#include "model.h"
#include "controller.h"

class View {
protected:
	Controller &controller;
	Model &model;

	bool stacked_input;

public:
	View(Controller &controller, Model &model);
	virtual ~View();

	virtual bool init();
	virtual void shutdown();

	virtual void render();

	virtual void keyboard(int key, bool pressed);
	virtual void mouse_button(int bn, bool pressed, int x, int y);
	virtual void mouse_motion(int x, int y, int dx, int dy);
	virtual void passive_mouse_motion(int x, int y, int dx, int dy);
	virtual void mouse_wheel(int delta);

	void enable_stacked_input(bool enable);
	bool stacked_input_enabled() const;
};

#endif // _VIEW_H_
