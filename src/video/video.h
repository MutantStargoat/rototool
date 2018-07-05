// Video.h

#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <string>
#include <vector>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#define MAX_CACHED_FRAMES 8

enum {
	VIDEO_CONV_NONE,
	VIDEO_CONV_RGB = 1,
};

class Video
{
	friend class VideoOps;
protected:
	AVFormatContext *pFormatCtx;
	AVCodecContext *pCodecCtx;
	int videoStream;
	AVCodec *pCodec;
	AVFrame *pFrame;
	AVFrame *pFrameRGB;
	uint8_t *buffer;
	double fps;

	bool convert_yuv;

	bool ReadPacket();
	void ConvertFrame();
	bool InitVideoStream();
	bool InitCodec();
	bool InitVideoFrame();

	bool video_loaded;

	bool ended;

	AVInputFormat *ProbeInputFormat(const char *fname);

	bool ReadAndDecodeUntilFrameComplete();

	// all of the cached frame pixel data, one after the other
	unsigned char *pixel_buffer;

	struct CachedFrame {
		bool valid;
		unsigned char *pixels;	// linear buffer
	};

	struct FrameIndex {
		int64_t packet_offset;
		double time_seconds;
		bool key;
		int64_t dts;
	};

	std::vector<FrameIndex> frame_index;

	// frame cache
	int frame_cache_base; // frame number of frame_cache[0] (whether it's available or not)
	CachedFrame frame_cache[MAX_CACHED_FRAMES];

	void clearCache();

	bool SeekToFrame(int frame, int *landed_at = nullptr);

public:
	Video();
	~Video();

	bool open(const char *fname, unsigned int conv = VIDEO_CONV_RGB);
	void close();
	bool is_open() const;

	bool GetFrame(int frame, unsigned char **pixels);
	double GetFrameTimeSeconds(int frame) const;

	int GetWidth() const;
	int GetHeight() const;

	double GetFPS() const;
	double GetFrameDuration() const;
	bool Ended() const;

	void SeekToLastFrame();
};



#endif // ndef _VIDEO_H_
