#ifndef VIDFILTER_UI_H_
#define VIDFILTER_UI_H_

#include "gmath/gmath.h"
#include "utk/ubertk.h"
#include "vidfilter.h"

class VFUINode : public utk::Window {
protected:
	utk::Button *bn_close, **bn_conn_in, **bn_conn_out;
	utk::VBox *uibox;
	utk::HBox *connbox;

public:
	VideoFilterNode *vfnode;

	explicit VFUINode(VideoFilterNode *vfn = 0);
	virtual ~VFUINode();

	virtual bool init();

	virtual int button_input_index(const utk::Button *bn) const;
	virtual int button_output_index(const utk::Button *bn) const;

	virtual Vec2 in_pos(int idx) const;
	virtual Vec2 out_pos(int idx) const;
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

	bool init();
};

class VFUIGaussBlur : public VFUINode {
public:
	explicit VFUIGaussBlur(VideoFilterNode *vfn = 0);

	bool init();
};

#endif	// VIDFILTER_UI_H_
