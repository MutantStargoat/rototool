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

	app_redraw();
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
	return true;
}
