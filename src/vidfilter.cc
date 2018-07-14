#include <assert.h>
#include "vidfilter.h"

VideoFilterChain vfchain;

void VideoFilterChain::clear()
{
	nodes.clear();
	taps.clear();
}

bool VideoFilterChain::empty() const
{
	return nodes.empty();
}

void VideoFilterChain::add(VideoFilterNode *n)
{
	if(have_node(n)) return;
	nodes.push_back(n);
}

void VideoFilterChain::remove(VideoFilterNode *n)
{
	int idx = -1;
	int num = nodes.size();
	for(int i=0; i<num; i++) {
		if(nodes[i] == n) {
			idx = i;
			break;
		}
	}

	if(idx == -1) return;

	disconnect(n);
	for(int i=0; i<n->num_inputs(); i++) {
		if(n->input(i)) {
			disconnect(n->input(i), n);
		}
	}

	nodes.erase(nodes.begin() + idx);

	std::map<int, VideoFilterNode*>::iterator it = taps.begin();
	while(it != taps.end()) {
		if(it->second == n) {
			taps.erase(it);
			break;
		}
		it++;
	}
}

bool VideoFilterChain::have_node(VideoFilterNode *n) const
{
	int num = nodes.size();
	for(int i=0; i<num; i++) {
		if(nodes[i] == n) {
			return true;
		}
	}
	return false;
}

void VideoFilterChain::connect(VideoFilterNode *n, VideoFilterNode *to)
{
	connect(n, 0, to, 0);
}

void VideoFilterChain::connect(VideoFilterNode *n, int out, VideoFilterNode *to, int in)
{
	if(!n || !to) {
		fprintf(stderr, "VideoFilterChain::connect: null pointer\n");
		return;
	}
	if(out >= n->num_outputs() || in >= to->num_inputs()) {
		fprintf(stderr, "VideoFilterChain::connect: invalid input or output specified (%d -> %d)\n", out, in);
		return;
	}
	if(n->output(out) == to && to->input(in) == n) {
		return;
	}

	disconnect(n, out);

	n->set_output(to, out);
	to->set_input(n, in);
}

void VideoFilterChain::disconnect(VideoFilterNode *n, int out)
{
	if(out == -1) {
		// disconnect all
		for(int i=0; i<n->num_outputs(); i++) {
			disconnect(n, i);
		}
		return;
	}

	if(out >= n->num_outputs()) {
		fprintf(stderr, "VideoFilterChain::disconnect: no such output (%d)\n", out);
		return;
	}

	VideoFilterNode *to = n->output(out);
	n->set_output(0, out);

	if(to) {
		for(int i=0; i<to->num_inputs(); i++) {
			if(to->input(i) == n) {
				to->set_input(0, i);
			}
		}
	}
}

void VideoFilterChain::disconnect(VideoFilterNode *n, VideoFilterNode *to)
{
	int outidx = -1, inidx = -1;

	for(int i=0; i<n->num_outputs(); i++) {
		if(n->output(i) == to) {
			outidx = i;
			break;
		}
	}

	for(int i=0; i<to->num_inputs(); i++) {
		if(to->input(i) == n) {
			inidx = i;
			break;
		}
	}

	if(outidx != -1) {
		assert(inidx != -1);
		n->set_output(0, outidx);
		to->set_input(0, inidx);
	} else {
		assert(inidx == -1);
	}
}

void VideoFilterChain::process()
{
	int num = nodes.size();
	for(int i=0; i<num; i++) {
		nodes[i]->proc_pending = true;
	}
	for(int i=0; i<num; i++) {
		nodes[i]->process();
	}
}

VideoFrame *VideoFilterChain::get_frame(VideoFilterNode *n) const
{
	n->commit();
	return &n->frm;
}

VideoFrame *VideoFilterChain::get_frame(int tap) const
{
	VideoFilterNode *n = get_tap(tap);
	if(n) {
		n->commit();
		return &n->frm;
	}
	return 0;
}

bool VideoFilterChain::set_tap(int tap, VideoFilterNode *n)
{
	if(!have_node(n)) return false;

	taps[tap] = n;
	return true;
}

VideoFilterNode *VideoFilterChain::get_tap(int tap) const
{
	std::map<int, VideoFilterNode*>::const_iterator it = taps.find(tap);
	return it == taps.end() ? 0 : it->second;
}

void VideoFilterChain::seek_video_sources(int frm)
{
	int num = nodes.size();
	for(int i=0; i<num; i++) {
		VideoFilterNode *n = nodes[i];
		if(n->type == VF_NODE_SOURCE) {
			((VFSource*)n)->set_frame_number(frm);
		}
	}
}

// ---- VideoFilterNode ----

VideoFilterNode::VideoFilterNode()
{
	type = VF_NODE_UNKNOWN;
	frm.width = frm.height = 0;
	frm.pixels = 0;
	status = true;
	proc_pending = false;
	num_in = num_out = 0;
}

VideoFilterNode::~VideoFilterNode()
{
	delete [] frm.pixels;
}

void VideoFilterNode::set_input(VideoFilterNode *n, int idx)
{
	assert(num_in == 0);
}

VideoFilterNode *VideoFilterNode::input(int idx) const
{
	assert(num_in == 0);
	return 0;
}

int VideoFilterNode::num_inputs() const
{
	return num_in;
}

void VideoFilterNode::set_output(VideoFilterNode *n, int idx)
{
	assert(num_out == 0);
}

VideoFilterNode *VideoFilterNode::output(int idx) const
{
	assert(num_out == 0);
	return 0;
}

int VideoFilterNode::num_outputs() const
{
	return num_out;
}


void VideoFilterNode::prepare(int width, int height)
{
	if(!frm.pixels || frm.width != width || frm.height != height) {
		delete [] frm.pixels;
		frm.width = width;
		frm.height = height;
		frm.pixels = new unsigned char[width * height * 4];
	}
}

void VideoFilterNode::commit()
{
}

// ---- VFSource ----
VFSource::VFSource()
{
	type = VF_NODE_SOURCE;
	frameno = 0;
	set_size(128, 128);
	num_out = 1;
	out = 0;
}

void VFSource::set_output(VideoFilterNode *n, int idx)
{
	if(idx != 0) {
		fprintf(stderr, "VFSource: trying to connect invalid output: %d\n", idx);
		return;
	}
	out = n;
}

VideoFilterNode *VFSource::output(int idx) const
{
	if(idx != 0) {
		fprintf(stderr, "VFSource: trying to access invalid output: %d\n", idx);
		return 0;
	}
	return out;
}

void VFSource::set_size(int w, int h)
{
	if(w != frm.width && h != frm.height) {
		delete [] frm.pixels;
		frm.pixels = 0;
		frm.width = w;
		frm.height = h;
	}
}

void VFSource::set_frame_number(int n)
{
	frameno = n;
}

void VFSource::prepare(int width, int height)
{
	if(!frm.pixels && width && height) {
		VideoFilterNode::prepare(width, height);

		unsigned char *pptr = frm.pixels;
		for (int i=0; i<height; i++) {
			for (int j=0; j<width; j++) {
				int x = i ^ j;
				*pptr++ = x << 3;
				*pptr++ = x << 2;
				*pptr++ = x << 1;
				*pptr++ = 255;
			}
		}
	}
}

void VFSource::process()
{
	if(!proc_pending) return;
	proc_pending = false;

	prepare(frm.width, frm.height);
	status = frm.width > 0 && frm.height > 0 && frm.pixels;
}

// ---- VFVideoSource ----

VFVideoSource::VFVideoSource()
{
	vid = 0;
}

void VFVideoSource::set_source(Video *v)
{
	vid = v;
}

void VFVideoSource::process()
{
	if(!proc_pending) return;

	status = true;

	if(!vid) {
		VFSource::process();
		status = false;
		return;
	}

	int width = vid->GetWidth();
	int height = vid->GetHeight();
	set_size(width, height);
	VFSource::prepare(width, height);

	unsigned char *pptr = 0;
	if(!vid->GetFrame(frameno, &pptr)) {
		VFSource::process();
		status = false;
		return;
	}

	memcpy(frm.pixels, pptr, frm.width * frm.height * 4);
	proc_pending = false;
}
