#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <vector>
#include "model.h"

class View;
class Controller {
private:
	std::vector<View*> view_stack;

	std::string video_file;
	std::string clip_file;

	int mouse_pos[2];

public:
	Model *model;
	View *view; // topmost view - this will receive input

	Controller();
	virtual ~Controller();

	bool init(const char *vidfile, const char *clipfile);
	void update();
	void render();
	void shutdown();

	void keyboard(int key, bool pressed);
	void mouse_button(int bn, bool pressed, int x, int y);
	void mouse_motion(int x, int y, int dx, int dy);
	void passive_mouse_motion(int x, int y, int dx, int dy);
	void mouse_wheel(int delta);

	void push_view(View *v);
	void pop_view();
	View *top_view() const;
	void print_view_stack() const;

	int mouse_x() const;
	int mouse_y() const;

	bool seek_video(int frame);
};

#endif // _CONTROLLER_H_
