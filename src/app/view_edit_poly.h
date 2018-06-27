#ifndef _VIEW_EDIT_POLY_H_
#define _VIEW_EDIT_POLY_H_

#include "view.h"

class ViewEditPoly : public View {
public:
	ViewEditPoly(Controller &controller, Model &model, ClipPoly &poly_to_edit);
	virtual ~ViewEditPoly();

	void render() const;

	void mouse_button(int bn, bool pressed, int x, int y);
	void mouse_motion(int x, int y, int dx, int dy);
	void passive_mouse_motion(int x, int y, int dx, int dy);

private:
	void move_highlight_vertex(float x, float y);

private:
	ClipPoly &poly;
	int highlight_vertex;
	bool moving;
};

#endif // _VIEW_EDIT_POLY_H_