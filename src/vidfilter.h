#ifndef VIDFILTER_H_
#define VIDFILTER_H_

#include <vector>
#include "video/video.h"

struct VideoFrame {
	int width, height;
	unsigned char *pixels;
};

class VideoFilterNode;

enum {
	VF_FRONT = -1,
	VF_BACK = -2
};

class VideoFilterChain {
private:
	std::vector<VideoFilterNode*> nodes;

public:
	void clear();
	bool empty() const;
	int size() const;

	void insert_node(VideoFilterNode *n, int at = VF_BACK);
	void remove_node(VideoFilterNode *n);

	void process();

	VideoFrame *get_frame(int at = VF_BACK) const;
};

class VideoFilterNode {
public:
	bool status;
	VideoFrame frm;

	VideoFilterNode();
	virtual ~VideoFilterNode();

	virtual void process(const VideoFrame *in) = 0;
};

class VFTestSource : public VideoFilterNode {
public:
	virtual void set_size(int w, int h);
	virtual void process(const VideoFrame *in);
};

class VFVideoSource : public VFTestSource {
public:
	Video *vid;
	int frameno;

	VFVideoSource();

	virtual void set_source(Video *v);
	virtual void set_frame_number(int n);
	virtual void process(const VideoFrame *in);
};


class VFShader : public VideoFilterNode {
public:
	unsigned int sdr;
	unsigned int src_tex, dst_tex;

	VFShader();
	virtual ~VFShader();

	virtual bool load_shader(const char *vsfile, const char *psfile);

	virtual void process(const VideoFrame *in);
};

class VFSobel : public VFShader {
public:
	virtual void process(const VideoFrame *in);
};

class VFGaussBlur : public VFShader {
public:
	float sdev;

	VFGaussBlur();

	virtual void set_sdev(float s);
	virtual void process(const VideoFrame *in);
};


#endif	/* VIDFILTER_H_ */
