#include <stdio.h>
#include <string.h>
#include <errno.h>
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

int VideoFilterChain::size() const
{
	return (int)nodes.size();
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
	for(int i=0; i<n->num_inputs; i++) {
		VideoFilterNode *in = n->input_node(i);
		if(in) {
			disconnect(in, n);
		}
	}

	nodes.erase(nodes.begin() + idx);

	std::map<int, VideoFilterNode*>::iterator it = taps.begin();
	while(it != taps.end()) {
		if(it->second == n) {
			taps.erase(it);
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

VideoFilterNode *VideoFilterChain::get_node(int idx) const
{
	return nodes[idx];
}

void VideoFilterChain::connect(VideoFilterNode *from, VideoFilterNode *to)
{
	connect(from, 0, to, 0);
}

void VideoFilterChain::connect(VideoFilterNode *from, int from_idx, VideoFilterNode *to, int to_idx)
{
	if(!from || !to) {
		fprintf(stderr, "VideoFilterChain::connect: null pointer\n");
		return;
	}
	if(from_idx >= from->num_outputs || to_idx >= to->num_inputs) {
		fprintf(stderr, "VideoFilterChain::connect: invalid input/output (%d -> %d)\n",
				from_idx, to_idx);
		return;
	}
	assert(from_idx >= 0);
	assert(to_idx >= 0);

	disconnect(from, from_idx);

	// if there's another connection coming to the destination socket, disconnect it first
	VFConnSocket *other_sock = to->inputs[to_idx].conn;
	if(other_sock) {
		VideoFilterNode *other = other_sock->node;
		disconnect(other, other->output_index(other_sock));
	}

	from->outputs[from_idx].conn = to->inputs + to_idx;
	to->inputs[to_idx].conn = from->outputs + from_idx;
}

void VideoFilterChain::disconnect(VideoFilterNode *n, int out)
{
	if(out == -1) {
		// disconnect all
		for(int i=0; i<n->num_outputs; i++) {
			disconnect(n, i);
		}
		return;
	}

	if(out >= n->num_outputs) {
		fprintf(stderr, "VideoFilterChain::disconnect: no such output (%d)\n", out);
		return;
	}

	VideoFilterNode *to = n->output_node(out);
	if(!to) {
		return;
	}
	disconnect(n, out, to, to->input_index(n->outputs[out].conn));
}

void VideoFilterChain::disconnect(VideoFilterNode *from, VideoFilterNode *to)
{
	for(int i=0; i<from->num_outputs; i++) {
		if(from->output_node(i) == to) {
			disconnect(from, i, to, to->input_index(from->outputs[i].conn));
		}
	}
}

void VideoFilterChain::disconnect(VideoFilterNode *from, int from_idx, VideoFilterNode *to, int to_idx)
{
	from->outputs[from_idx].conn = 0;
	to->inputs[to_idx].conn = 0;
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
		if(VF_IS_SOURCE(n->type)) {
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
	inputs = outputs = 0;
	num_inputs = num_outputs = 0;
}

VideoFilterNode::~VideoFilterNode()
{
	delete [] frm.pixels;
}

VideoFilterNode *VideoFilterNode::input_node(int idx) const
{
	if(idx >= num_inputs) return 0;
	if(!inputs[idx].conn) return 0;
	return inputs[idx].conn->node;
}

VideoFilterNode *VideoFilterNode::output_node(int idx) const
{
	if(idx >= num_outputs) return 0;
	if(!outputs[idx].conn) return 0;
	return outputs[idx].conn->node;
}

int VideoFilterNode::input_index(const VFConnSocket *s) const
{
	int idx = s - inputs;
	return idx < 0 || idx >= num_inputs ? -1 : idx;
}

int VideoFilterNode::output_index(const VFConnSocket *s) const
{
	int idx = s - outputs;
	return idx < 0 || idx >= num_outputs ? -1 : idx;
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

	num_outputs = 1;
	outputs = &out;

	out.type = VF_OUTPUT_SOCKET;
	out.node = this;
	out.conn = 0;
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
	type = VF_NODE_VIDEO_SOURCE;
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

bool dump_video_frame(VideoFrame *frm, const char *fname)
{
	FILE *fp = fopen(fname, "wb");
	if(!fp) {
		fprintf(stderr, "dump_video_frame(%s): failed: %s\n", fname, strerror(errno));
		return false;
	}

	unsigned char *pptr = frm->pixels;
	fprintf(fp, "P6\n%d %d\n255\n", frm->width, frm->height);
	for(int i=0; i<frm->width * frm->height; i++) {
		fputc(pptr[2], fp);
		fputc(pptr[1], fp);
		fputc(pptr[0], fp);
		pptr += 4;
	}
	fclose(fp);
	return true;
}
