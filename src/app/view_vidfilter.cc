#include "opengl.h"
#include "view_vidfilter.h"
#include "ui.h"
#include "vidfilter.h"
#include "vidfilter_ui.h"

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

	return true;
}

void ViewVideoFilter::shutdown()
{
	if(toolbox) {
		toolbox->hide();
		utk::destroy_window(toolbox);
		toolbox = 0;
	}
}

void ViewVideoFilter::render()
{
	glClearColor(0.3, 0.3, 0.3, 1);
	glClear(GL_COLOR_BUFFER_BIT);
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
