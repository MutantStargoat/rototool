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
	BN_FILTER_SOBEL,
	BN_FILTER_GAUSS,

	BN_LAYOUT,

	NUM_BUTTONS
};

static const char *bntext[NUM_BUTTONS] = {
	"Test",
	"Video",
	"Edge detect",
	"Gaussian blur",
	"Auto layout"
};
static const char *seplabels[NUM_BUTTONS] = {
	"Sources",
	0,
	"Filters",
	0,
	"Tools"
};

static const float tapcol[][3] = {
	{1.0, 0.2, 0.2}, {0.2, 1.0, 0.2}, {0.2, 0.2, 1.0},
	{0.9, 0.9, 0.15}, {0.9, 0.15, 0.9}, {0.15, 0.9, 0.9}
};


static utk::Button *buttons[NUM_BUTTONS];

static std::vector<VFUINode*> nodes;

static VFUINode *find_ui_node(const VideoFilterNode *vfn);
static void bn_click(utk::Event *ev, void *cls);
static void bn_tap_begin_drag(utk::Event *ev, void *cls);
static void bn_tap_end_drag(utk::Event *ev, void *cls);
static void prev_size_changed(utk::Event *ev, void *cls);

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

	case VF_NODE_GAUSSIAN:
		uin = new VFUIGaussBlur(n);
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

static void set_desat_col(utk::Drawable *w, float r, float g, float b)
{
	float col[3];
	utk::rgb_to_hsv(r, g, b, col, col + 1, col + 2);
	utk::hsv_to_rgb(col, col + 1, col + 2, col[0], col[1] * 0.5, col[2] * 0.7);
	w->set_color(col[0] * 255.0f, col[1] * 255.0f, col[2] * 255.0f);
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

		buttons[i] = utk::create_button(vbox, bntext[i], bn_click);
	}

	toolbox->set_size(vbox->get_size() + utk::IVec2(8, 8));
	toolbox->show();

	// taps window
	tapwin = utk::create_window(0, 0, 0, 10, 10, "Taps");
	vbox = utk::create_vbox(tapwin);

	static const char * const tapnames[] = {"Color", "Edges"};
	for(int i=0; i<2; i++) {
		int cidx = i % (sizeof tapcol / sizeof *tapcol);

		DragButton *bn = create_drag_button(vbox, tapnames[i]);
		set_desat_col(bn, tapcol[cidx][0], tapcol[cidx][1], tapcol[cidx][2]);
		bn->set_callback(EVENT_DRAG_BEGIN, bn_tap_begin_drag, (void*)(intptr_t)i);
		bn->set_callback(EVENT_DRAG_END, bn_tap_end_drag, (void*)(intptr_t)i);
	}

	tapwin->set_pos(0, toolbox->get_frame_height());
	tapwin->set_size(vbox->get_size() + utk::IVec2(8, 8));
	tapwin->show();

	// preview window
	preview = utk::create_window(0, 100, 10, 10, 10, "Preview");
	vbox = utk::create_vbox(preview);

	preview_img = new PreviewImage;
	vbox->add_child(preview_img);

	utk::HBox *hbox = create_hbox(vbox);

	DragButton *ptap = create_drag_button(hbox, "Tap");
	ptap->set_min_size(10, 10);
	utk::IVec2 sz = ptap->get_child()->get_size();
	ptap->set_size(sz.x + 8, sz.y + 8);
	int cidx = VF_PREVIEW_TAP % (sizeof tapcol / sizeof *tapcol);
	set_desat_col(ptap, tapcol[cidx][0], tapcol[cidx][1], tapcol[cidx][2]);
	ptap->set_callback(EVENT_DRAG_BEGIN, bn_tap_begin_drag, (void*)(intptr_t)VF_PREVIEW_TAP);
	ptap->set_callback(EVENT_DRAG_END, bn_tap_end_drag, (void*)(intptr_t)VF_PREVIEW_TAP);

	preview_sizes = create_combobox(hbox);
	preview_sizes->set_size(120, preview_sizes->get_height());
	preview_sizes->add_item("200%");
	preview_sizes->add_item("100%");
	preview_sizes->add_item("50%");
	preview_sizes->add_item("25%");
	preview_sizes->set_readonly(true);
	preview_sizes->select(1);
	preview_sizes->set_callback(utk::EVENT_MODIFY, prev_size_changed, this);

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

	if(tapwin) {
		tapwin->hide();
		utk::destroy_window(tapwin);
		tapwin = 0;
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

	float midx = (x0 + x1) / 2.0f;
	float dt = 1.0f / (float)(seg - 1);
	float t = 0.0f;

	glBegin(GL_LINE_STRIP);
	glColor3f(1, 0.6, 0.2);
	for(int i=0; i<seg; i++) {
		float x = bezier(x0, midx, midx, x1, t);
		float y = bezier(y0, y0, y1, y1, t);
		glVertex2f(x, y);
		t += dt;
	}
	glEnd();

	glPopAttrib();
}

static void draw_tap_tag(int tap, float x, float y, float width)
{
	float height = 10;
	float bev = 5;

	glBegin(GL_QUADS);
	glColor3fv(tapcol[tap % (sizeof tapcol / sizeof *tapcol)]);
	glVertex2f(x, y);
	glVertex2f(x + width, y);
	glVertex2f(x + width - bev, y - height);
	glVertex2f(x + bev, y +- height);
	glEnd();

	glLineWidth(1);
	glBegin(GL_LINE_STRIP);
	glColor3f(1, 1, 1);
	glVertex2f(x, y);
	glVertex2f(x + bev, y - height);
	glVertex2f(x + width - bev, y - height);
	glVertex2f(x + width, y);
	glEnd();

}

#define BEZ_SEG	16
void ViewVideoFilter::render()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, win_width, win_height, 0, -1, 1);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glBegin(GL_QUADS);
	glColor4f(0, 0, 0, 0.5);
	glVertex2f(0, 0);
	glVertex2f(win_width, 0);
	glVertex2f(win_width, win_height);
	glVertex2f(0, win_height);
	glEnd();

	glDisable(GL_BLEND);

	// draw the connection we're currently dragging
	if(is_conn_dragging()) {
		draw_curve(drag_cv[0].x, drag_cv[0].y, drag_cv[1].x, drag_cv[1].y, BEZ_SEG, 0, 0, 0);
	}

	// draw connections between nodes
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

		// find out how many (and which) tap decorators we need to draw
		int num_taps = 0;
		int taps[NUM_VF_TAPS];
		for(int i=0; i<NUM_VF_TAPS; i++) {
			if(vfchain.get_tap(i) == vfn) {
				taps[num_taps++] = i;
			}
		}

		float tags_width = 0.5f * uin->get_frame_width();
		float taptag_width = tags_width / num_taps;
		utk::IVec2 pos = uin->get_frame_global_pos();

		for(int i=0; i<num_taps; i++) {
			draw_tap_tag(taps[i], pos.x, pos.y, taptag_width);
			pos.x += taptag_width;
		}
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

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

void ViewVideoFilter::passive_mouse_motion(int x, int y, int dx, int dy)
{
	if(is_conn_dragging()) {
		drag_cv[1] = Vec2(x, y);
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
	drag_cv[1] = Vec2(app_mouse_x(), app_mouse_y());
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

	case BN_FILTER_SOBEL:
		uin = new VFUISobel;
		break;

	case BN_FILTER_GAUSS:
		uin = new VFUIGaussBlur;
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

static void bn_tap_begin_drag(utk::Event *ev, void *cls)
{
	app_mouse_cursor(CURSOR_CROSS);
}

static void bn_tap_end_drag(utk::Event *ev, void *cls)
{
	int tap = (intptr_t)cls;

	app_mouse_cursor(CURSOR_DEFAULT);

	utk::IVec2 mpos = utk::get_mouse_pos();
	utk::Widget *w = utk::get_root_widget()->get_child_at(mpos.x, mpos.y);
	utk::Window *win = w->get_window();

	VFUINode *uin = 0;

	int num = nodes.size();
	for(int i=0; i<num; i++) {
		if(nodes[i] == win) {
			uin = (VFUINode*)win;
			break;
		}
	}

	if(uin && uin->vfnode) {
		switch(tap) {
		case VF_COLOR_TAP:
			vfchain.set_tap(VF_COLOR_TAP, uin->vfnode);
			controller.redraw_video();
			app_redraw();
			break;

		case VF_EDGES_TAP:
			vfchain.set_tap(VF_EDGES_TAP, uin->vfnode);
			app_redraw();
			break;

		case VF_PREVIEW_TAP:
			vfchain.set_tap(VF_PREVIEW_TAP, uin->vfnode);
			((ViewVideoFilter*)controller.top_view())->preview_img->invalidate();
			app_redraw();
			break;
		}
	}
}

static void prev_size_changed(utk::Event *ev, void *cls)
{
	ViewVideoFilter *view = (ViewVideoFilter*)cls;
	utk::ComboBox *cb = (utk::ComboBox*)ev->widget;

	int scale_percent = 0;
	sscanf(cb->get_selected_text(), "%d%%", &scale_percent);
	if(scale_percent > 0) {
		view->preview_img->set_scale(scale_percent / 100.0f);
		((utk::Container*)view->preview->get_child())->layout();
		app_redraw();
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
