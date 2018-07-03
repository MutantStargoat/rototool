#ifndef _MODEL_H_
#define _MODEL_H_

#include <clip/clip.h>
#include <video/video.h>

class Model {
	friend class Controller;
private:
	Model();
	virtual ~Model();

public:
	// TODO: Allow multiple clips
	Clip clip;
	Video video;
	
	int get_cur_video_frame() const;

private:
	int cur_video_frame;
};

#endif // _MODEL_H_
