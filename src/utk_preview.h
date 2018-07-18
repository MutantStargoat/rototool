#ifndef UTK_PREVIEW_H_
#define UTK_PREVIEW_H_

#include "utk/ubertk.h"
#include "vidfilter.h"

class PreviewImage : public utk::Image {
protected:
	void update();

public:
	PreviewImage();
};

#endif	/* UTK_PREVIEW_H_ */
