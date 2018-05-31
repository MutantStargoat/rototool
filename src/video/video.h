// Video.h

#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

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

	double dts;
	int curr_frame;
	bool video_loaded;

	bool ended;

	AVInputFormat *ProbeInputFormat(const std::string &filename);

public:
	Video();
	~Video();

	bool open(const char *fname, unsigned int convert = VIDEO_CONV_RGB);
	void close();

	int GetCurrentFrame() const {return curr_frame;}
	bool SetCurrentFrame(int frame);
	bool PrepareNextFrame();
	unsigned char *GetBuffer();

	int GetWidth() const {return pCodecCtx->width;}
	int GetHeight() const {return pCodecCtx->height;}
	int GetStride() const {return pFrameRGB->linesize[0];}

	double GetFPS() const;
	double GetFrameDuration() const;
	bool Ended() const;
	void Restart();

	double GetTime() const;

	void SeekToLastFrame();
};



#endif // ndef _VIDEO_H_
