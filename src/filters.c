#include <math.h>
#include <stddef.h>
#include "opengl.h"
#include "filters.h"
#include "sdr.h"

enum {
	SDR_SOBEL,
	SDR_HGAUS,
	SDR_VGAUS,

	NUM_SDR
};

static unsigned int sdr_vertex;
static unsigned int pixsdr[NUM_SDR];
static unsigned int sdrprog[NUM_SDR];
static unsigned int fbo;

static unsigned int tmptex;
static int tmptex_xsz, tmptex_ysz;

static const char *sdrfiles[] = {
	"sdr/sobel.p.glsl",
	"sdr/gausblur.p.glsl",
	"sdr/gausblur.p.glsl",
	0
};
static const char *sdrdefs[] = {
	0,
	"#define HORIZ\n",
	"#define VERT\n",
};


int init_filters()
{
	int i;

	glGenFramebuffersEXT(1, &fbo);

	glGenTextures(1, &tmptex);
	glBindTexture(GL_TEXTURE_2D, tmptex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(!(sdr_vertex = load_vertex_shader("sdr/filters.v.glsl"))) {
		return -1;
	}

	for(i=0; sdrfiles[i]; i++) {
		if(sdrdefs[i]) {
			add_shader_header(GL_FRAGMENT_SHADER, sdrdefs[i]);
		}
		if(!(pixsdr[i] = load_pixel_shader(sdrfiles[i]))) {
			return -1;
		}
		clear_shader_header(GL_FRAGMENT_SHADER);

		if(!(sdrprog[i] = create_program_link(sdr_vertex, pixsdr[i], NULL))) {
			return -1;
		}
	}

	return 0;
}

void cleanup_filters()
{
	int i;

	for(i=0; i<NUM_SDR; i++) {
		if(sdrprog[i]) {
			free_program(sdrprog[i]);
			sdrprog[i] = 0;
		}
		if(pixsdr[i]) {
			free_shader(pixsdr[i]);
			pixsdr[i] = 0;
		}
	}

	if(sdr_vertex) {
		free_shader(sdr_vertex);
		sdr_vertex = 0;
	}

	if(fbo) {
		glDeleteFramebuffersEXT(1, &fbo);
		fbo = 0;
	}
	if(tmptex) {
		glDeleteTextures(1, &tmptex);
		tmptex = 0;
	}
}

void apply_filter(unsigned int dest, unsigned int src, int xsz, int ysz, unsigned int sdr)
{
	int vp[4];
	int tex_xsz, tex_ysz;
	float maxu, maxv;

	glGetIntegerv(GL_VIEWPORT, vp);
	glViewport(0, 0, xsz, ysz);

	tex_xsz = next_pow2(xsz);
	tex_ysz = next_pow2(ysz);
	maxu = (float)xsz / (float)tex_xsz;
	maxv = (float)ysz / (float)tex_ysz;

	if(dest == tmptex) {
		if(tmptex_xsz != tex_xsz || tmptex_ysz != tex_ysz) {
			glBindTexture(GL_TEXTURE_2D, tmptex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_xsz, tex_ysz, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
			tmptex_xsz = tex_xsz;
			tmptex_ysz = tex_ysz;
		}
	}
	glBindTexture(GL_TEXTURE_2D, src);

	glBindFramebufferEXT(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dest, 0);

	glUseProgram(sdr);
	set_uniform_float2(sdr, "pixsz", maxu / xsz, maxv / ysz);

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

void edge_detect(unsigned int dest, unsigned int src, int xsz, int ysz)
{
	if(!sdrprog[SDR_SOBEL]) return;

	apply_filter(dest, src, xsz, ysz, sdrprog[SDR_SOBEL]);
}

void gauss_blur(unsigned int dest, unsigned int src, int xsz, int ysz, float stddev)
{
	int sz;

	if(!sdrprog[SDR_HGAUS] || !sdrprog[SDR_VGAUS]) {
		return;
	}

	sz = (int)ceil(stddev * 6.0f);

	set_uniform_float(sdrprog[SDR_HGAUS], "stddev", stddev);
	set_uniform_int(sdrprog[SDR_HGAUS], "ksz", sz);
	set_uniform_float(sdrprog[SDR_VGAUS], "stddev", stddev);
	set_uniform_int(sdrprog[SDR_VGAUS], "ksz", sz);

	apply_filter(tmptex, src, xsz, ysz, sdrprog[SDR_HGAUS]);
	apply_filter(dest, tmptex, xsz, ysz, sdrprog[SDR_VGAUS]);
}
