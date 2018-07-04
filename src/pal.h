#ifndef PAL_H_
#define PAL_H_

#include <gmath/gmath.h>

extern Vec3 *palette;
extern int palette_size;

bool init_palette(int psz);
void destroy_palette();

// TODO: nearest color, palette optimization, etc

#endif	// PAL_H_
