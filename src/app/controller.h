#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <vector>

class Model;
class View;
class Controller {
public:
	Controller();
	virtual ~Controller();

	bool init();
	void update();
	void render();
	void shutdown();

	void mouse_button(int bn, bool pressed, int x, int y);
	void mouse_motion(int x, int y, int dx, int dy);
	void passive_mouse_motion(int x, int y, int dx, int dy);

	void push_view(View *v);
	void pop_view();


	int mouse_x() const;
	int mouse_y() const;
private:
	Model *model;
	View *view; // topmost view - this will receive input
	std::vector<View*> view_stack;

	int mouse_pos[2];
};

#endif // _CONTROLLER_H_