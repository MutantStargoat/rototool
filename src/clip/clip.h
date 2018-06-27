#ifndef _CLIP_H_
#define _CLIP_H_

#include <vector>
#include <gmath/gmath.h>

struct Clip;

class ClipPoly : public std::vector<int> {
public:
	// cached stuff
	std::vector<Vec2> verts;
	Vec2 bb_min, bb_max;

	void cache(const Clip &clip);
	void apply(Clip &clip) const;

	bool contains(const Vec2 &p) const;
};

struct ClipVertex {
	// TODO: Make it an envelope
	Vec2 pos;
};

struct Clip {
	std::vector<ClipVertex> verts;
	std::vector<ClipPoly> polys;
};

#endif // _CLIP_H_
