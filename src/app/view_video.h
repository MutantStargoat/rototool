#ifndef _VIEW_VIDEO_H_
#define _VIEW_VIDEO_H_

#include "view.h"
#include <video/video.h>
#include <videotex.h>

class ViewVideo : public View {
	friend class Controller;
protected:
	ViewVideo(Controller &controller, Model &model);
	virtual ~ViewVideo();

	void render();

	bool init();
	void shutdown();

	void keyboard(int key, bool pressed);
	void mouse_button(int bn, bool pressed, int x, int y);
	void mouse_motion(int x, int y, int dx, int dy);
	void passive_mouse_motion(int x, int y, int dx, int dy);
	void mouse_wheel(int delta);

private:
	VideoTexture *vtex;

	float pan_x, pan_y;
	float zoom;

	unsigned int dftex;
	int dftex_width, dftex_height;

	unsigned int null_tex;
	bool dbg_show_filt;
};

#endif // _VIEW_VIDEO_H_