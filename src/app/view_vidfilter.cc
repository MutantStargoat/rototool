#include <assert.h>
#include "app.h"
#include "gmath/gmath.h"
#include "opengl.h"
#include "view_vidfilter.h"
#include "ui.h"
#include "vidfilter.h"
#include "vfui.h"
#include "vport.h"
#include "utk_preview.h"
#include "utk_drag_button.h"

enum {
	BN_SRC_TEST,
	BN_SRC_VIDEO,
	BN_SRC_SOBEL,

	BN_SET_COLOR_TAP,
	BN_LAYOUT,

	NUM_BUTTONS
};

static const char *bntext[NUM_BUTTONS] = {
	"Test",
	"Video",
	"Edge detect",
	"Set color tap",
	"Auto layout"
};
static const char *seplabels[NUM_BUTTONS] = {
	"Sources",
	0,
	"Filters",
	"Tools"
};

static utk::Button *buttons[NUM_BUTTONS];

static std::vector<VFUINode*> nodes;

static VFUINode *find_ui_node(const VideoFilterNode *vfn);
static void bn_click(utk::Event *ev, void *cls);
static void bn_preview_tap_click(utk::Event *ev, void *cls);
static void bn_tap_begin_drag(utk::Event *ev, void *cls);
static void bn_tap_end_drag(utk::Event *ev, void *cls);

static void layout_ui_nodes();
static void sort_ui_nodes();


bool vfui_init()
{
	if(vfchain.empty()) {
		return true;
	}

	// create UI nodes based on existing nodes in the vfchain ...
	int num_nodes = vfchain.size();
	for(int i=0; i<num_nodes; i++) {
		VideoFilterNode *n = vfchain.get_node(i);
		VFUINode *uin = create_ui_node(n->type, n);
		if(!uin) return false;
		nodes.push_back(uin);
		uin->hide();
	}

	layout_ui_nodes();
	return true;
}


VFUINode *create_ui_node(VFNodeType type, VideoFilterNode *n)
{
	VFUINode *uin = 0;

	switch(type) {
	case VF_NODE_SOURCE:
		uin = new VFUITestSrc(n);
		break;

	case VF_NODE_VIDEO_SOURCE:
		uin = new VFUIVideoSrc(n);
		break;

	case VF_NODE_SOBEL:
		uin = new VFUISobel(n);
		break;

	case VF_NODE_SDR_FILTER:
		//uin = new VFUIShader(n);
		//break;

	case VF_NODE_FILTER:
	case VF_NODE_UNKNOWN:
	default:
		return 0;
	}

	if(uin) {
		if(!uin->init()) {
			delete uin;
			return 0;
		}
	}

	return uin;
}

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
	// toolbox window
	toolbox = utk::create_window(0, 0, 0, 10, 10, "Filter Toolbox");
	utk::VBox *vbox = utk::create_vbox(toolbox);

	for(int i=0; i<NUM_BUTTONS; i++) {
		if(seplabels[i]) {
			utk::create_label(vbox, seplabels[i]);
		}

		if(i == BN_SET_COLOR_TAP) {
			DragButton *bn = create_drag_button(vbox, bntext[i]);
			bn->set_callback(EVENT_DRAG_BEGIN, bn_tap_begin_drag);
			bn->set_callback(EVENT_DRAG_END, bn_tap_end_drag);
		} else {
			buttons[i] = utk::create_button(vbox, bntext[i], bn_click);
		}
	}

	toolbox->set_size(vbox->get_size() + utk::IVec2(8, 8));
	toolbox->show();

	// preview window
	preview = utk::create_window(0, 100, 10, 10, 10, "Preview");
	vbox = utk::create_vbox(preview);

	PreviewImage *pimg = new PreviewImage;
	vbox->add_child(pimg);

	utk::create_button(vbox, "Tap", bn_preview_tap_click);
	preview->show();

	// this is called every time we enter the view, so make sure to show all nodes
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

	if(preview) {
		preview->hide();
		utk::destroy_window(preview);
		preview = 0;
	}

	int num = nodes.size();
	for(int i=0; i<num; i++) {
		nodes[i]->hide();
	}
}

static void draw_curve(float x0, float y0, float x1, float y1, int seg, float r, float g, float b)
{
	glPushAttrib(GL_LINE_BIT | GL_ENABLE_BIT);
	glLineWidth(2);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);

	float midx = (x0 + x1) / 2.0f;
	float dt = 1.0f / (float)(seg - 1);
	float t = 0.0f;

	glBegin(GL_LINE_STRIP);
	glColor3f(1, 1, 1);
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
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glBegin(GL_QUADS);
	glColor4f(0, 0, 0, 0.5);
	glVertex2f(-1, -1);
	glVertex2f(1, -1);
	glVertex2f(1, 1);
	glVertex2f(-1, 1);
	glEnd();

	glDisable(GL_BLEND);

	if(is_conn_dragging()) {
		draw_curve(drag_cv[0].x, drag_cv[0].y, drag_cv[1].x, drag_cv[1].y, BEZ_SEG, 0, 0, 0);
	}

	int num = nodes.size();
	for(int i=0; i<num; i++) {
		VFUINode *uin = nodes[i];
		VideoFilterNode *vfn = uin->vfnode;
		if(!vfn) continue;

		assert(vfn->num_outputs < 2);	// XXX debug
		for(int j=0; j<vfn->num_outputs; j++) {
			VFConnSocket *osock = vfn->outputs + j;
			VFConnSocket *isock = osock->conn;
			if(isock) {
				VFUINode *other_uin = find_ui_node(isock->node);
				assert(other_uin);

				Vec2 cv[2];
				cv[0] = uin->out_pos(j);
				cv[1] = other_uin->in_pos(isock->node->input_index(isock));

				draw_curve(cv[0].x, cv[0].y, cv[1].x, cv[1].y, BEZ_SEG, 0, 0, 0);
			}
		}
	}

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
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

Vec2 vfgui_scr_to_view(float x, float y)
{
	x = 2.0f * x / win_width - 1.0f;
	y = 1.0f - 2.0f * y / win_height;

	if(win_aspect > 1.0f) {
		x *= 0.5f * win_aspect;
		y *= 0.5f;
	} else {
		x *= 0.5f;
		y *= 2.0f / win_aspect;
	}
	return Vec2(x, y);
}

void ViewVideoFilter::passive_mouse_motion(int x, int y, int dx, int dy)
{
	if(is_conn_dragging()) {
		drag_cv[1] = vfgui_scr_to_view(x, y);
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
	drag_cv[1] = vfgui_scr_to_view(app_mouse_x(), app_mouse_y());
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

void ViewVideoFilter::destroy_ui_node(VFUINode *uin)
{
	int num = nodes.size();
	for(int i=0; i<num; i++) {
		if(nodes[i] == uin) {
			nodes.erase(nodes.begin() + i);
			break;
		}
	}

	utk::destroy_window(uin);
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

	case BN_LAYOUT:
		layout_ui_nodes();
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

static void bn_preview_tap_click(utk::Event *ev, void *cls)
{
	printf("tap ... tap-tap\n");
}

static void bn_tap_begin_drag(utk::Event *ev, void *cls)
{
	app_mouse_cursor(CURSOR_CROSS);
}

static void bn_tap_end_drag(utk::Event *ev, void *cls)
{
	app_mouse_cursor(CURSOR_DEFAULT);

	utk::IVec2 mpos = utk::get_mouse_pos();
	utk::Widget *w = utk::get_root_widget()->get_child_at(mpos.x, mpos.y);
	utk::Window *win = w->get_window();

	int num = nodes.size();
	for(int i=0; i<num; i++) {
		if(nodes[i] == win) {
			VFUINode *n = (VFUINode*)win;
			if(n->vfnode) {
				vfchain.set_tap(VF_COLOR_TAP, n->vfnode);
				controller.redraw_video();
				app_redraw();
			}
			break;
		}
	}
}

static void layout_ui_nodes()
{
	sort_ui_nodes();

	int num_nodes = nodes.size();
	for(int i=0; i<num_nodes; i++) {
		int x = win_width * (i + 1) / (num_nodes + 1) - nodes[i]->get_width() / 2;
		int y = win_height / 2 - nodes[i]->get_height() / 2;
		nodes[i]->set_pos(x, y);
	}
	app_redraw();
}


// topological sort for UI nodes
enum {MARK_VISITED = 1, MARK_DONE = 2};

static bool topo_visit(std::vector<VFUINode*> *res, VFUINode **arr, int arrsz, int idx, int *mark)
{
	if(mark[idx] == MARK_DONE) return true;
	if(mark[idx] == MARK_VISITED) return false;	/* error: cycle detected */

	VFUINode *uin = arr[idx];
	VideoFilterNode *vfn = uin->vfnode;
	assert(vfn);

	mark[idx] = MARK_VISITED;

	for(int i=0; i<vfn->num_outputs; i++) {
		VideoFilterNode *outconn = vfn->output_node(i);
		if(outconn) {
			int conn_idx = -1;
			for(int j=0; j<arrsz; j++) {
				if(arr[j]->vfnode == outconn) {
					conn_idx = j;
					break;
				}
			}
			assert(conn_idx != -1);
			topo_visit(res, arr, arrsz, conn_idx, mark);
		}
	}

	res->push_back(arr[idx]);
	return true;
}

static void sort_ui_nodes()
{
	std::vector<VFUINode*> sorted;
	int num_nodes = nodes.size();
	int *mark = (int*)alloca(num_nodes * sizeof *mark);
	memset(mark, 0, num_nodes * sizeof *mark);

	sorted.reserve(num_nodes);

	for(int i=0; i<num_nodes; i++) {
		if(!mark[i]) {
			if(!topo_visit(&sorted, &nodes[0], num_nodes, i, mark)) {
				fprintf(stderr, "can't sort nodes, cycle detected\n");
				return;
			}
		}
	}

	if(sorted.size() != nodes.size()) {
		return;
	}

	for(int i=0; i<num_nodes; i++) {
		int j = num_nodes - i - 1;
		nodes[i] = sorted[j];
	}
}
