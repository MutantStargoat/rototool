#include <stdio.h>
#include <stdlib.h>
#include "opengl.h"


static void GLAPIENTRY gldebug_logger(GLenum src, GLenum type, GLuint id, GLenum severity,
		GLsizei len, const char *msg, const void *cls);

static const char *gldebug_srcstr(unsigned int src);
static const char *gldebug_typestr(unsigned int type);

struct GLCaps glcaps;

int init_opengl(void)
{
	glewInit();

	glcaps.debug = GLEW_ARB_debug_output;

#ifndef NDEBUG
	if(glcaps.debug) {
		printf("Installing OpenGL debug callback\n");
		glDebugMessageCallbackARB(gldebug_logger, 0);
	}
#endif

	return 0;
}

int next_pow2(int x)
{
	x--;
	x = (x >> 1) | x;
	x = (x >> 2) | x;
	x = (x >> 4) | x;
	x = (x >> 8) | x;
	x = (x >> 16) | x;
	return x + 1;
}

void dump_gl_texture(unsigned int tex, const char *fname)
{
	FILE *fp;
	int i, width, height;
	unsigned char *pixels, *pptr;

	glBindTexture(GL_TEXTURE_2D, tex);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	if(!(pixels = malloc(width * height * 4))) {
		return;
	}
	pptr = pixels;
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	if(!(fp = fopen(fname, "wb"))) {
		free(pixels);
		return;
	}
	fprintf(fp, "P6\n%d %d\n255\n", width, height);
	for(i=0; i<width * height; i++) {
		fputc(*pptr++, fp);
		fputc(*pptr++, fp);
		fputc(*pptr++, fp);
		pptr++;
	}
	fclose(fp);
	free(pixels);
}


static void GLAPIENTRY gldebug_logger(GLenum src, GLenum type, GLuint id, GLenum severity,
		GLsizei len, const char *msg, const void *cls)
{
	static const char *fmt = "[GLDEBUG] (%s) %s: %s\n";
	switch(type) {
	case GL_DEBUG_TYPE_ERROR:
		fprintf(stderr, fmt, gldebug_srcstr(src), gldebug_typestr(type), msg);
		break;

	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
	case GL_DEBUG_TYPE_PORTABILITY:
	case GL_DEBUG_TYPE_PERFORMANCE:
		printf(fmt, gldebug_srcstr(src), gldebug_typestr(type), msg);
		break;

	default:
		printf(fmt, gldebug_srcstr(src), gldebug_typestr(type), msg);
	}
}

static const char *gldebug_srcstr(unsigned int src)
{
	switch(src) {
	case GL_DEBUG_SOURCE_API:
		return "api";
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		return "wsys";
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		return "sdrc";
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		return "3rdparty";
	case GL_DEBUG_SOURCE_APPLICATION:
		return "app";
	case GL_DEBUG_SOURCE_OTHER:
		return "other";
	default:
		break;
	}
	return "unknown";
}

static const char *gldebug_typestr(unsigned int type)
{
	switch(type) {
	case GL_DEBUG_TYPE_ERROR:
		return "error";
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return "deprecated";
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return "undefined behavior";
	case GL_DEBUG_TYPE_PORTABILITY:
		return "portability warning";
	case GL_DEBUG_TYPE_PERFORMANCE:
		return "performance warning";
	case GL_DEBUG_TYPE_OTHER:
		return "other";
	default:
		break;
	}
	return "unknown";
}

/*
static const char *gldebug_sevstr(unsigned int sev)
{
	switch(sev) {
	case GL_DEBUG_SEVERITY_HIGH:
		return "high";
	case GL_DEBUG_SEVERITY_MEDIUM:
		return "medium";
	case GL_DEBUG_SEVERITY_LOW:
		return "low";
	default:
		break;
	}
	return "unknown";
}
*/
