#ifndef _CLIP_H_
#define _CLIP_H_

#include <vector>

#include "clipvertex.h"
#include "clippoly.h"

class Clip {
public:
	Clip();
	virtual ~Clip();

	std::vector<ClipVertex> verts;
	std::vector<ClipPoly> polys;
};

#endif // _CLIP_H_