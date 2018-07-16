#ifndef VIEW_VIDFILTER_H_
#define VIEW_VIDFILTER_H_

#include "view.h"
#include "vidfilter.h"
#include "utk/ubertk.h"

class ViewVideoFilter : public View {
private:
	utk::Window *toolbox;

public:
	ViewVideoFilter(Controller *ctrl, Model *model);
	~ViewVideoFilter();

	bool init();
	void shutdown();

	void render();

	void keyboard(int key, bool pressed);
	void mouse_button(int bn, bool pressed, int x, int y);
	void passive_mouse_motion(int x, int y, int dx, int dy);

	// called by vfui to start dragging a connection curve (view space)
	void start_conn_curve(float x, float y);
	void stop_conn_curve();
};

#endif	// VIEW_VIDFILTER_H_
