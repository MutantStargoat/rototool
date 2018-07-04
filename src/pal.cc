#include "pal.h"

Vec3 *palette;
int palette_size;

bool init_palette(int sz)
{
	palette = new Vec3[sz];
	palette_size = sz;

	for(int i=0; i<sz; i++) {
		palette[i] = Vec3(1, 1, 1);
	}
	return true;
}

void destroy_palette()
{
	delete [] palette;
	palette = 0;
	palette_size = 0;
}
