#include "videotex.h"
#include "opengl.h"
#include "vidfilter.h"

VideoTexture::VideoTexture(Video &v) : vid(v)
{
	tex = 0;
	tex_width = tex_height = 0;
	tex_frame = -1;
}

VideoTexture::~VideoTexture()
{
	if (tex) {
		glDeleteTextures(1, &tex);
		tex = 0;
		tex_width = tex_height = 0;
	}
}

int VideoTexture::get_width() const
{
	VideoFrame *frm = vfchain.get_frame();
	if(!frm) {
		return vid.GetWidth();
	}
	return frm->width;
}

int VideoTexture::get_height() const
{
	VideoFrame *frm = vfchain.get_frame();
	if(!frm) {
		return vid.GetHeight();
	}
	return frm->height;
}

int VideoTexture::get_tex_width() const
{
	return tex_width;
}

int VideoTexture::get_tex_height() const
{
	return tex_height;
}

void VideoTexture::bind(int video_frame, int tunit)
{
	update_texture(video_frame);

	glActiveTexture(GL_TEXTURE0 + tunit);
	glBindTexture(GL_TEXTURE_2D, tex);
	glActiveTexture(GL_TEXTURE0);
	bound_unit = tunit;
}

void VideoTexture::load_tex_scale()
{
	glLoadIdentity();
	if(tex_width != 0 && tex_height != 0) {
		glScalef((float)get_width() / (float)tex_width, (float)get_height() / (float)tex_height, 1);
	}
}

void VideoTexture::update_texture(int video_frame)
{
	if(tex_frame == video_frame) {
		return;
	}

	// update frame
	vfchain.process();

	glPushAttrib(GL_TEXTURE_BIT);

	if(!tex) {
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	} else {
		glBindTexture(GL_TEXTURE_2D, tex);
	}

	int xsz = get_width();
	int ysz = get_height();
	int new_tx = next_pow2(xsz);
	int new_ty = next_pow2(ysz);

	if(new_tx != tex_width || new_ty != tex_height) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, new_tx, new_ty, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		tex_width = new_tx;
		tex_height = new_ty;
	}

	unsigned char *pptr = nullptr;
	VideoFrame *frm = vfchain.get_frame(VF_COLOR_TAP);
	if(!frm) {
		if (!vid.GetFrame(video_frame, &pptr)) {
			printf("Error getting frame %d\n", video_frame);
			return;
		}
	} else {
		pptr = frm->pixels;
	}

	if(pptr) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, xsz, ysz, GL_BGRA, GL_UNSIGNED_BYTE, pptr);
	}

	tex_frame = video_frame;

	glPopAttrib();
}

unsigned int VideoTexture::get_texture() const
{
	return tex;
}

void VideoTexture::invalidate()
{
	tex_frame = -1;
}
