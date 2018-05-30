#ifndef OPENGL_H_
#define OPENGL_H_

#include <GL/glew.h>

struct GLCaps {
	int debug;	/* ARB_debug_output */
};

extern struct GLCaps glcaps;

#ifdef __cplusplus
extern "C" {
#endif

int init_opengl(void);

#ifdef __cplusplus
}
#endif

#endif	/* OPENGL_H_ */
