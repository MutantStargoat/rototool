#include <assert.h>
#include "opengl.h"
#include "vidfilter.h"
#include "filters.h"	/* shader filters, rename at some point or merge here */
#include "sdr.h"

VideoFilterChain vfchain;

VideoFilterChain::VideoFilterChain()
{
	color_tap = VF_BACK;
}

void VideoFilterChain::clear()
{
	int num = nodes.size();
	for(int i=0; i<num; i++) {
		delete nodes[i];
	}
	nodes.clear();
}

bool VideoFilterChain::empty() const
{
	return nodes.empty();
}

int VideoFilterChain::size() const
{
	return (int)nodes.size();
}

void VideoFilterChain::insert_node(VideoFilterNode *n, int at)
{
	if(at == VF_FRONT) {
		nodes.insert(nodes.begin(), n);
	} else if(at == VF_BACK) {
		nodes.push_back(n);
	} else {
		nodes.insert(nodes.begin() + at, n);
	}
}

void VideoFilterChain::remove_node(VideoFilterNode *n)
{
	int num = nodes.size();
	for(int i=0; i<num; i++) {
		if(nodes[i] == n) {
			nodes.erase(nodes.begin() + i);
			return;
		}
	}
}

void VideoFilterChain::process()
{
	VideoFrame *prev_frame = 0;
	int num = nodes.size();
	for(int i=0; i<num; i++) {
		nodes[i]->process(prev_frame);
		prev_frame = &nodes[i]->frm;
	}
}

VideoFrame *VideoFilterChain::get_frame(int at) const
{
	if(empty()) {
		return 0;
	}

	if(at == VF_COLOR_TAP) {
		at = color_tap;
	}

	if(at == VF_FRONT) {
		at = 0;
	} else if(at == VF_BACK) {
		at = nodes.size() - 1;
	}
	return &nodes[at]->frm;
}

void VideoFilterChain::set_color_tap(int at)
{
	if(at >= size()) {
		at = VF_BACK;
	}
	color_tap = at;
}

// ---- VideoFilterNode ----

VideoFilterNode::VideoFilterNode()
{
	frm.width = frm.height = 0;
	frm.pixels = 0;
	status = true;
}

VideoFilterNode::~VideoFilterNode()
{
	delete [] frm.pixels;
}

// ---- VFSource ----
VFSource::VFSource()
{
	frameno = 0;
	set_size(128, 128);
}

void VFSource::set_size(int w, int h)
{
	if(w == frm.width && h == frm.height && frm.pixels) {
		return;
	}

	delete [] frm.pixels;
	frm.pixels = new unsigned char[w * h * 4];
	frm.width = w;
	frm.height = h;

	unsigned char *pptr = frm.pixels;
	for (int i=0; i<h; i++) {
		for (int j=0; j<w; j++) {
			int x = i ^ j;
			*pptr++ = x << 3;
			*pptr++ = x << 2;
			*pptr++ = x << 1;
			*pptr++ = 255;
		}
	}
}

void VFSource::set_frame_number(int n)
{
	frameno = n;
}

void VFSource::process(const VideoFrame *in)
{
	status = frm.width > 0 && frm.height > 0 && frm.pixels && !in;
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

void VFVideoSource::process(const VideoFrame *in)
{
	status = true;

	if(!vid) {
		VFSource::process(in);
		status = false;
	}

	int vid_width = vid->GetWidth();
	int vid_height = vid->GetHeight();

	if(frm.width != vid_width || frm.height != vid_height) {
		set_size(vid_width, vid_height);
	}

	unsigned char *pptr = 0;
	if(!vid->GetFrame(frameno, &pptr)) {
		VFSource::process(in);
		status = false;
	}

	memcpy(frm.pixels, pptr, frm.width * frm.height * 4);
}

// ---- VFShader ----
VFShader::VFShader()
{
	sdr = 0;
	src_tex = dst_tex = 0;
}

VFShader::~VFShader()
{
	if(sdr) {
		free_program(sdr);
	}
	if(src_tex) {
		glDeleteTextures(1, &src_tex);
	}
	if(dst_tex) {
		glDeleteTextures(1, &dst_tex);
	}
}

bool VFShader::load_shader(const char *vsfile, const char *psfile)
{
	if(sdr) {
		free_program(sdr);
	}
	if(!(sdr = create_program_load(vsfile, psfile))) {
		return false;
	}
	return true;
}

void VFShader::process(const VideoFrame *in)
{
	// TODO: continue
}
