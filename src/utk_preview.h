#ifndef UTK_PREVIEW_H_
#define UTK_PREVIEW_H_

#include "utk/ubertk.h"
#include "vidfilter.h"

#define PREVIEW_TAP		42

class PreviewImage : public utk::Image {
protected:
	void update();

public:
	PreviewImage();
};

#endif	/* UTK_PREVIEW_H_ */
