#include "clip.h"

#include <algorithm>
#include <list>

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

	triangulate();
}

void ClipPoly::apply(Clip &clip) const {
	if (size() != verts.size()) {
		return;
	}

	for (int i = 0; i < (int)size(); i++) {
		clip.verts[(*this)[i]].pos = verts[i];
	}
}

bool ClipPoly::contains(const Vec2 &p) const {
	if (p.x < bb_min.x || p.x > bb_max.x) return false;
	if (p.y < bb_min.y || p.y > bb_max.y) return false;

	// test a random ray (pointing up) against all edges
	int hit_count = 0;
	for (int i = 0; i < (int)size(); i++) {
		int j = (i + 1) % ((int)size());

		const Vec2 &a = verts[i];
		const Vec2 &b = verts[j];

		// is edge completely at the left or right side of the point?
		if (a.x < p.x && b.x < p.x) continue;
		if (a.x > p.x && b.x > p.x) continue;

		// p is in the middle horizontally. If it is over both vertices, it's out
		if (a.y < p.y && b.y < p.y) continue;

		// if it is under both vertices, it is in
		if (a.y >= p.y && b.y >= p.y) {
			hit_count++;
			continue;
		}

		// We'll calculate the winding of the triangle ABP. Convert to Vec3
		Vec3 a3(a.x, a.y, 0);
		Vec3 b3(b.x, b.y, 0);
		Vec3 p3(p.x, p.y, 0);

		// p is between a and b. Make sure a is at the left
		if (a.x > b.x) std::swap(a3, b3);

		Vec3 c = cross(b3 - a3, p3 - b3);

		if (c.z < 0) hit_count++; // clockwise
	}

	return (hit_count & 1) != 0;
}

static bool trim_ear(const ClipPoly &clip_poly, std::list<int> &poly, std::vector<int> &tris) {
	if (poly.size() < 4) {
		return false;
	}

	for (auto it = poly.begin(); it != poly.end(); it++) {
		auto a = it;
		auto b = std::next(a);
		if (b == poly.end()) b = poly.begin();
		auto c = std::next(b);
		if (c == poly.end()) c = poly.begin();

		// the triangle is an "ear" if the middle of BC is inside the polygon
		Vec2 m = (clip_poly.verts[*a] + clip_poly.verts[*c]) * 0.5f;

		if (clip_poly.contains(m)) {
			// we've got an "ear"
			tris.push_back(*a);
			tris.push_back(*b);
			tris.push_back(*c);
			poly.erase(b);
			return true;
		}
	}

	return false;
}

void ClipPoly::triangulate() {
	triangles.resize(0);

	if (size() < 3) {
		return;
	}

	if (size() == 3) {
		// triangle indices refer to the cached vertices of this polygon
		triangles.push_back(0);
		triangles.push_back(1);
		triangles.push_back(2);
		return;
	}

	// trim "ears"
	std::list<int> tmp;
	for (int i = 0; i < (int)size(); i++) {
		tmp.push_back(i);
	}
	while (tmp.size() > 3) {
		if (!trim_ear(*this, tmp, triangles)) break;
	}

	if (tmp.size() == 3) {
		for (const int i : tmp) {
			triangles.push_back(i);
		}
	}
}

