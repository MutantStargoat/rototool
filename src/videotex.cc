#include "videotex.h"
#include "opengl.h"

VideoTexture::VideoTexture()
{
	tex = 0;
	tex_width = tex_height = 0;
	tex_frame = -1;
	cur_frame = -1;
}

VideoTexture::~VideoTexture()
{
	close();
}

bool VideoTexture::open(const char *fname)
{
	close();
	if(vid.open(fname)) {
		cur_frame = 0;
		tex_frame = -1;
		return true;
	}
	return false;
}

void VideoTexture::close()
{
	vid.close();

	if(tex) {
		glDeleteTextures(1, &tex);
		tex = 0;
		tex_width = tex_height = 0;
	}
}

int VideoTexture::get_width() const
{
	return vid.GetWidth();
}

int VideoTexture::get_height() const
{
	return vid.GetHeight();
}

void VideoTexture::rewind()
{
	cur_frame = 0;
}

void VideoTexture::seek_frame(int frm)
{
	cur_frame = frm;
}

void VideoTexture::seek_frame_rel(int dfrm)
{
	cur_frame += dfrm;
}

int VideoTexture::get_cur_frame() const
{
	return cur_frame;
}

void VideoTexture::bind(int tunit)
{
	update_texture();

	glActiveTexture(GL_TEXTURE0 + tunit);
	glBindTexture(GL_TEXTURE_2D, tex);
	glActiveTexture(GL_TEXTURE0);
	bound_unit = tunit;
}

void VideoTexture::load_tex_scale()
{
	glLoadIdentity();
	glScalef((float)vid.GetWidth() / tex_width, (float)vid.GetHeight() / tex_height, 1);
}

void VideoTexture::update_texture()
{
	if(tex_frame == cur_frame) {
		return;
	}

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

	int xsz = vid.GetWidth();
	int ysz = vid.GetHeight();
	int new_tx = next_pow2(vid.GetWidth());
	int new_ty = next_pow2(vid.GetHeight());

	if(new_tx != tex_width || new_ty != tex_height) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, new_tx, new_ty, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		tex_width = new_tx;
		tex_height = new_ty;
	}

	unsigned char *pptr = nullptr;
	double dts;
	if (!vid.GetFrame(cur_frame, &pptr, &dts)) {
		printf("Error getting frame %d\n", cur_frame);
		return;
	}
	for(int i=0; i<ysz; i++) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, i, xsz, 1, GL_BGRA, GL_UNSIGNED_BYTE, pptr);
		pptr += vid.GetWidth() * vid.GetHeight() * 4;
	}

	tex_frame = cur_frame;

	glPopAttrib();
}
