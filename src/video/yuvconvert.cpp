// YUVConvert.cpp

static inline unsigned char Clip(int i)
{
	if (i > 255) return 255;
	if (i < 0) return 0;
	return (unsigned char) i;
}

static inline void ConvertYUV(unsigned char *rgb, unsigned char y, unsigned char u, unsigned char v)
{
	int C298 = (y - 16) * 298;
	int D = u - 128;
	int E = v - 128;

	rgb[2] = Clip((C298 + 409 * E + 128) >> 8);
	rgb[1] = Clip((C298 - 100 * D - 208 * E + 128) >> 8);
	rgb[0] = Clip((C298 + 516 * D + 128) >> 8);
	rgb[3] = 0xff;
}

bool ConvertYUV
#include "imgconvert.inl"