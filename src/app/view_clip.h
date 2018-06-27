#ifndef _VIEW_CLIP_H_
#define _VIEW_CLIP_H_

#include "view.h"

class ViewClip : public View {
	friend class Controller;

	ViewClip(Controller &controller, Model &model);
	virtual ~ViewClip();


	void render() const;

	void mouse_button(int bn, bool pressed, int x, int y);
	void mouse_motion(int x, int y, int dx, int dy);
	void passive_mouse_motion(int x, int y, int dx, int dy);

private:
	int highlight_poly;
};

#endif // _VIEW_CLIP_H_