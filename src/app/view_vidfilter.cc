#include "app.h"
#include "gmath/gmath.h"
#include "opengl.h"
#include "view_vidfilter.h"
#include "ui.h"
#include "vidfilter.h"
#include "vfui.h"
#include "vport.h"

enum {
	BN_SRC_TEST,
	BN_SRC_VIDEO,
	BN_SRC_SOBEL,

	NUM_BUTTONS
};

static const char *bntext[NUM_BUTTONS] = {
	"Test",
	"Video",
	"Edge detect"
};
static const char *seplabels[NUM_BUTTONS] = {
	"Sources",
	0,
	"Filters"
};

static utk::Button *buttons[NUM_BUTTONS];

static std::vector<VFUINode*> nodes;

static Vec2 curve[2];
static bool curve_valid;

static void bn_click(utk::Event *ev, void *cls);

ViewVideoFilter::ViewVideoFilter(Controller *ctrl, Model *model)
	: View(ctrl, model)
{
	type = VIEW_VIDEO_FILTER;
	toolbox = 0;
}

ViewVideoFilter::~ViewVideoFilter()
{
	shutdown();
}

bool ViewVideoFilter::init()
{
	toolbox = create_window(ui_root(), 0, 0, 10, 10, "Filter Toolbox");
	utk::VBox *vbox = create_vbox(toolbox);

	for(int i=0; i<NUM_BUTTONS; i++) {
		if(seplabels[i]) {
			utk::create_label(vbox, seplabels[i]);
		}
		buttons[i] = utk::create_button(vbox, bntext[i], bn_click);
	}

	toolbox->set_size(vbox->get_size() + utk::IVec2(8, 8));
	toolbox->show();

	int num = nodes.size();
	for(int i=0; i<num; i++) {
		nodes[i]->show();
	}

	return true;
}

void ViewVideoFilter::shutdown()
{
	if(toolbox) {
		toolbox->hide();
		utk::destroy_window(toolbox);
		toolbox = 0;
	}

	int num = nodes.size();
	for(int i=0; i<num; i++) {
		nodes[i]->hide();
	}
}

#define BEZ_SEG	16
void ViewVideoFilter::render()
{
	glClearColor(0.3, 0.3, 0.3, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	if(curve_valid) {
		glPushAttrib(GL_LINE_BIT);
		glLineWidth(2);

		float midx = (curve[0].x + curve[1].x) / 2.0f;

		float dt = 1.0f / (float)(BEZ_SEG - 1);
		float t = 0.0f;

		glBegin(GL_LINE_STRIP);
		glColor3f(0, 0, 0);
		for(int i=0; i<BEZ_SEG; i++) {
			float x = bezier(curve[0].x, midx, midx, curve[1].x, t);
			float y = bezier(curve[0].y, curve[0].y, curve[1].y, curve[1].y, t);
			glVertex2f(x, y);
			t += dt;
		}
		glEnd();

		glPopAttrib();
	}
}

void ViewVideoFilter::keyboard(int key, bool pressed)
{
	if(!pressed) return;

	if(key == KEY_ESC && curve_valid) {
		curve_valid = false;
		app_redraw();
	}
}

void ViewVideoFilter::mouse_button(int bn, bool pressed, int x, int y)
{
	if(!pressed) return;

	if(bn != 0 && curve_valid) {
		curve_valid = false;
		app_redraw();
	}
}

void ViewVideoFilter::passive_mouse_motion(int x, int y, int dx, int dy)
{
	if(curve_valid) {
		curve[1] = scr_to_view(x, y);
		app_redraw();
	}
}

void ViewVideoFilter::start_conn_curve(float x, float y)
{
	curve[0] = Vec2(x, y);
	curve[1] = scr_to_view(app_mouse_x(), app_mouse_y());
	curve_valid = true;
	app_redraw();
}

void ViewVideoFilter::stop_conn_curve()
{
	curve_valid = false;
	app_redraw();
}

static void bn_click(utk::Event *ev, void *cls)
{
	int bidx = -1;
	for(int i=0; i<NUM_BUTTONS; i++) {
		if(ev->widget == buttons[i]) {
			bidx = i;
			break;
		}
	}

	VFUINode *uin = new VFUINode;
	if(!uin->init()) {
		delete uin;
		return;
	}
	nodes.push_back(uin);

	/*
	switch(bidx) {
	case BN_SRC_TEST:
		nodes.push_back(new VFUITestSrc);
		break;

	case BN_SRC_VIDEO:
		nodes.push_back(new VFUIVideoSrc);
		break;

	case BN_SRC_SOBEL:
		nodes.push_back(new VFUISobel);
		break;
	}
	*/
}
