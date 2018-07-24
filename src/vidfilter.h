#ifndef VIDFILTER_H_
#define VIDFILTER_H_

#include <vector>
#include <map>
#include "video/video.h"

class VideoFilterNode;
class VideoFilterChain;

struct VideoFrame {
	int width, height;
	unsigned char *pixels;
};

void scale_video_frame(VideoFrame *dest, const VideoFrame *src, float scale);
bool dump_video_frame(VideoFrame *frm, const char *fname);

enum VFNodeType {
	VF_NODE_UNKNOWN,
	// sources
	VF_NODE_SOURCE,
	VF_NODE_VIDEO_SOURCE,

	// filters
	VF_NODE_FILTER,

	// shader-based filters
	VF_NODE_SDR_FILTER,
	VF_NODE_SOBEL,
	VF_NODE_GAUSSBLUR_PASS,
	VF_NODE_THRES
};

#define VF_IS_SOURCE(type) \
	((type) == VF_NODE_SOURCE || (type) == VF_NODE_VIDEO_SOURCE)
#define VF_IS_SDR_FILTER(type) ((type) >= VF_NODE_SDR_FILTER)

enum VFPassDir {
	VF_PASS_HORIZ,
	VF_PASS_VERT
};

extern VideoFilterChain vfchain;

enum VFConnSocketType {
	VF_INPUT_SOCKET,
	VF_OUTPUT_SOCKET
};

struct VFConnSocket {
	VFConnSocketType type;
	VideoFilterNode *node;
	VFConnSocket *conn;
};

class VideoFilterChain {
private:
	std::vector<VideoFilterNode*> nodes;
	std::map<int, VideoFilterNode*> taps;

public:
	void clear();
	bool empty() const;
	int size() const;

	// add doesn't transfer ownership to the VideoFilterChain
	void add(VideoFilterNode *n);
	// remove doesn't free node memory
	void remove(VideoFilterNode *n);
	bool have_node(VideoFilterNode *n) const;
	VideoFilterNode *get_node(int idx) const;

	// simple connect, assuming each node has one input and one output
	void connect(VideoFilterNode *from, VideoFilterNode *to);
	// connect specific output to specific input (general case)
	void connect(VideoFilterNode *from, int from_idx, VideoFilterNode *to, int to_idx);
	// disconnect specific output of "n", regardless of where it's connected to
	void disconnect(VideoFilterNode *n, int out = -1);	// default (-1): all outputs
	// disconnect all outputs of "from" which are connected to any input of "to"
	void disconnect(VideoFilterNode *from, VideoFilterNode *to);
	// disconnect the output of "n" which is connected to "to" if such a connection exists
	void disconnect(VideoFilterNode *from, int from_idx, VideoFilterNode *to, int to_idx);

	void process();

	VideoFrame *get_frame(VideoFilterNode *n) const;
	VideoFrame *get_frame(int tap) const;

	bool set_tap(int tap, VideoFilterNode *n);
	VideoFilterNode *get_tap(int tap) const;

	void seek_video_sources(int frm);
};

class VideoFilterNode {
protected:
	virtual void prepare(int width, int height);

public:
	VFNodeType type;
	bool status;
	bool proc_pending;
	VideoFrame frm;

	VFConnSocket *inputs, *outputs;
	int num_inputs, num_outputs;

	VideoFilterNode();
	virtual ~VideoFilterNode();

	virtual VideoFilterNode *input_node(int idx = 0) const;
	virtual VideoFilterNode *output_node(int idx = 0) const;

	virtual int input_index(const VFConnSocket *s) const;
	virtual int output_index(const VFConnSocket *s) const;

	virtual void process() = 0;

	/* commit is called before accessing the frame, to make sure it's updated
	 * with the results of the last process call. by default does nothing.
	 */
	virtual void commit();
};

class VFGroup : public VideoFilterNode {
	// TODO
};

class VFSource : public VideoFilterNode {
protected:
	VFConnSocket out;

	virtual void prepare(int width, int height);

public:
	int frameno;

	VFSource();

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
	VFConnSocket in, out;
	bool own_sdr;
	bool commit_pending;

	virtual void prepare(int width, int height);

public:
	unsigned int sdr;
	unsigned int tex;
	int tex_width, tex_height;

	VFShader();
	virtual ~VFShader();

	virtual bool load_shader(const char *vsfile, const char *psfile);
	virtual void set_shader(unsigned int sdr);

	virtual void process();
	virtual void commit();
};

class VFSobel : public VFShader {
protected:
	virtual void prepare(int width, int height);

public:
	VFSobel();
};

class VFGaussBlurPass : public VFShader {
protected:
	virtual void prepare(int width, int height);

public:
	float sdev;
	int ksz;
	VFPassDir dir;

	VFGaussBlurPass();
};

class VFThreshold : public VFShader {
protected:
	virtual void prepare(int width, int height);

public:
	float thres;
	float smooth;

	VFThreshold();
};


#endif	/* VIDFILTER_H_ */
