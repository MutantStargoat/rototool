#ifndef _VIEW_CLIP_H_
#define _VIEW_CLIP_H_

#include "view.h"

class ViewClip : public View {
private:
	int highlight_poly;

public:
	ViewClip(Controller &controller, Model &model);
	virtual ~ViewClip();

	void render();

	void mouse_button(int bn, bool pressed, int x, int y);
	void mouse_motion(int x, int y, int dx, int dy);
	void passive_mouse_motion(int x, int y, int dx, int dy);

	void update_hover(int x = -1, int y = -1);
};

#endif // _VIEW_CLIP_H_
