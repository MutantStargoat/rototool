#include <assert.h>
#include "opengl.h"
#include "vidfilter.h"
#include "sdr.h"

#if defined(WIN32) || defined(__WIN32__)
#include <malloc.h>
#else
#include <alloca.h>
#endif

static unsigned int fbo;
static unsigned int tmptex;
static int tmptex_width, tmptex_height;

static unsigned int sdr_vertex;

static bool init();

VFShader::VFShader()
{
	type = VF_NODE_SDR_FILTER;
	sdr = 0;
	tex = 0;
	tex_width = tex_height = 0;
	own_sdr = false;
	commit_pending = false;
}

VFShader::~VFShader()
{
	if(tex) {
		glDeleteTextures(1, &tex);
	}
	if(sdr && own_sdr) {
		free_program(sdr);
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
	own_sdr = true;
	return true;
}

void VFShader::set_shader(unsigned int sdr)
{
	this->sdr = sdr;
	own_sdr = true;
}

void VFShader::prepare(int width, int height)
{
	static bool done_init;
	if(!done_init) {
		init();
		done_init = true;
	}

	VideoFilterNode::prepare(width, height);

	int tx = next_pow2(width);
	int ty = next_pow2(height);

	if(!tex) {
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		tex_width = tex_height = 0;	// just to be sure we'll get to run glTexImage2D
	}

	if(tx != tex_width || ty != tex_height) {
		// if the size is wrong, and we re-create the texture
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tx, ty, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		tex_width = tx;
		tex_height = ty;
	}

	/* if the previous node wasn't a shader, then we must load the texture data, and
	 * also re-create the tmptex if it's not of the correct size
	 * and make sure to leave the appropriate source texture bound
	 */
	if(prev && prev->type != VF_NODE_SDR_FILTER) {
		glBindTexture(GL_TEXTURE_2D, tmptex);

		if(tx != tmptex_width || ty != tmptex_height) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tx, ty, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
			tmptex_width = tx;
			tmptex_height = ty;
		}

		prev->commit();
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, prev->frm.width, prev->frm.height, GL_BGRA,
				GL_UNSIGNED_BYTE, prev->frm.pixels);
	} else {
		glBindTexture(GL_TEXTURE_2D, ((VFShader*)prev)->tex);
	}
}

void VFShader::process(const VideoFrame *in)
{
	prepare(in->width, in->height);

	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	glViewport(0, 0, frm.width, frm.height);

	glBindFramebufferEXT(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

	float maxu = (float)frm.width / (float)tex_width;
	float maxv = (float)frm.height / (float)tex_height;

	glUseProgram(sdr);
	set_uniform_float2(sdr, "pixsz", maxu / frm.width, maxv / frm.height);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(-1, -1);
	glTexCoord2f(maxu, 0);
	glVertex2f(1, -1);
	glTexCoord2f(maxu, maxv);
	glVertex2f(1, 1);
	glTexCoord2f(0, maxv);
	glVertex2f(-1, 1);
	glEnd();

	glUseProgram(0);
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
	glViewport(vp[0], vp[1], vp[2], vp[3]);

	commit_pending = true;
}

void VFShader::commit()
{
	if(!commit_pending) return;

	glBindTexture(GL_TEXTURE_2D, tex);

	/* if the frame size is different thant the texture size (which will happen
	 * in case the frame size is not a power of two), we need to allocate a tmp
	 * buffer to get the pixels from GL, and copy it in the frame pixelbuffer one
	 * scanline at a time, since there is no glGetTexSubImage for some reason...
	 */
	if(frm.width < tex_width || frm.height < tex_height) {
		unsigned char *buf = (unsigned char*)alloca(tex_width * tex_height * 4);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, buf);

		unsigned char *dest = frm.pixels;
		for(int i=0; i<frm.height; i++) {
			memcpy(dest, buf, frm.width * 4);
			dest += frm.width * 4;
			buf += tex_width * 4;
		}
	} else {
		glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, frm.pixels);
	}
}

static bool init()
{
	glGenFramebuffersEXT(1, &fbo);

	glGenTextures(1, &tmptex);
	glBindTexture(GL_TEXTURE_2D, tmptex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return true;
}

// ---- VFSobel ----
static unsigned int sdr_sobel, prog_sobel;

void VFSobel::prepare(int width, int height)
{
	VFShader::prepare(width, height);

	if(!sdr_vertex) {
		if(!(sdr_vertex = load_vertex_shader("sdr/filters.v.glsl"))) {
			abort();
		}
	}
	if(!sdr_sobel) {
		if(!(sdr_sobel = load_pixel_shader("sdr/sobel.p.glsl"))) {
			abort();
		}
	}
	if(!prog_sobel) {
		if(!(prog_sobel = create_program_link(sdr_vertex, sdr_sobel, 0))) {
			abort();
		}
		set_shader(prog_sobel);
	}
}
