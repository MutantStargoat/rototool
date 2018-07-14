#ifndef VIDEOTEX_H_
#define VIDEOTEX_H_

#include "video/video.h"

class VideoTexture {
private:
	unsigned int tex;
	int tex_width, tex_height;
	int tex_frame;
	int bound_unit;
	int vftap;

	void update_texture(int video_frame);

public:
	VideoTexture();
	~VideoTexture();

	void use_tap(int tap);
	int current_tap() const;

	int get_width() const;
	int get_height() const;
	int get_tex_width() const;
	int get_tex_height() const;

	void bind(int video_frame, int tunit = 0);
	void load_tex_scale();

	unsigned int get_texture() const;

	void invalidate();
};

#endif	// VIDEOTEX_H_
