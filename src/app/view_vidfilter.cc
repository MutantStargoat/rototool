#include <assert.h>
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

static VFUINode *find_ui_node(const VideoFilterNode *vfn);
static void bn_click(utk::Event *ev, void *cls);

ViewVideoFilter::ViewVideoFilter(Controller *ctrl, Model *model)
	: View(ctrl, model)
{
	type = VIEW_VIDEO_FILTER;
	toolbox = 0;

	drag_uin = 0;
	drag_sock = 0;
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

static void draw_curve(float x0, float y0, float x1, float y1, int seg, float r, float g, float b)
{
	glPushAttrib(GL_LINE_BIT);
	glLineWidth(2);

	float midx = (x0 + x1) / 2.0f;

	float dt = 1.0f / (float)(seg - 1);
	float t = 0.0f;

	glBegin(GL_LINE_STRIP);
	glColor3f(r, g, b);
	for(int i=0; i<seg; i++) {
		float x = bezier(x0, midx, midx, x1, t);
		float y = bezier(y0, y0, y1, y1, t);
		glVertex2f(x, y);
		t += dt;
	}
	glEnd();

	glPopAttrib();
}

#define BEZ_SEG	16
void ViewVideoFilter::render()
{
	glClearColor(0.3, 0.3, 0.3, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	if(is_conn_dragging()) {
		draw_curve(drag_cv[0].x, drag_cv[0].y, drag_cv[1].x, drag_cv[1].y, BEZ_SEG, 0, 0, 0);
	}
/*
	int num = nodes.size();
	for(int i=0; i<num; i++) {
		VFUINode *uin = nodes[i];
		VideoFilterNode *vfn = uin->vfnode;
		if(!vfn) continue;

		int num_out = vfn->num_outputs();
		for(int j=0; j<num_out; j++) {
			VideoFilterNode *dest = vfn->output(j);
			if(dest) {
				Vec2 start = n->out_pos(j);
				VFUINode *dest_uin = find_ui_node(dest);
				Vec2 end = n->in_pos(

	}
	*/
}

void ViewVideoFilter::keyboard(int key, bool pressed)
{
	if(!pressed) return;

	if(key == KEY_ESC && is_conn_dragging()) {
		stop_conn_drag();
		app_redraw();
	}
}

void ViewVideoFilter::mouse_button(int bn, bool pressed, int x, int y)
{
	if(!pressed) return;

	if(bn != 0 && is_conn_dragging()) {
		stop_conn_drag();
		app_redraw();
	}
}

void ViewVideoFilter::passive_mouse_motion(int x, int y, int dx, int dy)
{
	if(is_conn_dragging()) {
		drag_cv[1] = scr_to_view(x, y);
		app_redraw();
	}
}

void ViewVideoFilter::start_conn_drag(VFUINode *uin, VFConnSocket *sock)
{
	drag_uin = uin;
	drag_sock = sock;

	VideoFilterNode *vfn = uin->vfnode;
	assert(vfn);

	int idx;
	if((idx = vfn->input_index(sock)) != -1) {
		drag_cv[0] = uin->in_pos(idx);
	} else {
		idx = vfn->output_index(sock);
		assert(idx != -1);
		drag_cv[0] = uin->out_pos(idx);
	}
	drag_cv[1] = scr_to_view(app_mouse_x(), app_mouse_y());
	app_redraw();
}

void ViewVideoFilter::stop_conn_drag()
{
	drag_uin = 0;
	app_redraw();
}

bool ViewVideoFilter::is_conn_dragging() const
{
	return drag_uin != 0;
}

static VFUINode *find_ui_node(const VideoFilterNode *vfn)
{
	int num = nodes.size();
	for(int i=0; i<num; i++) {
		if(nodes[i]->vfnode == vfn) {
			return nodes[i];
		}
	}
	return 0;
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

	VFUINode *uin = 0;

	switch(bidx) {
	case BN_SRC_TEST:
		uin = new VFUITestSrc;
		break;

	case BN_SRC_VIDEO:
		uin = new VFUIVideoSrc;
		break;

	case BN_SRC_SOBEL:
		uin = new VFUISobel;
		break;
	}

	if(uin) {
		if(!uin->init()) {
			delete uin;
			return;
		}
		nodes.push_back(uin);
	}
}
