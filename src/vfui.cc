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

	View *view = controller.top_view();
	if(view->type == VIEW_VIDEO_FILTER) {
		ViewVideoFilter *vfv = (ViewVideoFilter*)view;

		float x = uin->get_frame_global_pos().x + uin->get_frame_width();
		float y = ev->widget->get_global_pos().y + ev->widget->get_height() / 2.0f;
		Vec2 pos = scr_to_view(x, y);

		vfv->start_conn_curve(pos.x, pos.y);
		app_redraw();
	}
}

static void del_handler(utk::Event *ev, void *cls)
{
	utk::destroy_window((utk::Window*)cls);
}

bool VFUINode::init()
{
	//if(!vfnode) return false;

	if(!vfnode) {
		vfnode = new VFSource;
		vfchain.add(vfnode);
	}

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

	int num = vfnode->num_inputs();
	if(num) {
		bn_conn_in = new utk::Button*[num];

		for(int i=0; i<num; i++) {
			sprintf(name, "%c", 'A' + i);
			bn_conn_in[i] = create_button(inbox, name, conn_handler, this);
			bn_conn_in[i]->set_min_size(10, 10);
			utk::Widget *lab = bn_conn_in[i]->get_child();
			int w = lab->get_size().x + 4;
			bn_conn_in[i]->set_size(w, bn_conn_in[i]->get_height());
		}
		inbox->layout();
	}

	num = vfnode->num_outputs();
	if(num) {
		bn_conn_out = new utk::Button*[num];

		for(int i=0; i<num; i++) {
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
