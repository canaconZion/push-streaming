
#include "streaming_p.h"

PushStreaming::PushStreaming()
{
}
PushStreaming::~PushStreaming()
{
}

int PushStreaming::PushStream(char *filename, char *rtmpurl)
{
	ofmt = nullptr;
	ifmt_ctx = nullptr;
	ofmt_ctx = nullptr;
	in_filename = filename;
	out_filename = rtmpurl;
	int ret, i;
	if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0)
	{
		printf("Could not open input file.");
		goto end;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0)
	{
		printf("Failed to retrieve input stream information");
		goto end;
	}

	for (i = 0; i < ifmt_ctx->nb_streams; i++)
		if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}

	av_dump_format(ifmt_ctx, 0, in_filename, 0);

	avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", out_filename); // RTMP

	if (!ofmt_ctx)
	{
		printf("Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt = ofmt_ctx->oformat;
	for (i = 0; i < ifmt_ctx->nb_streams; i++)
	{
		// 根据输入流创建输出流（Create output AVStream according to input AVStream）
		AVStream *in_stream = ifmt_ctx->streams[i];
		AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
		if (!out_stream)
		{
			printf("Failed allocating output stream\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}
		// 复制AVCodecContext的设置（Copy the settings of AVCodecContext）
		//  in_stream->codec->bit_rate = bit_rate;
		ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
		if (ret < 0)
		{
			printf("Failed to copy context from input to output stream codec context\n");
			goto end;
		}
		out_stream->codec->codec_tag = 0;
		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
	av_dump_format(ofmt_ctx, 0, out_filename, 1);
	// 打开输出URL（Open output URL）
	if (!(ofmt->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			printf("Could not open output URL '%s'\n", out_filename);
			goto end;
		}
	}
	// 写文件头（Write file header）
	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0)
	{
		printf("Error occurred when opening output URL\n");
		goto end;
	}

	start_time = av_gettime();
	pFrame = av_frame_alloc();
	printf("Streaming...\n");
	while (1)
	{
		AVStream *in_stream, *out_stream;
		// 获取一个AVPacket（Get an AVPacket）
		ret = av_read_frame(ifmt_ctx, &pkt);
		if (ret < 0)
			break;
		int got_picture = 0;
		if (pkt.pts == AV_NOPTS_VALUE)
		{
			// Write PTS
			AVRational time_base1 = ifmt_ctx->streams[videoindex]->time_base;
			int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(ifmt_ctx->streams[videoindex]->r_frame_rate);
			// Parameters
			pkt.pts = (double)(frame_index * calc_duration) / (double)(av_q2d(time_base1) * AV_TIME_BASE);
			pkt.dts = pkt.pts;
			pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1) * AV_TIME_BASE);
		}
		if (pkt.stream_index == videoindex)
		{
			AVRational time_base = ifmt_ctx->streams[videoindex]->time_base;
			AVRational time_base_q = {1, AV_TIME_BASE};
			int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
			int64_t now_time = av_gettime() - start_time;
			if (pts_time > now_time)
				av_usleep(pts_time - now_time);
		}

		in_stream = ifmt_ctx->streams[pkt.stream_index];
		out_stream = ofmt_ctx->streams[pkt.stream_index];
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		if (pkt.stream_index == videoindex)
		{
			frame_index++;
		}
		ret = av_interleaved_write_frame(ofmt_ctx, &pkt);

		if (ret < 0)
		{
			printf("Error muxing packet!\n");
			break;
		}
		// printf("Send one frame successful\n");
		av_free_packet(&pkt);
	}
	av_write_trailer(ofmt_ctx);
end:
	avformat_close_input(&ifmt_ctx);
	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);
	if (ret < 0 && ret != AVERROR_EOF)
	{
		printf("Error occurred.\n");
		return ret;
	}
	return 0;
}
