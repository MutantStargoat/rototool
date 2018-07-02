#include "vport.h"
#include "video/video.h"
#include "app.h"

Mat4 proj_mat;

float view_pan_x, view_pan_y;
float view_zoom = 1.0f;
Mat4 view_mat;

void update_proj()
{
	if(win_aspect >= 1.0f) {
		proj_mat.scaling(2.0f / win_aspect, 2.0f, 2.0f);
	} else {
		proj_mat.scaling(2.0f, 2.0f * win_aspect, 2.0f);
	}
}

void update_view()
{
	view_mat = Mat4::identity;
	view_mat.pre_scale(view_zoom, view_zoom, view_zoom);
	view_mat.pre_translate(view_pan_x, view_pan_y, 0);
}

Vec2 scr_to_view(float x, float y)
{
	Vec3 p = Vec3(2.0 * x / win_width - 1.0, 1.0 - 2.0 * y / win_height, 0);
	p = inverse(proj_mat) * p;
	p = inverse(view_mat) * p;
	return p.xy();
}

Vec2 view_to_scr(float x, float y)
{
	Vec3 p = Vec3(x, y, 0);
	p = view_mat * p;
	p = proj_mat * p;
	return Vec2((p.x * 0.5 + 0.5) * win_width, (0.5 - p.y * 0.5) * win_height);
}

Vec2 scr_to_vid(const Video *vid, float x, float y)
{
	Vec2 p = scr_to_view(x, y);
	float vidx = vid->GetWidth();
	float vidy = vid->GetHeight();
	return Vec2((p.x * 0.5 + 0.5) * vidx, (0.5 - p.y * 0.5) * vidy);
}

Vec2 vid_to_scr(const Video *vid, float x, float y)
{
	return Vec2(0, 0);	// TODO

}
