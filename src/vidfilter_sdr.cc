#include <assert.h>
#include "opengl.h"
#include "vidfilter.h"
#include "sdr.h"

static void get_tex_image(unsigned int tex, int tex_width, int tex_height, VideoFrame *frm);

static unsigned int fbo;
static unsigned int tmptex, scaletex;
static int tmptex_width, tmptex_height;
static int scaletex_width, scaletex_height;

static unsigned int sdr_vertex;
static unsigned int sdr_sobel, prog_sobel;
static unsigned int sdr_gauss[2], prog_gauss[2];
static unsigned int sdr_thres, prog_thres;


static bool init()
{
	glGenFramebuffersEXT(1, &fbo);

	unsigned int tex[2];
	glGenTextures(2, tex);
	for(int i=0; i<2; i++) {
		glBindTexture(GL_TEXTURE_2D, tex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	tmptex = tex[0];
	scaletex = tex[1];

	return true;
}

static void load_shaders()
{
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
	}

	for(int i=0; i<2; i++) {
		if(!sdr_gauss[i]) {
			add_shader_header(GL_FRAGMENT_SHADER, i == 0 ? "#define HORIZ" : "#define VERT");
			if(!(sdr_gauss[i] = load_pixel_shader("sdr/gausblur.p.glsl"))) {
				abort();
			}
			clear_shader_header(GL_FRAGMENT_SHADER);
		}
		if(!prog_gauss[i]) {
			if(!(prog_gauss[i] = create_program_link(sdr_vertex, sdr_gauss[i], 0))) {
				abort();
			}
		}
	}

	if(!sdr_thres) {
		if(!(sdr_thres = load_pixel_shader("sdr/thres.p.glsl"))) {
			abort();
		}
	}
	if(!prog_thres) {
		if(!(prog_thres = create_program_link(sdr_vertex, sdr_thres, 0))) {
			abort();
		}
	}
}

void scale_video_frame(VideoFrame *dest, const VideoFrame *src, float scale)
{
	dest->width = src->width * scale;
	dest->height = src->height * scale;
	dest->pixels = new unsigned char[dest->width * dest->height * 4];

	// prepare framebuffer texture
	int tx = next_pow2(dest->width);
	int ty = next_pow2(dest->height);

	glBindTexture(GL_TEXTURE_2D, tmptex);
	if(tmptex_width != tx || tmptex_height != ty) {
		tmptex_width = tx;
		tmptex_height = ty;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tx, ty, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tmptex, 0);

	// load src pixels into scaletex
	tx = next_pow2(src->width);
	ty = next_pow2(src->height);

	glBindTexture(GL_TEXTURE_2D, scaletex);
	if(scaletex_width != tx || scaletex_height != ty) {
		scaletex_width = tx;
		scaletex_height = ty;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tx, ty, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	}
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, src->width, src->height, GL_BGRA,
			GL_UNSIGNED_BYTE, src->pixels);

	float maxu = (float)src->width / (float)tx;
	float maxv = (float)src->height / (float)ty;

	glPushAttrib(GL_ENABLE_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	glViewport(0, 0, dest->width, dest->height);

	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	glTexCoord2f(0, 0);
	glVertex2f(-1, -1);
	glTexCoord2f(maxu, 0);
	glVertex2f(1, -1);
	glTexCoord2f(maxu, maxv);
	glVertex2f(1, 1);
	glTexCoord2f(0, maxv);
	glVertex2f(-1, 1);
	glEnd();

	glViewport(vp[0], vp[1], vp[2], vp[3]);

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	get_tex_image(tmptex, tmptex_width, tmptex_height, dest);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}


VFShader::VFShader()
{
	type = VF_NODE_SDR_FILTER;
	sdr = 0;
	tex = 0;
	tex_width = tex_height = 0;
	own_sdr = false;
	commit_pending = false;

	num_inputs = num_outputs = 1;
	inputs = &in;
	outputs = &out;

	in.type = VF_INPUT_SOCKET;
	out.type = VF_OUTPUT_SOCKET;
	in.node = out.node = this;
	in.conn = out.conn = 0;
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
	own_sdr = false;
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
	VideoFilterNode *in = input_node();
	if(!VF_IS_SDR_FILTER(in->type)) {
		glBindTexture(GL_TEXTURE_2D, tmptex);

		if(tx != tmptex_width || ty != tmptex_height) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tx, ty, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
			tmptex_width = tx;
			tmptex_height = ty;
		}

		in->commit();
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, in->frm.width, in->frm.height, GL_BGRA,
				GL_UNSIGNED_BYTE, in->frm.pixels);
	} else {
		glBindTexture(GL_TEXTURE_2D, ((VFShader*)in)->tex);
	}
}

void VFShader::process()
{
	VideoFilterNode *in = input_node();

	if(!proc_pending || !in) return;
	proc_pending = false;

	in->process();

	prepare(in->frm.width, in->frm.height);

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

static void get_tex_image(unsigned int tex, int tex_width, int tex_height, VideoFrame *frm)
{
	glBindTexture(GL_TEXTURE_2D, tex);

	/* if the frame size is different thant the texture size (which will happen
	 * in case the frame size is not a power of two), we need to allocate a tmp
	 * buffer to get the pixels from GL, and copy it in the frame pixelbuffer one
	 * scanline at a time, since there is no glGetTexSubImage for some reason...
	 */
	if(frm->width < tex_width || frm->height < tex_height) {
		unsigned char *buf = new unsigned char[tex_width * tex_height * 4];
		unsigned char *bptr = buf;
		glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, buf);

		unsigned char *dest = frm->pixels;
		for(int i=0; i<frm->height; i++) {
			memcpy(dest, bptr, frm->width * 4);
			dest += frm->width * 4;
			bptr += tex_width * 4;
		}
		delete [] buf;
	} else {
		glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, frm->pixels);
	}
}

void VFShader::commit()
{
	if(!commit_pending) return;

	get_tex_image(tex, tex_width, tex_height, &frm);

	commit_pending = false;
}

// ---- VFSobel ----
VFSobel::VFSobel()
{
	type = VF_NODE_SOBEL;
}

void VFSobel::prepare(int width, int height)
{
	load_shaders();
	set_shader(prog_sobel);

	VFShader::prepare(width, height);
}

// ---- VFGaussBlurPass ----
VFGaussBlurPass::VFGaussBlurPass()
{
	type = VF_NODE_GAUSSBLUR_PASS;
	sdev = 5.0;
	ksz = 5;
	dir = VF_PASS_HORIZ;
}

void VFGaussBlurPass::prepare(int width, int height)
{
	load_shaders();
	set_shader(prog_gauss[dir]);
	set_uniform_float(sdr, "stddev", sdev);
	set_uniform_int(sdr, "ksz", ksz);
	VFShader::prepare(width, height);
}

// ---- VFThreshold ----
VFThreshold::VFThreshold()
{
	thres = 0.5f;
	smooth = 0.01f;
	inverse = false;
}

void VFThreshold::prepare(int width, int height)
{
	load_shaders();
	set_shader(prog_thres);
	set_uniform_float(sdr, "thres", thres);
	set_uniform_float(sdr, "smooth", smooth);
	set_uniform_float(sdr, "invsign", inverse ? -1.0f : 1.0f);
	VFShader::prepare(width, height);
}
