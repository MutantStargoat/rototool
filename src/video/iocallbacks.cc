// IOCallbacks.cpp
#include "iocallbacks.h"

#define FFMPEG_BUFFER_SIZE 1024 * 256

extern "C"
{
#include <libavcodec/avcodec.h>

static int read_func(void *opaque, uint8_t *buf, int buf_size)
{
	FILE *file = ((FFMpegIO*)opaque)->file;
	if (!file) return -1;
	return fread(buf, 1, buf_size, file);
}

static int64_t seek_func(void *opaque, int64_t offset, int whence)
{
	FFMpegIO *ffio = (FFMpegIO*)opaque;

	if (!ffio->file) return -1;

	if (whence == AVSEEK_SIZE) {
		return ffio->fsize;
	}

	fseek(ffio->file, offset, whence);
	return ftell(ffio->file);
}

} // end extern c


FFMpegIO::FFMpegIO()
{
	file = NULL;
	buffer = NULL;
	fsize = 0;
	io_ctx = 0;
}

FFMpegIO::~FFMpegIO()
{
	close();
}

bool FFMpegIO::open(const char *fname)
{
	close();

	if(!(file = fopen(fname, "rb"))) {
		fprintf(stderr, "FFMpegIO::open: failed to open %s: %s\n", fname, strerror(errno));
		return false;
	}
	fseek(file, 0, SEEK_END);
	fsize = ftell(file);
	rewind(file);

	try {
		buffer = new unsigned char[FFMPEG_BUFFER_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
	}
	catch(...) {
		fprintf(stderr, "FFMpegIO::open: failed to allocate a buffer for video i/o\n");
		close();
		return false;
	}

	if(!(io_ctx = avio_alloc_context(buffer, FFMPEG_BUFFER_SIZE, 0, this, read_func, 0, seek_func))) {
		fprintf(stderr, "FFMpegIO::open: failed to allocate AVIOContext\n");
		close();
		return false;
	}

	return true;
}

void FFMpegIO::close()
{
	delete [] buffer;
	buffer = NULL;

	if(file) {
		fclose(file);
		file = NULL;
	}
	fsize = 0;

	if(io_ctx) {
		avio_context_free(&io_ctx);
		io_ctx = 0;
	}
}

AVIOContext *FFMpegIO::GetIOContext()
{
	if (!buffer || !file) return NULL;
	return io_ctx;
}

int64_t FFMpegIO::Tell() const
{
	if (!file) return 0;
	return ftell(file);
}
