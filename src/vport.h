#ifndef VPORT_H_
#define VPORT_H_

#include <gmath/gmath.h>

class Video;

extern Mat4 proj_mat;

extern float view_pan_x, view_pan_y;
extern float view_zoom;
extern Mat4 view_mat;

void update_proj();
void update_view();

// convert pixel coordinates to viewport coordinates
Vec2 scr_to_view(float x, float y);
// convert viewport coordinates to pixel coordinates
Vec2 view_to_scr(float x, float y);

Vec2 scr_to_vid(const Video *vid, float x, float y);
Vec2 vid_to_scr(const Video *vid, float x, float y);

#endif	// VPORT_H_
