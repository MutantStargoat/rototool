#ifndef VIDFILTER_H_
#define VIDFILTER_H_

#include "video/video.h"

struct VideoFrame {
	int width, height;
	unsigned char *pixels;
};

class VideoFilterNode;
class VideoFilterChain;

enum {
	VF_FRONT = -1,
	VF_BACK = -2,

	VF_COLOR_TAP = -3	// only valid on get_frame
};

enum VFNodeType {
	VF_NODE_UNKNOWN,
	VF_NODE_SOURCE,
	VF_NODE_FILTER,
	VF_NODE_SDR_FILTER
};

extern VideoFilterChain vfchain;

class VideoFilterChain {
private:
	VideoFilterNode *vflist, *vftail;
	int vflist_size;
	int color_tap;

public:
	VideoFilterChain();

	void clear();
	bool empty() const;
	int size() const;

	void insert_node(VideoFilterNode *n, int at = VF_BACK);
	void remove_node(VideoFilterNode *n);
	void delete_node(VideoFilterNode *n);

	void process();

	VideoFrame *get_frame(int at = VF_BACK) const;

	void set_color_tap(int at);
	int get_color_tap() const;

	void seek_video_source(int frm);
};

class VideoFilterNode {
public:
	VFNodeType type;
	bool status;
	VideoFrame frm;

	VideoFilterNode *prev, *next;

	VideoFilterNode();
	virtual ~VideoFilterNode();

	virtual void process(const VideoFrame *in) = 0;
};

class VFSource : public VideoFilterNode {
public:
	int frameno;

	VFSource();

	virtual void set_size(int w, int h);
	virtual void set_frame_number(int n);
	virtual void process(const VideoFrame *in);
};

class VFVideoSource : public VFSource {
public:
	Video *vid;

	VFVideoSource();

	virtual void set_source(Video *v);
	virtual void process(const VideoFrame *in);
};


class VFShader : public VideoFilterNode {
protected:
	bool own_sdr;

	virtual void prepare();

public:
	unsigned int sdr;

	VFShader();
	virtual ~VFShader();

	virtual bool load_shader(const char *vsfile, const char *psfile);
	virtual void set_shader(unsigned int sdr);

	virtual void process(const VideoFrame *in);
};

class VFSobel : public VFShader {
protected:
	virtual void prepare();
};

class VFGaussBlur : public VFShader {
protected:
	virtual void prepare();

public:
	float sdev;

	VFGaussBlur();

	virtual void set_sdev(float s);
};


#endif	/* VIDFILTER_H_ */
