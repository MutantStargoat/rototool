#ifndef _MODEL_H_
#define _MODEL_H_

#include "clip/clip.h"

class Model {
	friend class Controller;
private:
	
	Model();
	virtual ~Model();

public:
	// TODO: Allow multiple clips
	Clip clip;
};

#endif // _MODEL_H_
