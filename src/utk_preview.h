#ifndef UTK_PREVIEW_H_
#define UTK_PREVIEW_H_

#include "utk/ubertk.h"
#include "vidfilter.h"

class PreviewImage : public utk::Image {
protected:
	float scale;
	void update();

public:
	PreviewImage();

	void set_scale(float s);

	void draw() const;
};

#endif	/* UTK_PREVIEW_H_ */
