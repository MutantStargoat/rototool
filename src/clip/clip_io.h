#ifndef _CLIP_IO_H_
#define _CLIP_IO_H_

#include "clip.h"

class ClipIO {
public:
	ClipIO();
	virtual ~ClipIO();

	bool load(const char *filename, Clip *clip);
	bool save(const char *filename, const Clip &clip);

};

#endif // _CLIP_IO_H_
