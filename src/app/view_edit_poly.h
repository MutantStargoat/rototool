#ifndef _VIEW_EDIT_POLY_H_
#define _VIEW_EDIT_POLY_H_

#include "view.h"
#include "clip/clip.h"

class ViewEditPoly : public View {
private:
	enum class Mode {
		NONE, MOVE, INSERT, DELETE
	};

	Mode mode;

	ClipPoly *poly;
	int highlight_vertex;

	Vec2 ivert;
	int ivert_edge_a, ivert_edge_b;

	void move_highlight_vertex(const Vec2 &m);
	void delete_highlight_vertex();
	void update_ivert(const Vec2 &m);
	int insert_ivert();
	void auto_color();

public:
	ViewEditPoly(Controller *controller, Model *model, ClipPoly *poly_to_edit);
	virtual ~ViewEditPoly();

	ClipPoly *get_poly() const;

	void render();

	void keyboard(int key, bool pressed);
	void mouse_button(int bn, bool pressed, int x, int y);
	void mouse_motion(int x, int y, int dx, int dy);
	void passive_mouse_motion(int x, int y, int dx, int dy);
	void move_z(int offset);
};

#endif // _VIEW_EDIT_POLY_H_
