#ifndef _CLIP_H_
#define _CLIP_H_

#include <map>
#include <set>
#include <vector>
#include <gmath/gmath.h>

struct Clip;

class ClipPoly : public std::vector<int> {
public:
	ClipPoly();

	// cached stuff
	std::vector<Vec2> verts;
	Vec2 bb_min, bb_max;
	std::vector<int> triangles;
	Vec3 color;
	int palcol;	/* default -1: direct color, ignoring the palette */
	int z;

	void cache(const Clip &clip, int frame);
	void apply(Clip &clip, int frame) const;

	bool contains(const Vec2 &p) const;

	// returns closest point on contour
	Vec2 closest_point(const Vec2 &p, int *edge_a = nullptr, int *edge_b = nullptr) const;

	void triangulate();

	friend bool operator ==(const ClipPoly &a, const ClipPoly &b);
};

class ClipVertex {
public:
	ClipVertex();
	// lerp all frames of the other two clipverts
	ClipVertex(const ClipVertex &a, const ClipVertex &b, float t);
	~ClipVertex();


	Vec2 get_pos(int frame) const;
	void set_pos(const Vec2 &pos, int frame);

	std::set<int> get_keyframes() const;

private:
	std::map<int, Vec2> pos;
};

struct Clip {
	Clip();
	~Clip();

	void remove_poly(ClipPoly *poly);
	void clear_orphaned_verts();
	void sort_polys();

public:
	std::vector<ClipVertex> verts;
	std::vector<ClipPoly> polys;

	int cur_video_frame;

	// TODO: remove
	double cur_video_time;

	
};

#endif // _CLIP_H_
