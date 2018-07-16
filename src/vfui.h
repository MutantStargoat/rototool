#ifndef VIDFILTER_UI_H_
#define VIDFILTER_UI_H_

#include "utk/ubertk.h"
#include "vidfilter.h"

class VFUINode : public utk::Window {
protected:
	VideoFilterNode *vfnode;
	utk::Button *bn_close, **bn_conn_in, **bn_conn_out;
	utk::HBox *connbox;

public:

	explicit VFUINode(VideoFilterNode *vfn = 0);
	virtual ~VFUINode();

	virtual bool init();
};

class VFUITestSrc : public VFUINode {
private:
	utk::Entry *tx_width, *tx_height;

public:
	explicit VFUITestSrc(VideoFilterNode *vfn = 0);
	~VFUITestSrc();

	bool init();
};

class VFUIVideoSrc : public VFUINode {
private:
	utk::Entry *tx_fname;
	utk::Button *bn_open;

public:
	explicit VFUIVideoSrc(VideoFilterNode *vfn = 0);
	~VFUIVideoSrc();

	bool init();
};

class VFUISobel : public VFUINode {
public:
	explicit VFUISobel(VideoFilterNode *vfn = 0);
};

#endif	// VIDFILTER_UI_H_
