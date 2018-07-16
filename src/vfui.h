#ifndef VIDFILTER_UI_H_
#define VIDFILTER_UI_H_

#include "utk/ubertk.h"
#include "vidfilter.h"

class VFUINode : public utk::Window {
protected:
	VideoFilterNode *vfnode;
	utk::Image *img_preview;
	utk::Button *bn_close, *bn_conn;
	VFUINode *prev, *next;

public:

	explicit VFUINode(VideoFilterNode *vfn = 0);
	virtual ~VFUINode();

	virtual bool init();
	virtual void destroy();

	virtual void connect(VFUINode *n);
};

class VFUITestSrc : public VFUINode {
private:
	utk::Entry *tx_width, *tx_height;

public:
	explicit VFUITestSrc(VideoFilterNode *vfn = 0);
	~VFUITestSrc();

	bool init();
	void destroy();
};

class VFUIVideoSrc : public VFUINode {
private:
	utk::Entry *tx_fname;
	utk::Button *bn_open;

public:
	explicit VFUIVideoSrc(VideoFilterNode *vfn = 0);
	~VFUIVideoSrc();

	bool init();
	void destroy();
};

class VFUISobel : public VFUINode {
public:
	explicit VFUISobel(VideoFilterNode *vfn = 0);
};

#endif	// VIDFILTER_UI_H_
