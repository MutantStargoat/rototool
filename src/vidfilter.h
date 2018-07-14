#ifndef VIDFILTER_H_
#define VIDFILTER_H_

#include <vector>
#include <map>
#include "video/video.h"

struct VideoFrame {
	int width, height;
	unsigned char *pixels;
};

class VideoFilterNode;
class VideoFilterChain;

enum VFNodeType {
	VF_NODE_UNKNOWN,
	VF_NODE_SOURCE,
	VF_NODE_FILTER,
	VF_NODE_SDR_FILTER
};

extern VideoFilterChain vfchain;

class VideoFilterChain {
private:
	std::vector<VideoFilterNode*> nodes;
	std::map<int, VideoFilterNode*> taps;

public:
	void clear();
	bool empty() const;

	// add doesn't transfer ownership to the VideoFilterChain
	void add(VideoFilterNode *n);
	// remove doesn't free node memory
	void remove(VideoFilterNode *n);
	bool have_node(VideoFilterNode *n) const;

	void connect(VideoFilterNode *n, VideoFilterNode *to);
	void connect(VideoFilterNode *n, int out, VideoFilterNode *to, int in);
	void disconnect(VideoFilterNode *n, int out = -1);	// default: all outputs
	// disconnect the output of "n" which is connected to "to" if such a connection exists
	void disconnect(VideoFilterNode *n, VideoFilterNode *to);

	void process();

	VideoFrame *get_frame(VideoFilterNode *n) const;
	VideoFrame *get_frame(int tap) const;

	bool set_tap(int tap, VideoFilterNode *n);
	VideoFilterNode *get_tap(int tap) const;

	void seek_video_sources(int frm);
};

class VideoFilterNode {
protected:
	int num_in, num_out;

	virtual void prepare(int width, int height);

public:
	VFNodeType type;
	bool status;
	bool proc_pending;
	VideoFrame frm;

	VideoFilterNode();
	virtual ~VideoFilterNode();

	virtual void set_input(VideoFilterNode *n, int idx = 0);
	virtual VideoFilterNode *input(int idx = 0) const;
	virtual int num_inputs() const;

	virtual void set_output(VideoFilterNode *n, int idx = 0);
	virtual VideoFilterNode *output(int idx = 0) const;
	virtual int num_outputs() const;

	virtual void process() = 0;

	/* commit is called before accessing the frame, to make sure it's updated
	 * with the results of the last process call. by default does nothing.
	 */
	virtual void commit();
};

class VFSource : public VideoFilterNode {
protected:
	VideoFilterNode *out;

	virtual void prepare(int width, int height);

public:
	int frameno;

	VFSource();

	virtual void set_output(VideoFilterNode *n, int idx = 0);
	virtual VideoFilterNode *output(int idx = 0) const;

	virtual void set_size(int w, int h);
	virtual void set_frame_number(int n);
	virtual void process();
};

class VFVideoSource : public VFSource {
public:
	Video *vid;

	VFVideoSource();

	virtual void set_source(Video *v);
	virtual void process();
};


class VFShader : public VideoFilterNode {
protected:
	VideoFilterNode *in, *out;

	bool own_sdr;
	bool commit_pending;

	virtual void prepare(int width, int height);

public:
	unsigned int sdr;
	unsigned int tex;
	int tex_width, tex_height;

	VFShader();
	virtual ~VFShader();

	virtual void set_input(VideoFilterNode *n, int idx = 0);
	virtual VideoFilterNode *input(int idx = 0) const;
	virtual void set_output(VideoFilterNode *n, int idx = 0);
	virtual VideoFilterNode *output(int idx = 0) const;

	virtual bool load_shader(const char *vsfile, const char *psfile);
	virtual void set_shader(unsigned int sdr);

	virtual void process();
	virtual void commit();
};

class VFSobel : public VFShader {
protected:
	virtual void prepare(int width, int height);
};

class VFGaussBlur : public VFShader {
protected:
	virtual void prepare(int width, int height);

public:
	float sdev;

	VFGaussBlur();

	virtual void set_sdev(float s);
};


#endif	/* VIDFILTER_H_ */
