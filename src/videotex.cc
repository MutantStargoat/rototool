#include "videotex.h"
#include "opengl.h"

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
	return vid.GetWidth();
}

int VideoTexture::get_height() const
{
	return vid.GetHeight();
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
	glScalef((float)vid.GetWidth() / tex_width, (float)vid.GetHeight() / tex_height, 1);
}

void VideoTexture::update_texture(int video_frame)
{
	if(tex_frame == video_frame) {
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
	if (!vid.GetFrame(video_frame, &pptr)) {
		printf("Error getting frame %d\n", video_frame);
		return;
	}
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, xsz, ysz, GL_BGRA, GL_UNSIGNED_BYTE, pptr);

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
