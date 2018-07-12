#ifndef _VIEW_INSERT_POLY_H_
#define _VIEW_INSERT_POLY_H_

#include "view.h"

class ViewInsertPoly : public View {
private:
	int start_mouse_pos[2];
	int curr_mouse_pos[2];

	void insert_poly();

public:
	ViewInsertPoly(Controller *controller, Model *model, int x, int y);
	virtual ~ViewInsertPoly();

	void render();

	void mouse_button(int bn, bool pressed, int x, int y);
	void mouse_motion(int x, int y, int dx, int dy);
};

#endif // _VIEW_INSERT_POLY_H_
