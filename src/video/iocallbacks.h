// IOCallbacks.h

#ifndef _IO_CALLBACKS_H_
#define _IO_CALLBACKS_H_

#include <stdio.h>
#include <stdint.h>

extern "C"
{
#include <libavformat/avio.h>
} // end extern c


class FFMpegIO
{
protected:
	unsigned char *buffer;

public:
	AVIOContext *io_ctx;
	FILE *file;
	int64_t fsize;

	FFMpegIO();
	~FFMpegIO();

	bool open(const char *fname);
	void close();

	AVIOContext *GetIOContext();

	int64_t Tell() const;
};


#endif // ndef _IO_CALLBACKS_H_
