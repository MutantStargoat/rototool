#ifndef VIEW_VIDFILTER_H_
#define VIEW_VIDFILTER_H_

#include "view.h"
#include "vidfilter.h"
#include "utk/ubertk.h"

class ViewVideoFilter : public View {
private:
	utk::Window *toolbox;

public:
	ViewVideoFilter(Controller *ctrl, Model *model);
	~ViewVideoFilter();

	bool init();
	void shutdown();

	void render();
};

#endif	// VIEW_VIDFILTER_H_
