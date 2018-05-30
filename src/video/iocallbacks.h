// IOCallbacks.h

#ifndef _IO_CALLBACKS_H_
#define _IO_CALLBACKS_H_

#include <stdio.h>
#include <string>

extern "C"
{
#include <libavformat/avio.h>
} // end extern c


class FFMpegIO
{
protected:
	ByteIOContext byte_io_context;
	unsigned char *buffer;
	FILE *file;

	void CleanUp();

public:
	FFMpegIO(const std::string &filename);
	~FFMpegIO();

	ByteIOContext *GetIOContext();

	Integer64 Tell() const;
};


#endif // ndef _IO_CALLBACKS_H_
