#include "utk/ubertk.h"

enum {
	EVENT_DRAG_BEGIN = 42,
	EVENT_DRAG,
	EVENT_DRAG_END
};

class DragButton : public utk::Button {
protected:
	bool dragging;

public:
	explicit DragButton(utk::Widget *child, utk::Callback cb = 0);

	utk::Widget *handle_event(utk::Event *ev);

	virtual void on_drag_begin(utk::Event *ev);
	virtual void on_drag(utk::Event *ev);
	virtual void on_drag_end(utk::Event *ev);

	virtual bool is_dragging() const;
};

DragButton *create_drag_button(utk::Widget *parent, const char *text,
		utk::Callback func = 0,	void *cdata = 0);
DragButton *create_drag_button(utk::Widget *parent, utk::Widget *child,
		utk::Callback func = 0,	void *cdata = 0);
