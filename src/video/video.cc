// Video.cpp

#include <stdlib.h>
#include "video.h"
#include "imgconvert.h"

bool Video::ReadPacket()
{
	AVPacket packet;

	if (av_read_frame(pFormatCtx, &packet) < 0)
	{
		ended = true;
	}

	// Is this a packet from the video stream?
	if (packet.stream_index != videoStream)
	{
		av_packet_unref(&packet);
		return false;
	}

	// Decode video frame
	int frame_finished;
	avcodec_decode_video2(pCodecCtx, pFrame, &frame_finished, &packet);

	AVRational time_base = pFormatCtx->streams[videoStream]->time_base;
	int64_t start_time = pFormatCtx->streams[videoStream]->start_time;
	double start = 0;
	if (start_time != 0x8000000000000000)
	{
		start = (double)start_time * (double)time_base.num / (double)time_base.den;
	}

	dts = (double)pFormatCtx->streams[videoStream]->cur_dts * (double) time_base.num /
		(double) time_base.den - start;

	//dts = (double)packet.dts * (double) time_base.num / (double) time_base.den -
	//	start;

	// Free the packet that was allocated by av_read_frame
	av_packet_unref(&packet);

	return frame_finished ? true : false;
}

void Video::ConvertFrame()
{
	if (!pCodecCtx) return;
	if (pCodecCtx->pix_fmt != AV_PIX_FMT_YUV420P) return;

	if (convert_yuv) {
		ConvertYUV((unsigned char*)pFrameRGB->data[0], pFrameRGB->linesize[0],
			(const unsigned char*)pFrame->data[0], pFrame->linesize[0],
			(const unsigned char*)pFrame->data[1], pFrame->linesize[1],
			(const unsigned char*)pFrame->data[2], pFrame->linesize[2],
			pCodecCtx->width, pCodecCtx->height);
	} else {
		FlattenYUV((unsigned char*)pFrameRGB->data[0], pFrameRGB->linesize[0],
			(const unsigned char*)pFrame->data[0], pFrame->linesize[0],
			(const unsigned char*)pFrame->data[1], pFrame->linesize[1],
		(const unsigned char*)pFrame->data[2], pFrame->linesize[2],
			pCodecCtx->width, pCodecCtx->height);
	}
}

bool Video::InitVideoStream()
{
	// Retrieve stream information
	if (avformat_find_stream_info(pFormatCtx, 0) < 0)
	{
		fprintf(stderr, "E: Could not find stream information\n");
		return false;
	}

	// Find the first video stream
	videoStream = -1;
	for (unsigned int i=0; i<pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStream=i;
			break;
		}
	}
	if (videoStream == -1)
	{
		fprintf(stderr, "E: Did not find a video stream\n");
		return false;
	}

	// Get a pointer to the codec context for the video stream
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;

	// Get information about the video
	fps = 1.0 / av_q2d(pCodecCtx->time_base);

	return true;
}

bool Video::InitCodec()
{
	// Find the decoder for the video stream
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (!pCodec)
	{
		fprintf(stderr, "E: Unsupported codec\n");
		return false;
	}

	/*
	// Inform the codec that we can handle truncated bitstreams -- i.e.,
	// bitstreams where frame boundaries can fall in the middle of packets
	if(pCodec->capabilities & CODEC_CAP_TRUNCATED)
	{
		pCodecCtx->flags |= CODEC_FLAG_TRUNCATED;
	}*/

	// Open codec
	if (avcodec_open2(pCodecCtx, pCodec, 0) < 0)
	{
		fprintf(stderr, "E: Could not open codec");
		return false;
	}

	return true;
}

bool Video::InitVideoFrame()
{
	// Allocate video frame
	pFrame = av_frame_alloc();
	if (!pFrame) return false;

	// Allocate an AVFrame structure
	pFrameRGB = av_frame_alloc();
	if (!pFrameRGB) return false;

	int numBytes;
	// Determine required buffer size and allocate buffer
	numBytes = avpicture_get_size(AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height);

	//buffer = (uint8_t *) av_malloc(numBytes*sizeof(uint8_t));
	buffer = new uint8_t[numBytes];

	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture
	avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB32,
		pCodecCtx->width, pCodecCtx->height);

	return true;
}


AVInputFormat *Video::ProbeInputFormat(const char *fname)
{
	AVProbeData probe_data;
	probe_data.filename = fname;
	probe_data.buf_size = 4096;

	// try to open the file
	FILE *fp = fopen(fname, "rb");
	if (!fp) return NULL;
	// get file size
	fseek(fp, 0, SEEK_END);
	ssize_t file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (file_size < probe_data.buf_size) probe_data.buf_size = file_size;

	// allocate memory to read the first bytes
	probe_data.buf = (unsigned char *)malloc(probe_data.buf_size);
	if (!probe_data.buf) {
		fclose(fp);
		fp = NULL;
		return NULL;
	}

	// read data
	fread(probe_data.buf, 1, probe_data.buf_size, fp);

	// probe
	AVInputFormat *ret = av_probe_input_format(&probe_data, 1);

	// cleanup
	if (fp) fclose(fp);
	fp = NULL;
	if (probe_data.buf) free(probe_data.buf);
	probe_data.buf = NULL;

	return ret;
}

Video::Video()
{
	pFormatCtx = NULL;
	pCodecCtx = NULL;
	videoStream = -1;
	pCodec = NULL;
	pFrame = NULL;
	pFrameRGB = NULL;
	buffer = NULL;
	fps = 25.0;
	curr_frame = 0;
	convert_yuv = true;
	ended = false;
}

Video::~Video()
{
	close();
}

bool Video::open(const char *fname, unsigned int conv)
{
	close();

	convert_yuv = (conv & VIDEO_CONV_RGB) != 0;
	ended = false;

	av_register_all();

	// find the input format
	AVInputFormat *input_format = ProbeInputFormat(fname);
	if (!input_format) {
		fprintf(stderr, "Failed to find input format\n");
		goto err;
	}

	// open input stream
	if (avformat_open_input(&pFormatCtx, fname, input_format, NULL)) {
		fprintf(stderr, "av_open_input_stream() failed\n");
		goto err;
	}

	if (!InitVideoStream()) {
		fprintf(stderr, "Failed to init video stream\n");
		goto err;
	}

	if (!InitCodec()) {
		fprintf(stderr, "Failed to initialize the codec\n");
		goto err;
	}

	if (!InitVideoFrame()) {
		fprintf(stderr, "Failed to initialize video frames\n");
		goto err;
	}

	video_loaded = true;
	return true;

err:
	close();
	return false;
}

void Video::close()
{
	// Free the RGB image
	delete [] buffer;
	buffer = NULL;

	if (pFrameRGB) {
		av_free(pFrameRGB);
		pFrameRGB = NULL;
	}
	if (pFrame) {
		av_free(pFrame);
		pFrame = NULL;
	}
	if (pCodecCtx) {
		avcodec_close(pCodecCtx);
		pCodecCtx = NULL;
	}
	if (pFormatCtx) {
		avformat_close_input(&pFormatCtx);
		pFormatCtx = NULL;
	}
}

int Video::GetCurrentFrame() const
{
	return curr_frame;
}

bool Video::SetCurrentFrame(int frame)
{
	bool backward = frame < GetCurrentFrame();

	int64_t nanoseconds = frame;
	nanoseconds *= 40000; //1000000 / 25;

	//AVRational tb = pFormatCtx->streams[videoStream]->time_base;
	//int64_t stream_time = nanoseconds * tb.den / tb.num;

	int flags = AVSEEK_FLAG_ANY | (backward ? AVSEEK_FLAG_BACKWARD : 0);
	//int ret = av_seek_frame(pFormatCtx, videoStream, stream_time, flags);
	int ret = av_seek_frame(pFormatCtx, -1, nanoseconds, flags);
	avcodec_flush_buffers(pCodecCtx);

	curr_frame = frame;

	return ret != 0;
}

bool Video::PrepareNextFrame()
{
	if (ended) return false;

	for (int i=0; i<10000; i++)
	{
		if (ReadPacket())
		{
			ConvertFrame();
			curr_frame ++;
			return true;
		}
	}

	return false;
}

unsigned char *Video::GetBuffer()
{
	return (unsigned char*)pFrameRGB->data[0];
}

int Video::GetWidth() const
{
	return pCodecCtx->width;
}

int Video::GetHeight() const
{
	return pCodecCtx->height;
}

int Video::GetStride() const
{
	return pFrameRGB->linesize[0];
}


double Video::GetFPS() const
{
	return fps;
}

double Video::GetFrameDuration() const
{
	if (fps != 0.0)	return 1.0 / fps;
	return 1.0 / 25.0;
}

bool Video::Ended() const
{
	return ended;
}

void Video::Restart()
{
	SetCurrentFrame(0);
	ended = false;
}

double Video::GetTime() const
{
	return dts;
}

void Video::SeekToLastFrame() {
	av_seek_frame(pFormatCtx, -1, pFormatCtx->duration, 0/* AVSEEK_FLAG_ANY */);
	avcodec_flush_buffers(pCodecCtx);
}