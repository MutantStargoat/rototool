// YUVFlatten.cpp

static inline void ConvertYUV(unsigned char *rgb, unsigned char y, unsigned char u, unsigned char v)
{
	rgb[2] = y;
	rgb[1] = u;
	rgb[0] = v;
}

bool FlattenYUV
#include "imgconvert.inl"