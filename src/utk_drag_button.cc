#include <stdio.h>
#include "utk_drag_button.h"

DragButton::DragButton(utk::Widget *child, utk::Callback cb)
	: utk::Button(child, cb)
{
	dragging = false;
}

utk::Widget *DragButton::handle_event(utk::Event *ev)
{
	utk::MButtonEvent *bev = dynamic_cast<utk::MButtonEvent*>(ev);
	if(bev && bev->button == utk::MOUSE_LEFT) {
		if(bev->pressed) {
			if(hit_test(bev->x, bev->y)) {
				ev->widget = this;
				on_drag_begin(ev);
				return this;
			}
		} else {
			if(dragging) {
				ev->widget = this;
				on_drag_end(ev);
				return this;
			}
		}
	}

	if(dynamic_cast<utk::MMotionEvent*>(ev) && utk::get_button_state() == utk::MOUSE_LEFT) {
		if(dragging) {
			ev->widget = this;
			on_drag(ev);
			return this;
		}
	}

	return 0;
}

void DragButton::on_drag_begin(utk::Event *ev)
{
	dragging = true;
	pressed = true;

	callback(ev, EVENT_DRAG_BEGIN);
	utk::grab_mouse(this);
}

void DragButton::on_drag(utk::Event *ev)
{
	callback(ev, EVENT_DRAG);
}

void DragButton::on_drag_end(utk::Event *ev)
{
	dragging = false;
	pressed = false;

	utk::grab_mouse(0);
	callback(ev, EVENT_DRAG_END);
}

bool DragButton::is_dragging() const
{
	return dragging;
}

DragButton *create_drag_button(utk::Widget *parent, const char *text,
		utk::Callback func, void *cdata)
{
	return create_drag_button(parent, new utk::Label(text), func, cdata);
}

DragButton *create_drag_button(utk::Widget *parent, utk::Widget *child,
		utk::Callback func, void *cdata)
{
	DragButton *bn = new DragButton(child);
	bn->set_callback(EVENT_DRAG_END, func, cdata);
	parent->add_child(bn);
	return bn;
}
