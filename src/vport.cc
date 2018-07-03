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
	Vec3 p = vpmat * Vec3(x, y, 0);
	return Vec2((p.x * 0.5 + 0.5) * win_width, (0.5 - p.y * 0.5) * win_height);
}

Vec2 view_to_vid(const Video *vid, float x, float y)
{
	float vidx = vid->GetWidth();
	float vidy = vid->GetHeight();
	return Vec2(vidx * 0.5 + x * vidy, vidy * 0.5 - y * vidy);
}

Vec2 vid_to_view(const Video *vid, float x, float y)
{
	float vidx = vid->GetWidth();
	float vidy = vid->GetHeight();
	return Vec2((x - vidx * 0.5) / vidy, (vidy * 0.5 - y) / vidy);
}
