#include <assert.h>
#include "vidfilter.h"

VideoFilterChain vfchain;

VideoFilterChain::VideoFilterChain()
{
	color_tap = VF_BACK;
	vflist = vftail = 0;
	vflist_size = 0;
}

void VideoFilterChain::clear()
{
	while(vflist) {
		VideoFilterNode *n = vflist;
		vflist = vflist->next;
		delete n;
	}
	vflist = vftail = 0;
	vflist_size = 0;
}

bool VideoFilterChain::empty() const
{
	return vflist == 0;
}

int VideoFilterChain::size() const
{
	return vflist_size;
}

void VideoFilterChain::insert_node(VideoFilterNode *n, int at)
{
	if(!vflist) {
		vflist = vftail = n;
		n->next = n->prev = 0;
		vflist_size++;
		return;
	}

	if(at > vflist_size) {
		at = VF_BACK;
	}

	if(at == VF_FRONT || at == 0) {
		n->prev = 0;
		n->next = vflist;
		vflist->prev = n;
		vflist = n;

	} else if(at == VF_BACK) {
		n->next = 0;
		n->prev = vftail;
		vftail->next = n;
		vftail = n;

	} else {
		//nodes.insert(nodes.begin() + at, n);
		VideoFilterNode *it = vflist;
		for(int i=0; i<at - 1; i++) {
			it = it->next;
		}

		n->next = it->next;
		n->prev = it;
		if(it == vftail) vftail = n;
	}
	vflist_size++;
}

void VideoFilterChain::remove_node(VideoFilterNode *n)
{
	if(n->prev) {
		n->prev->next = n->next;
	}
	if(n->next) {
		n->next->prev = n->prev;
	}
	if(vflist == n) vflist = n->next;
	if(vftail == n) vftail = n->prev;

	n->next = n->prev = 0;

	if(--vflist_size < 0) {
		fprintf(stderr, "VideoFilterNode::remove_node: BUG removing causes vflist_size to become: %d\n", vflist_size);
		vflist_size = 0;
	}
}

void VideoFilterChain::delete_node(VideoFilterNode *n)
{
	remove_node(n);
	delete n;
}

void VideoFilterChain::process()
{
	VideoFrame *prev_frame = 0;
	VideoFilterNode *n = vflist;
	while(n) {
		n->process(prev_frame);
		prev_frame = &n->frm;
		n = n->next;
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
	if(at >= vflist_size) {
		at = VF_BACK;
	}

	VideoFilterNode *n;

	if(at == VF_FRONT) {
		n = vflist;
	} else if(at == VF_BACK) {
		n = vftail;
	} else {
		n = vflist;
		for(int i=0; i<at; i++) {
			n = n->next;
		}
	}

	n->commit();
	return &n->frm;
}

void VideoFilterChain::set_color_tap(int at)
{
	if(at >= size() - 1) {
		at = VF_BACK;
	}
	color_tap = at;
}

int VideoFilterChain::get_color_tap() const
{
	return color_tap;
}

void VideoFilterChain::seek_video_source(int frm)
{
	VideoFilterNode *n = vflist;
	while(n) {
		if(n->type == VF_NODE_SOURCE) {
			((VFSource*)n)->set_frame_number(frm);
		}
		n = n->next;
	}
}

// ---- VideoFilterNode ----

VideoFilterNode::VideoFilterNode()
{
	type = VF_NODE_UNKNOWN;
	frm.width = frm.height = 0;
	frm.pixels = 0;
	status = true;

	prev = next = 0;
}

VideoFilterNode::~VideoFilterNode()
{
	delete [] frm.pixels;
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

void VFSource::process(const VideoFrame *in)
{
	prepare(frm.width, frm.height);
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
		return;
	}

	int width = vid->GetWidth();
	int height = vid->GetHeight();
	set_size(width, height);
	VFSource::prepare(width, height);

	unsigned char *pptr = 0;
	if(!vid->GetFrame(frameno, &pptr)) {
		VFSource::process(in);
		status = false;
		return;
	}

	memcpy(frm.pixels, pptr, frm.width * frm.height * 4);
}
