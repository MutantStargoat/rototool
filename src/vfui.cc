#include <assert.h>
#include "vfui.h"
#include "app.h"
#include "app/view_vidfilter.h"
#include "vport.h"

VFUINode::VFUINode(VideoFilterNode *vfn)
{
	vfnode = vfn;
	bn_close = 0;
	bn_conn_in = 0;
	bn_conn_out = 0;
}

VFUINode::~VFUINode()
{
	if(vfnode) {
		vfchain.remove(vfnode);
		delete vfnode;
	}
}

static void conn_handler(utk::Event *ev, void *cls)
{
	VFUINode *uin = (VFUINode*)cls;

	int idx;
	VFConnSocketType sock_type;

	if((idx = uin->button_input_index((utk::Button*)ev->widget)) != -1) {
		sock_type = VF_INPUT_SOCKET;
	} else if((idx = uin->button_output_index((utk::Button*)ev->widget)) != -1) {
		sock_type = VF_OUTPUT_SOCKET;
	} else {
		assert(0);
		return;
	}

	View *view = controller.top_view();
	if(view->type != VIEW_VIDEO_FILTER) {
		return;
	}
	ViewVideoFilter *vfv = (ViewVideoFilter*)view;


	if(vfv->is_conn_dragging()) {
		// we're already dragging the end of a connection, so establish it
		if(sock_type == vfv->drag_sock->type) {
			// we can't connect input to input or output to output
			return;
		}
		if(uin->vfnode == vfv->drag_uin->vfnode) {
			// we can't connect a node to itself
			return;
		}

		int start_idx;
		if(vfv->drag_sock->type == VF_INPUT_SOCKET) {
			start_idx = vfv->drag_uin->vfnode->input_index(vfv->drag_sock);
		} else {
			start_idx = vfv->drag_uin->vfnode->output_index(vfv->drag_sock);
		}

		// connection goes from output to input
		if(sock_type == VF_INPUT_SOCKET) {
			vfchain.connect(vfv->drag_uin->vfnode, start_idx, uin->vfnode, idx);
		} else {
			vfchain.connect(uin->vfnode, idx, vfv->drag_uin->vfnode, start_idx);
		}

		vfv->stop_conn_drag();
	} else {
		VFConnSocket *sock = sock_type == VF_INPUT_SOCKET ? uin->vfnode->inputs + idx :
			uin->vfnode->outputs + idx;

		vfv->start_conn_drag(uin, sock);
	}

	controller.redraw_video();
}

static void del_handler(utk::Event *ev, void *cls)
{
	View *view = controller.top_view();
	if(view->type != VIEW_VIDEO_FILTER) {
		return;
	}
	ViewVideoFilter *vfv = (ViewVideoFilter*)view;

	vfv->destroy_ui_node((VFUINode*)cls);
}

bool VFUINode::init()
{
	if(!vfnode) return false;

	set_text("node");
	utk::WinFrame *frm = new utk::WinFrame(this);
	utk::get_root_widget()->add_child(frm);
	rise();
	show();

	utk::VBox *winbox = create_vbox(this);
	uibox = create_vbox(winbox);

	connbox = create_hbox(winbox);

	utk::VBox *inbox = create_vbox(connbox);
	create_label(inbox, "in");
	utk::VBox *outbox = create_vbox(connbox);
	create_label(outbox, "out");

	if(vfnode->num_inputs) {
		bn_conn_in = new utk::Button*[vfnode->num_inputs];

		for(int i=0; i<vfnode->num_inputs; i++) {
			char name[16];
			sprintf(name, "%c", 'A' + i);
			bn_conn_in[i] = create_button(inbox, name, conn_handler, this);
			bn_conn_in[i]->set_min_size(10, 10);
			utk::Widget *lab = bn_conn_in[i]->get_child();
			int w = lab->get_size().x + 4;
			bn_conn_in[i]->set_size(w, bn_conn_in[i]->get_height());
		}
		inbox->layout();
	}

	if(vfnode->num_outputs) {
		bn_conn_out = new utk::Button*[vfnode->num_outputs];

		for(int i=0; i<vfnode->num_outputs; i++) {
			char name[16];
			sprintf(name, "%c", 'A' + i);
			bn_conn_out[i] = create_button(outbox, name, conn_handler, this);
			bn_conn_out[i]->set_min_size(10, 10);
			utk::Widget *lab = bn_conn_out[i]->get_child();
			int w = lab->get_size().x + 4;
			bn_conn_out[i]->set_size(w, bn_conn_out[i]->get_height());
		}
		outbox->layout();
	}

	bn_close = create_button(winbox, "delete", del_handler, this);
	utk::Widget *lab = bn_close->get_child();
	int w = lab->get_size().x + 8;
	int h = lab->get_size().y + 8;
	bn_close->set_min_size(w, h);
	bn_close->set_size(w, h);
	winbox->layout();

	set_pos(100, 100);
	set_size(winbox->get_size() + utk::IVec2(8, 8));
	show();
	return true;
}

int VFUINode::button_input_index(const utk::Button *bn) const
{
	if(bn_conn_in && vfnode) {
		for(int i=0; i<vfnode->num_inputs; i++) {
			if(bn == bn_conn_in[i]) {
				return i;
			}
		}
	}
	return -1;
}

int VFUINode::button_output_index(const utk::Button *bn) const
{
	if(bn_conn_out && vfnode) {
		for(int i=0; i<vfnode->num_outputs; i++) {
			if(bn == bn_conn_out[i]) {
				return i;
			}
		}
	}
	return -1;
}

Vec2 VFUINode::in_pos(int idx) const
{
	float x = get_frame_global_pos().x;
	float y = bn_conn_in[idx]->get_global_pos().y + bn_conn_in[idx]->get_height() / 2.0f;
	return Vec2(x, y);
}

Vec2 VFUINode::out_pos(int idx) const
{
	float x = get_frame_global_pos().x + get_frame_width();
	float y = bn_conn_out[idx]->get_global_pos().y + bn_conn_out[idx]->get_height() / 2.0f;
	return Vec2(x, y);
}

VFUITestSrc::VFUITestSrc(VideoFilterNode *vfn)
{
	vfnode = vfn;
	tx_width = tx_height = 0;
}

VFUITestSrc::~VFUITestSrc()
{
}

bool VFUITestSrc::init()
{
	if(!vfnode) {
		vfnode = new VFSource;
		vfchain.add(vfnode);
	}

	if(!VFUINode::init()) {
		return false;
	}
	set_text("test src");
	return true;
}

VFUIVideoSrc::VFUIVideoSrc(VideoFilterNode *vfn)
{
	vfnode = vfn;
	tx_fname = 0;
	bn_open = 0;
}

VFUIVideoSrc::~VFUIVideoSrc()
{
}

bool VFUIVideoSrc::init()
{
	if(!vfnode) {
		vfnode = new VFVideoSource;
		vfchain.add(vfnode);
	}

	if(!VFUINode::init()) {
		return false;
	}
	set_text("video");
	return true;
}

VFUISobel::VFUISobel(VideoFilterNode *vfn)
{
	vfnode = vfn;
}

bool VFUISobel::init()
{
	if(!vfnode) {
		vfnode = new VFSobel;
		vfchain.add(vfnode);
	}

	if(!VFUINode::init()) {
		return false;
	}
	set_text("edgedet");
	return true;
}

VFUIGaussBlur::VFUIGaussBlur(VideoFilterNode *vfn)
{
	vfnode = vfn;
}

static void gauss_dir_modified(utk::Event *ev, void *cls)
{
	VFUIGaussBlur *uin = (VFUIGaussBlur*)ev->widget->get_window();
	VFGaussBlurPass *vfn = (VFGaussBlurPass*)uin->vfnode;

	vfn->dir = (VFPassDir)(intptr_t)cls;
	controller.redraw_video();
}

static void gauss_ksz_spin_modified(utk::Event *ev, void *cls)
{
	VFUIGaussBlur *uin = (VFUIGaussBlur*)ev->widget->get_window();
	VFGaussBlurPass *vfn = (VFGaussBlurPass*)uin->vfnode;
	utk::Entry *tx = (utk::Entry*)cls;

	int delta = ((utk::Button*)ev->widget)->get_text()[0] == '+' ? 1 : -1;
	int val = atoi(tx->get_text()) + delta;

	if(val < 0) return;

	char buf[32];
	sprintf(buf, "%d", val);
	tx->set_text(buf);

	vfn->ksz = val;
	controller.redraw_video();
}

bool VFUIGaussBlur::init()
{
	if(!vfnode) {
		vfnode = new VFGaussBlurPass;
		vfchain.add(vfnode);
	}

	if(!VFUINode::init()) {
		return false;
	}
	set_text("gaussian");

	VFGaussBlurPass *vfn = (VFGaussBlurPass*)vfnode;

	utk::create_radiobox(uibox, "horiz", true, gauss_dir_modified, (void*)(intptr_t)VF_PASS_HORIZ);
	utk::create_radiobox(uibox, "vert", false, gauss_dir_modified, (void*)(intptr_t)VF_PASS_VERT);

	char valstr[32];
	sprintf(valstr, "%d", vfn->ksz);

	utk::HBox *spinbox = utk::create_hbox(uibox);
	utk::Entry *spintx = utk::create_entry(0, valstr, 25);
	utk::create_button(spinbox, "-", 20, 20, gauss_ksz_spin_modified, spintx);
	spinbox->add_child(spintx);
	utk::create_button(spinbox, "+", 20, 20, gauss_ksz_spin_modified, spintx);

	set_size(get_child()->get_size() + utk::IVec2(8, 8));
	return true;
}


VFUIThreshold::VFUIThreshold(VideoFilterNode *vfn)
{
	vfnode = vfn;
}

static void thres_slider_modify(utk::Event *ev, void *cls)
{
	utk::Slider *slider = (utk::Slider*)ev->widget;
	VFUIThreshold *uin = (VFUIThreshold*)slider->get_window();
	VFThreshold *vfn = (VFThreshold*)uin->vfnode;

	int idx = (intptr_t)cls;
	switch(idx) {
	case 0:
		vfn->thres = slider->get_value();
		controller.redraw_video();
		break;

	case 1:
		vfn->smooth = slider->get_value();
		controller.redraw_video();
		break;

	default:
		break;
	}
}

static void thres_inverse_modify(utk::Event *ev, void *cls)
{
	VFUIThreshold *uin = (VFUIThreshold*)ev->widget->get_window();
	VFThreshold *vfn = (VFThreshold*)uin->vfnode;
	utk::CheckBox *cbox = (utk::CheckBox*)ev->widget;

	vfn->inverse = cbox->is_checked();
	controller.redraw_video();
}

bool VFUIThreshold::init()
{
	if(!vfnode) {
		vfnode = new VFThreshold;
		vfchain.add(vfnode);
	}

	if(!VFUINode::init()) {
		return false;
	}
	set_text("threshold");

	VFThreshold *vfn = (VFThreshold*)vfnode;

	utk::create_label(uibox, "threshold");
	utk::create_slider(uibox, 0, 1, thres_slider_modify, (void*)0)->set_value(vfn->thres);
	utk::create_label(uibox, "smoothness");
	utk::create_slider(uibox, 0, 1, thres_slider_modify, (void*)1)->set_value(vfn->smooth);
	utk::create_checkbox(uibox, "invert", vfn->inverse, thres_inverse_modify);

	set_size(get_child()->get_size() + utk::IVec2(8, 8));
	return true;
}
