#ifndef _CLIP_H_
#define _CLIP_H_

#include <vector>
#include <gmath/gmath.h>

typedef std::vector<int> ClipPoly;

struct ClipVertex {
	// TODO: Make it an envelope
	Vec2 pos;
};

struct Clip {
	std::vector<ClipVertex> verts;
	std::vector<ClipPoly> polys;
};

#endif // _CLIP_H_
