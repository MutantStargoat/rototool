#include "view_vidfilter.h"
#include "ui.h"

static void bn_src_click(utk::Event *ev, void *cls);
static void bn_filt_click(utk::Event *ev, void *cls);
static void bn_tap_click(utk::Event *ev, void *cls);

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
	utk::HBox *hbox = create_hbox(toolbox);

	utk::create_button(hbox, "Add source", bn_src_click, this);
	utk::create_button(hbox, "Add filter", bn_filt_click, this);
	utk::create_button(hbox, "Set tap", bn_tap_click, this);

	toolbox->set_size(hbox->get_size() + utk::IVec2(8, 8));
	toolbox->show();

	return true;
}

void ViewVideoFilter::shutdown()
{
	if(toolbox) {
		toolbox->hide();
		utk::destroy_window(toolbox);
	}
}

void ViewVideoFilter::render()
{
}

static void bn_src_click(utk::Event *ev, void *cls)
{
}

static void bn_filt_click(utk::Event *ev, void *cls)
{
}

static void bn_tap_click(utk::Event *ev, void *cls)
{
}
