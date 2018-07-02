#ifndef _MODEL_H_
#define _MODEL_H_

#include <clip/clip.h>
#include <video/video.h>

class Model {
public:
	// TODO: Allow multiple clips
	Clip clip;
	Video video;

	Model();
	virtual ~Model();
};

#endif // _MODEL_H_
