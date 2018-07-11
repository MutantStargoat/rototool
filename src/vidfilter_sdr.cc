#include <assert.h>
#include "opengl.h"
#include "vidfilter.h"
#include "sdr.h"

static unsigned int fbo;
static unsigned int tex[2];
static int tex_width, tex_height;
static int srcidx;

static unsigned int sdr_vertex;

static bool init();

VFShader::VFShader()
{
	type = VF_NODE_SDR_FILTER;
	sdr = 0;
	own_sdr = false;
}

VFShader::~VFShader()
{
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

void VFShader::prepare()
{
	static bool done_init;
	if(!done_init) {
		init();
		done_init = true;
	}

	bool must_load = false;
	int tx = next_pow2(frm.width);
	int ty = next_pow2(frm.height);

	if(tx != tex_width || ty != tex_height) {
		// if the size is wrong, and we re-create the texture, then force a reload
		must_load = true;
		for(int i=0; i<2; i++) {
			glBindTexture(GL_TEXTURE_2D, tex[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tx, ty, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		}
	}

	// if the previous node wasn't a shader, then we must load the texture data
	if(prev && prev->type != VF_NODE_SDR_FILTER) {
		must_load = true;
	}

	// if there is no previous node, then cancel loading, there's nothing to load
	if(!prev) {
		must_load = false;
	}

	if(must_load) {
		assert(prev);
		srcidx = 0;
		glBindTexture(GL_TEXTURE_2D, tex[0]);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, prev->frm.width, prev->frm.height, GL_RGB,
				GL_UNSIGNED_BYTE, prev->frm.pixels);
	} else {
		srcidx = (srcidx + 1) & 1;
	}
}

void VFShader::process(const VideoFrame *in)
{
	prepare();

	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	glViewport(0, 0, frm.width, frm.height);

	int dstidx = (srcidx + 1) & 1;
	glBindFramebufferEXT(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			tex[dstidx], 0);

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
}

static bool init()
{
	glGenFramebuffersEXT(1, &fbo);

	glGenTextures(2, tex);
	for(int i=0; i<2; i++) {
		glBindTexture(GL_TEXTURE_2D, tex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	return true;
}

// ---- VFSobel ----
static unsigned int sdr_sobel, prog_sobel;

void VFSobel::prepare()
{
	VFShader::prepare();

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
		if(!(prog_sobel = create_program_link(sdr_vertex, sdr_sobel))) {
			abort();
		}
		set_shader(prog_sobel);
	}
}
