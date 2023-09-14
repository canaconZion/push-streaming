#ifndef STREAMING_P_HPP
#define STREAMING_P_HPP
#include <stdio.h>
#include <string>
#include <iostream>
#include "Rrconfig.h"
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
//#include <libavutil\opt.h>
#ifdef __cplusplus
};
#endif

class PushStreaming
{
public:
	PushStreaming();
	~PushStreaming();

    AVOutputFormat *ofmt;
    AVCodecContext* pCodecCtx;
	AVFormatContext *ifmt_ctx, *ofmt_ctx;
	AVFrame* pFrame;
	AVPacket pkt;
	char *in_filename, *out_filename;
	int videoindex=-1;
	int frame_index=0;
	int64_t start_time=0;
	// int64_t bit_rate = 400*1000;
public:
    int PushStream(char* filename, char* rtmpurl);
};

#endif