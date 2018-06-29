#ifndef _VIEW_EDIT_POLY_H_
#define _VIEW_EDIT_POLY_H_

#include "view.h"

class ViewEditPoly : public View {
public:
	ViewEditPoly(Controller &controller, Model &model, ClipPoly &poly_to_edit);
	virtual ~ViewEditPoly();

	void render() const;

	void keyboard(int key, bool pressed);
	void mouse_button(int bn, bool pressed, int x, int y);
	void mouse_motion(int x, int y, int dx, int dy);
	void passive_mouse_motion(int x, int y, int dx, int dy);

private:
	void move_highlight_vertex(float x, float y);
	void update_ivert(const Vec2 &m);
	int insert_ivert();

	enum class Mode {
		NONE, MOVE, INSERT, DELETE
	};

	Mode mode;

private:
	ClipPoly &poly;
	int highlight_vertex;

	Vec2 ivert;
	int ivert_edge_a, ivert_edge_b;
};

#endif // _VIEW_EDIT_POLY_H_