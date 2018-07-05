#include "clip.h"

#include <algorithm>
#include <list>

ClipPoly::ClipPoly()
{
	color = Vec3(0.5, 0.5, 0.5);
	palcol = -1;
}

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

static inline bool cw(const Vec2 &a, const Vec2 &b, const Vec2 &c) {
	Vec2 s = b - a;
	Vec2 t = c - a;
	return (s.x * t.y - s.y * t.x) < 0;
}

bool ClipPoly::contains(const Vec2 &p) const {
	if (p.x < bb_min.x || p.x > bb_max.x) return false;
	if (p.y < bb_min.y || p.y > bb_max.y) return false;

	// test a random ray (pointing up) against all edges
	int hit_count = 0;
	for (int i = 0; i < (int)size(); i++) {
		int j = (i + 1) % ((int)size());

		Vec2 a = verts[i];
		Vec2 b = verts[j];

		// Make sure a is at the left
		if (a.x > b.x) std::swap(a, b);

		// is edge completely at the left or right side of the point?
		if (a.x > p.x) continue;
		if (b.x <= p.x) {
			continue;
		}

		// p is in the middle horizontally. If it is over both vertices, it's out
		if (a.y < p.y && b.y < p.y) {
			continue;
		}

		// if it is under both vertices, it is in
		if (a.y >= p.y && b.y >= p.y) {
			hit_count++;
			continue;
		}

		if (cw(a, b, p)) {
			hit_count++;
		}
	}

	return (hit_count & 1) != 0;
}

static Vec2 project(const Vec2 &a, const Vec2 &b, const Vec2 &p) {
	Vec2 abn = b - a;
	abn.normalize();

	Vec2 perp = a - p;
	perp -= abn * dot(abn, perp);

	return p + perp;
}

Vec2 ClipPoly::closest_point(const Vec2 &p, int *edge_a, int *edge_b) const {
	if (size() < 3) return Vec2();

	float mind;
	Vec2 ret;
	bool found = false;
	for (int i = 0; i < (int)size(); i++) {
		int j = (i + 1) % ((int)size());

		Vec2 proj = project(verts[i], verts[j], p);
		float d = distance(p, proj);

		// check if projection is within the edge
		if (dot(proj - verts[i], proj - verts[j]) > 0) {
			continue;
		}

		if (!found || d < mind) {
			found = true;
			ret = proj;
			mind = d;
			if (edge_a) *edge_a = i;
			if (edge_b) *edge_b = j;
		}
	}

	// include vertices to the search
	for (int i = 0; i < (int)size(); i++) {
		float d = distance(p, verts[i]);

		if (!found || d < mind) {
			found = true;
			ret = verts[i];
			mind = d;
			if (edge_a) *edge_a = i;
			if (edge_b) *edge_b = (i + 1) % ((int) size());
		}
	}

	return ret;
}

static bool intersect_line_seg(const Vec2 &a1, const Vec2 &a2, const Vec2 &b1, const Vec2 &b2) {
	if (cw(a1, a2, b1) == cw(a1, a2, b2)) {
		return false;
	}

	if (cw(b1, b2, a1) == cw(b1, b2, a2)) {
		return false;
	}
	
	return true;
}

static bool trim_ear(const ClipPoly &original, std::list<int> &poly, std::vector<int> &tris) {
	if (poly.size() < 4) {
		return false;
	}

	for (auto it = poly.begin(); it != poly.end(); it++) {
		auto a = it;
		auto b = std::next(a);
		if (b == poly.end()) b = poly.begin();
		auto c = std::next(b);
		if (c == poly.end()) c = poly.begin();

		// the triangle is an "ear" if AC is completely inside the polygon
		
		// so it must be clockwise
		if (!cw(original.verts[*a], original.verts[*b], original.verts[*c])) {
			continue;
		}

		// A completely inside edge must not intersect any other non-adjacent edge
		bool intersects_other_edge = false;
		for (auto it2 = poly.begin(); it2 != poly.end(); it2++) {
			auto ea = it2;
			auto eb = std::next(ea);
			if (eb == poly.end()) eb = poly.begin();

			// ignore same or adjacent edges
			if (ea == a || ea == c) continue;
			if (eb == a || eb == c) continue;

			if (intersect_line_seg(original.verts[*a], original.verts[*c],
				original.verts[*ea], original.verts[*eb])) {
				// not an ear
				intersects_other_edge = true;
				break;
			}
		}
		if (intersects_other_edge) continue;

		// we've got an "ear"
		tris.push_back(*a);
		tris.push_back(*b);
		tris.push_back(*c);
		poly.erase(b);
		return true;		
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

Clip::Clip() {
	cur_video_frame = 0;
	cur_video_time = 0;
}

Clip::~Clip() {
	
}
