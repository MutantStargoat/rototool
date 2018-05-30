// IOCallbacks.cpp
#include "iocallbacks.h"

#define FFMPEG_BUFFER_SIZE 1024 * 256

extern "C"
{
#include <libavcodec/avcodec.h>

int FFMpegReadFunc(void *opaque, uint8_t *buf, int buf_size)
{
	FILE *file = (FILE*) opaque;
	if (!file) return -1;
	return fread(buf, 1, buf_size, file);
}

int64_t FFMpegSeekFunc(void *opaque, int64_t offset, int whence)
{
	FILE *file = (FILE*) opaque;
	if (!file) return -1;

	if (whence == AVSEEK_SIZE) {
		size_t goback = ftell(file);
		fseek(file, SEEK_END);
		size_t ret = ftell(fp);
		fseek(file, goback, SEEK_CUR);
		return ret;
	}

	fseek(file, offset, whence);
	return ftell(fp);
}

} // end extern c

void FFMpegIO::CleanUp()
{
	if (buffer) delete [] buffer;
	buffer = NULL;
	if (file) fclose(file);
	file = NULL;
}

FFMpegIO::FFMpegIO(const std::string &filename)
{
	file = NULL;
	buffer = NULL;

	try {
		file = fopen(filename.c_str(), "rb");
		if (!file) throw("Failed to open file");

		buffer = new unsigned char[FFMPEG_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
		if (!buffer) throw("Failed to allocate a buffer for video i/o");

		init_put_byte(&byte_io_context, buffer, FFMPEG_BUFFER_SIZE, 0 /*write flag*/,
			file, FFMpegReadFunc, NULL, FFMpegSeekFunc);

	} catch (const char *message) {
		printf("E: %s : %s\n", filename.c_str(), message);
		CleanUp();
	}
}

FFMpegIO::~FFMpegIO()
{
	CleanUp();
}

ByteIOContext *FFMpegIO::GetIOContext()
{
	if (!buffer || !file) return NULL;
	return &byte_io_context;
}

Integer64 FFMpegIO::Tell() const {
	if (!file) return 0;
	return ftell(file);
}
