#ifndef VIDEOTEX_H_
#define VIDEOTEX_H_

#include "video/video.h"

class VideoTexture {
private:
	Video vid;
	unsigned int tex;
	int tex_width, tex_height;
	int tex_frame, cur_frame;
	int bound_unit;

	void update_texture();

public:
	VideoTexture();
	~VideoTexture();

	bool open(const char *fname);
	void close();

	int get_width() const;
	int get_height() const;

	void rewind();
	void seek_frame(int frm);
	void seek_frame_rel(int dfrm);
	int get_cur_frame() const;

	void bind(int tunit = 0);
	void load_tex_scale();
};

#endif	// VIDEOTEX_H_
