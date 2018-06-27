#include "clip.h"

#include <algorithm>

void ClipPoly::cache(const Clip &clip) {
	verts.resize(0);
	bool first = true;
	for (const int i : *this) {
		const Vec2 &v = clip.verts[i].pos;
		verts.push_back(v);
		if (first) {
			first = false;
			bb_min = bb_max = v;
		} else {
			bb_min.x = std::min(bb_min.x, v.x);
			bb_min.y = std::min(bb_min.y, v.y);
			bb_max.x = std::max(bb_max.x, v.x);
			bb_max.y = std::max(bb_max.y, v.y);
		}
	}
}

void ClipPoly::apply(Clip &clip) const {
	if (size() != verts.size()) {
		return;
	}

	for (int i = 0; i < (int)size(); i++) {
		clip.verts[i].pos = verts[i];
	}
}

bool ClipPoly::contains(const Vec2 &p) const {
	if (p.x < bb_min.x || p.x > bb_max.x) return false;
	if (p.y < bb_min.y || p.y > bb_max.y) return false;

	// TODO: implement point-in-poly

	return true;
}