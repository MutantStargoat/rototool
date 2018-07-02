#include "vport.h"
#include "video/video.h"
#include "app.h"

Mat4 proj_mat;

float view_pan_x, view_pan_y;
float view_zoom = 1.0f;
Mat4 view_mat;
static Mat4 vpmat, inv_vpmat;

void update_proj()
{
	if(win_aspect >= 1.0f) {
		proj_mat.scaling(2.0f / win_aspect, 2.0f, 2.0f);
	} else {
		proj_mat.scaling(2.0f, 2.0f * win_aspect, 2.0f);
	}

	vpmat = view_mat * proj_mat;
	inv_vpmat = inverse(vpmat);
}

void update_view()
{
	view_mat = Mat4::identity;
	view_mat.pre_scale(view_zoom, view_zoom, view_zoom);
	view_mat.pre_translate(view_pan_x, view_pan_y, 0);

	vpmat = view_mat * proj_mat;
	inv_vpmat = inverse(vpmat);
}

Vec2 scr_to_view(float x, float y)
{
	Vec3 p = Vec3(2.0 * x / win_width - 1.0, 1.0 - 2.0 * y / win_height, 0);
	p = inv_vpmat * p;
	return p.xy();
}

Vec2 view_to_scr(float x, float y)
{
	Vec3 p = inv_vpmat * Vec3(x, y, 0);
	return Vec2((p.x * 0.5 + 0.5) * win_width, (0.5 - p.y * 0.5) * win_height);
}

Vec2 scr_to_vid(const Video *vid, float x, float y)
{
	Vec2 p = scr_to_view(x, y);
	float vidy = vid->GetHeight();
	return Vec2(p.x * vidy, p.y * vidy);
}

Vec2 vid_to_scr(const Video *vid, float x, float y)
{
	float vidy = vid->GetHeight();
	Vec2 p = Vec2(x / vidy, y / vidy);
	return view_to_scr(p.x, p.y);
}
