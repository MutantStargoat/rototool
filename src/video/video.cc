// Video.cpp

#include <stdlib.h>
#include <string.h>
#include "video.h"
#include "imgconvert.h"
#include <algorithm>

#include <libavutil/error.h>

bool Video::ReadPacket()
{
	AVPacket packet;
	int ret = av_read_frame(pFormatCtx, &packet);
	if (ret < 0)
	{
		printf("WTF2: %d\n", ret);
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
	memset(&probe_data, 0, sizeof probe_data);
	probe_data.filename = fname;
	probe_data.buf_size = 4096;

	// try to open the file
	FILE *fp = fopen(fname, "rb");
	if (!fp) {
		fprintf(stderr, "Failed to open file: %s\n", fname);
		return NULL;
	}
	// get file size
	fseek(fp, 0, SEEK_END);
	size_t file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (file_size < (size_t)probe_data.buf_size) probe_data.buf_size = (int)file_size;

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
	convert_yuv = true;
	ended = false;
	pixel_buffer = nullptr;
	video_loaded = false;
}

Video::~Video()
{
	close();
}

bool Video::open(const char *fname, unsigned int conv)
{
	size_t bytes_per_frame, buffer_size;
	int64_t last_pts;
	double time_base;

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
	pFormatCtx->flags |= AVFMT_FLAG_FLUSH_PACKETS;

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

	// allocate pixel buffer;
	bytes_per_frame = GetWidth() * GetHeight() * 4;
	buffer_size = MAX_CACHED_FRAMES * bytes_per_frame;
	pixel_buffer = new unsigned char[buffer_size];

	// initialize frame cache
	frame_cache_base = 0;
	clearCache();

	// create frame index
	last_pts = -1;
	time_base = av_q2d(pFormatCtx->streams[videoStream]->time_base);
	while (true) {
		AVPacket packet;
		if (av_read_frame(pFormatCtx, &packet) < 0) {
			// end of file
			break;
		}

		// Is this a packet from the video stream?
		if (packet.stream_index != videoStream) {
			av_packet_unref(&packet);
			continue;
		}

		// is this packet part of a previous frame?
		if (last_pts == packet.pts) {
			av_packet_unref(&packet);
			continue;
		}

		last_pts = packet.pts;
		FrameIndex index;
		index.packet_offset = packet.pos;
		index.key = (packet.flags & AV_PKT_FLAG_KEY) ? true : false;
		index.time_seconds = time_base * (double)packet.pts;
		index.dts = packet.dts;
		frame_index.push_back(index);

		av_packet_unref(&packet);
	}

	// seek to the first frame
	if (frame_index.size() > 0) {
		SeekToFrame(0);
	}

	return true;

err:
	close();
	return false;
}

void Video::close()
{
	for (int i = 0; i < MAX_CACHED_FRAMES; i++) {
		frame_cache[i].pixels = nullptr;
		frame_cache[i].valid = false;
	}

	// free pixel buffer
	if (pixel_buffer) {
		delete[] pixel_buffer;
		pixel_buffer = nullptr;
	}

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

	video_loaded = false;
}

bool Video::is_open() const
{
	return video_loaded;
}

void Video::clearCache() {

	size_t bytes_per_frame = GetWidth() * GetHeight() * 4;

	for (int i = 0; i < MAX_CACHED_FRAMES; i++) {
		frame_cache[i].valid = false;
		frame_cache[i].pixels = pixel_buffer + i * bytes_per_frame;
	}
}

bool Video::GetFrame(int frame, unsigned char **pixels) {
	if (frame < 0) {
		return false;
	}

	// check if frame is cached
	if (frame >= frame_cache_base && frame < frame_cache_base + MAX_CACHED_FRAMES) {
		int cache_index = frame - frame_cache_base;

		if (frame_cache[cache_index].valid) {
			// hit
			if (pixels) {
				*pixels = frame_cache[cache_index].pixels;
			}
			return true;
		}
	}

	// otherwise, we need to refill cache
	clearCache();
	frame_cache_base = frame - MAX_CACHED_FRAMES / 2;
	if (frame_cache_base < 0) {
		frame_cache_base = 0;
	}

	// seek the video stream to previous keyframe
	int landed_at;
	if (!SeekToFrame(frame_cache_base, &landed_at)) {
		//printf("E: Failed to seek to frame %d\n", frame_cache_base);
		return false;
	}

	// read and decode until we have filled our cache
	for (int i=landed_at; i< frame_cache_base + MAX_CACHED_FRAMES; i++) {
		if (frame_cache[MAX_CACHED_FRAMES - 1].valid) {
			break;
		}

		if (ReadAndDecodeUntilFrameComplete() == false) {
			// something broke
			return false;
		}

		int index = i - frame_cache_base;
		if (index < 0 || index >= MAX_CACHED_FRAMES) {
			// don't cache this. it's outside cached window
			continue;
		}

		// we've got ourselves a new frame. Add it to cache
		CachedFrame &cached_frame = frame_cache[index];

		// copy pixels
		const unsigned char *src = (unsigned char*)pFrameRGB->data[0];
		int stride = pFrameRGB->linesize[0];
		int scanline_bytes = GetWidth() * 4;
		unsigned char *dst = cached_frame.pixels;

		// cache pixels
		for (int j = 0; j < GetHeight(); j++) {
			memcpy(dst, src, scanline_bytes);
			src += stride;
			dst += scanline_bytes;
		}
				
		cached_frame.valid = true;
	}

	// check again that we have it
	if (frame >= frame_cache_base && frame < frame_cache_base + MAX_CACHED_FRAMES) {
		int cache_index = frame - frame_cache_base;

		if (frame_cache[cache_index].valid) {
			// hit
			if (pixels) {
				*pixels = frame_cache[cache_index].pixels;
			}
			return true;
		}
	}

	return false;
}

bool Video::SeekToFrame(int frame, int *landed_at)
{
	frame = std::max(0, std::min((int) frame_index.size(), frame));

	// find nearest keyframe
	int nearest_key = 0;
	for (int i = frame; i >= 0; i--) {
		if (frame_index[i].key) {
			nearest_key = i;
			break;
		}
	}

	const FrameIndex &index = frame_index[nearest_key];

	int64_t ret = avio_seek(pFormatCtx->pb, index.packet_offset, SEEK_SET);
	if (ret < 0) {
		printf("Seek error\n");
	}

	avformat_seek_file(pFormatCtx, videoStream, index.dts, index.dts, index.dts, 0);

	avcodec_flush_buffers(pCodecCtx);

	if (landed_at != nullptr) {
		*landed_at = nearest_key;
	}

	return true;
}

bool Video::ReadAndDecodeUntilFrameComplete()
{
	for (int i=0; i<10000; i++)
	{
		if (ReadPacket())
		{
			ConvertFrame();
			return true;
		}
	}

	printf("WTF\n");
	return false;
}


int Video::GetWidth() const
{
	return pCodecCtx->width;
}

int Video::GetHeight() const
{
	return pCodecCtx->height;
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


void Video::SeekToLastFrame() {
	av_seek_frame(pFormatCtx, -1, pFormatCtx->duration, 0/* AVSEEK_FLAG_ANY */);
	avcodec_flush_buffers(pCodecCtx);
}
