#ifndef VIEW_VIDFILTER_H_
#define VIEW_VIDFILTER_H_

#include "gmath/gmath.h"
#include "view.h"
#include "vidfilter.h"
#include "utk/ubertk.h"

class VFUINode;

class ViewVideoFilter : public View {
private:
	utk::Window *toolbox;

public:
	// used while dragging a connection curve
	VFUINode *drag_uin;
	VFConnSocket *drag_sock;
	Vec2 drag_cv[2]; // curve end points in view space

	ViewVideoFilter(Controller *ctrl, Model *model);
	~ViewVideoFilter();

	bool init();
	void shutdown();

	void render();

	void keyboard(int key, bool pressed);
	void mouse_button(int bn, bool pressed, int x, int y);
	void passive_mouse_motion(int x, int y, int dx, int dy);

	// called by vfui to start dragging a connection curve
	void start_conn_drag(VFUINode *uin, VFConnSocket *sock);
	void stop_conn_drag();
	bool is_conn_dragging() const;

	void destroy_ui_node(VFUINode *uin);
};

bool vfui_init();
VFUINode *create_ui_node(VFNodeType type, VideoFilterNode *n);

#endif	// VIEW_VIDFILTER_H_
