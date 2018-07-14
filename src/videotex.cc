#include "app.h"
#include "videotex.h"
#include "opengl.h"
#include "vidfilter.h"

VideoTexture::VideoTexture()
{
	tex = 0;
	tex_width = tex_height = 0;
	tex_frame = -1;
	vftap = VF_COLOR_TAP;
}

VideoTexture::~VideoTexture()
{
	if(tex) {
		glDeleteTextures(1, &tex);
		tex = 0;
		tex_width = tex_height = 0;
	}
}

void VideoTexture::use_tap(int tap)
{
	if(tap != vftap) {
		vftap = tap;
		invalidate();
	}
}

int VideoTexture::current_tap() const
{
	return vftap;
}

int VideoTexture::get_width() const
{
	VideoFrame *frm = vfchain.get_frame(VF_COLOR_TAP);
	return frm ? frm->width : 0;
}

int VideoTexture::get_height() const
{
	VideoFrame *frm = vfchain.get_frame(VF_COLOR_TAP);
	return frm ? frm->height : 0;
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
	}

	int xsz = get_width();
	int ysz = get_height();
	int new_tx = next_pow2(xsz);
	int new_ty = next_pow2(ysz);

	if(new_tx != tex_width || new_ty != tex_height) {
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, new_tx, new_ty, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		tex_width = new_tx;
		tex_height = new_ty;
	}

	VideoFrame *frm = vfchain.get_frame(vftap);
	if(frm) {
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, xsz, ysz, GL_BGRA, GL_UNSIGNED_BYTE, frm->pixels);
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
