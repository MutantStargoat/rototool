#include <map>
#include "opengl.h"
#include "dtx/drawtext.h"

static void draw_quad(int x, int y, int xsz, int ysz, unsigned int tex);
static unsigned int create_texture(int xsz, int ysz, const void *pixels);
static void update_texture(unsigned int tex, int xsz, int ysz, const void *pixels);

static float color[4];
static float scissor[4];

static std::map<const void*, unsigned int> texcache;


void utk_color(int r, int g, int b, int a)
{
	color[0] = r / 255.0;
	color[1] = g / 255.0;
	color[2] = b / 255.0;
	color[3] = 1.0;
}

void utk_clip(int x1, int y1, int x2, int y2)
{
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	scissor[0] = x1;
	scissor[1] = vp[3] - y2;
	scissor[2] = x2 - x1;
	scissor[3] = y2 - y1;
}

void utk_image(int x, int y, const void *pixels, int xsz, int ysz)
{
	unsigned int tex;
	std::map<const void*, unsigned int>::const_iterator iter;
	if((iter = texcache.find(pixels)) == texcache.end()) {
		tex = create_texture(xsz, ysz, pixels);
		texcache[pixels] = tex;
	} else {
		tex = iter->second;
		update_texture(tex, xsz, ysz, pixels);
	}

	glPushAttrib(GL_ENABLE_BIT);

	glEnable(GL_SCISSOR_TEST);
	glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);

	draw_quad(x, y, xsz, ysz, tex);

	glPopAttrib();
}

void utk_rect(int x1, int y1, int x2, int y2)
{
	glPushAttrib(GL_ENABLE_BIT);

	glEnable(GL_SCISSOR_TEST);
	glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);

	draw_quad(x1, y1, x2 - x1, y2 - y1, 0);

	glPopAttrib();
}

void utk_line(int x1, int y1, int x2, int y2, int width)
{
	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
	glLineWidth((float)width);

	glEnable(GL_SCISSOR_TEST);
	glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);

	glBegin(GL_LINES);
	glColor4f(color[0], color[1], color[2], color[3]);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();

	glPopAttrib();
}

void utk_text(int x, int y, const char *txt, int sz)
{
	int vp[4];

	glGetIntegerv(GL_VIEWPORT, vp);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(x, vp[3] - y + dtx_baseline(), 0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, vp[2], 0, vp[3], -1, 1);

	glPushAttrib(GL_ENABLE_BIT);

	glEnable(GL_SCISSOR_TEST);
	glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
	glColor3f(0, 0, 0);

	dtx_string(txt);

	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

int utk_text_spacing()
{
	return dtx_line_height();
}

int utk_text_width(const char *txt, int sz)
{
	return dtx_string_width(txt) + 1;
}

static void draw_quad(int x, int y, int xsz, int ysz, unsigned int tex)
{
	if(tex) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex);
	} else {
		glDisable(GL_TEXTURE_2D);
	}

	glBegin(GL_QUADS);
	glColor4f(color[0], color[1], color[2], color[3]);
	glTexCoord2f(0, 0);
	glVertex2i(x, y);
	glTexCoord2f(1, 0);
	glVertex2i(x + xsz, y);
	glTexCoord2f(1, 1);
	glVertex2i(x + xsz, y + ysz);
	glTexCoord2f(0, 1);
	glVertex2i(x, y + ysz);
	glEnd();
}

static unsigned int create_texture(int xsz, int ysz, const void *pixels)
{
	unsigned int tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, xsz, ysz, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	return tex;
}

static void update_texture(unsigned int tex, int xsz, int ysz, const void *pixels)
{
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, xsz, ysz, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}
