#ifndef _CLIP_VERTEX_H_
#define _CLIP_VERTEX_H_

#include <gmath/gmath.h>

class ClipVertex {
public:
	ClipVertex();
	virtual ~ClipVertex();


public:
	// TODO: Make it an envelope
	Vec2 pos;
};

#endif // _CLIP_VERTEX_H_