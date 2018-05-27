#include"Emedia.h"
#include"EmediaImpl.h"
#include"EmpegException.h"
#include<iostream>
using namespace std;

#ifndef _FFMPEG_H
#define _FFMPEG_H
extern "C"{
#include"libavformat/avformat.h"
#include"libavcodec/avcodec.h"
#include<libswscale/swscale.h>
#include"libavutil/imgutils.h"
}
#endif

shared_ptr<Emedia> Emedia::generate(const string& path){
	std::string fileType;
	getFileType(path, fileType);
	
	if (fileType != "mp4"){
		throw ParamExceptionPara(path, fileType);
	}
	shared_ptr<Emedia> ptr = shared_ptr<Emedia>(new EmediaImpl(path));	

	std::cout << "--------------------------" << std::endl;
	ptr->_open_();			//throw
	return  ptr;
}


//--合成音频、视频
static bool combine(const std::string& videoPath, const std::string& audioPath, const std::string& mediaPath)
{
	Muxer muxer(videoPath, audioPath, mediaPath);
	muxer.combineVideoAudio();				//throw
	return 0;
}

//--指针处理
void Muxer::EmideaClose(){
	avformat_close_input(&_ifmt_ctx_v);
	avformat_close_input(&_ifmt_ctx_a);
	/* close output */
	if (_ofmt_ctx && !(_ofmt->flags & AVFMT_NOFILE))
		avio_close(_ofmt_ctx->pb);
	avformat_free_context(_ofmt_ctx);
	int ret = -1;
	if (ret < 0 && ret != AVERROR_EOF) {
		printf("Error occurred.\n");
		//return -1;
	}
}

Muxer::Muxer(const std::string& videoPath, const std::string& audioPath, const std::string& mediaPath)
:_videoPath(videoPath), _audioPath(audioPath), _mediaPath(mediaPath)
{
	av_register_all();								//初始化封装												
	avformat_network_init();						//初始化网络库 （可以打开rtsp rtmp http 协议的流媒体视频）
	avcodec_register_all();							//注册解码器
	AVDictionary *opts = NULL;						//参数设置
	av_dict_set(&opts, "rtsp_transport", "tcp", 0); //设置rtsp流已tcp协议打开
	av_dict_set(&opts, "max_delay", "500", 0);		//网络延时时间

	_videoindex_v = -1;
	_audioindex_a = -1;
	_videoindex_out = -1;
	_audioindex_out = -1;
	frame_index = -1;
}

Muxer::Muxer()
:_videoPath(nullptr), _audioPath(nullptr), _mediaPath(nullptr){
	_videoindex_v = -1;
	_audioindex_a = -1;
	_videoindex_out = -1;
	_audioindex_out = -1;
	frame_index = -1;
}

Muxer::~Muxer(){
	EmideaClose();
}

bool Muxer::combineVideoAudio(){
	//AVPacket pkt;
	int ret;
	int64_t cur_pts_v = 0, cur_pts_a = 0;

	const char *out_filename = _mediaPath.c_str();//Output file URL  

	av_register_all();

	//--指针处理
	openInit();	//Input					//throw

	//Output  
	avformat_alloc_output_context2(&_ofmt_ctx, NULL, NULL, out_filename);
	if (!_ofmt_ctx) {
		//printf("Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		EmideaClose();
		throw OpenException("Could not create output context");
	}
	_ofmt = _ofmt_ctx->oformat;

	findStream();						//throw

	//Open output file  

	if (!(_ofmt->flags & AVFMT_NOFILE)) {
		if (avio_open(&_ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE) < 0) {
			throw OpenException("Could not open output file");
			//printf("Could not open output file '%s'", out_filename);
		}
	}

	//Write file header  
	if (avformat_write_header(_ofmt_ctx, NULL) < 0) {
		//printf("Error occurred when opening output file\n");
		//return 1;
		throw  OpenException("Error occurred when opening output file");
	}

	writeFream(cur_pts_v, cur_pts_a);		//throw

	//Write file trailer  
	av_write_trailer(_ofmt_ctx);

	/*
	#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);
	#endif
	#if USE_AACBSF
	av_bitstream_filter_close(aacbsfc);
	#endif
	*/
	//end:
	EmideaClose();
	cout << "----end---------";
	return 0;
}

void Muxer::openInit()
{
	int ret = -1;
	const char *in_filename_v = _videoPath.c_str();
	const char *in_filename_a = _audioPath.c_str();

	av_register_all();								//初始化封装		
	if ((ret = avformat_open_input(&_ifmt_ctx_v, in_filename_v, 0, 0)) < 0) {
		char buf[512] = { 0 };						//存放错误内容
		av_strerror(ret, buf, sizeof(buf)-1);
		throw OpenException("Muxer::openInit()->avformat_open_input",buf);
	}
	if ((ret = avformat_find_stream_info(_ifmt_ctx_v, 0)) < 0) {
		throw OpenException("Muxer::openInit()->avformat_find_stream_info", _ifmt_ctx_v);
	}

	if ((ret = avformat_open_input(&_ifmt_ctx_a, in_filename_a, 0, 0)) < 0) {
		char buf[512] = { 0 };						//存放错误内容
		av_strerror(ret, buf, sizeof(buf)-1);
		throw OpenException("Muxer::openInit()->avformat_open_input", buf);
	}
	if ((ret = avformat_find_stream_info(_ifmt_ctx_a, 0)) < 0) {
		throw OpenException("Muxer::openInit()->avformat_find_stream_info", _ifmt_ctx_a);
	}
}

void Muxer::findStream()
{
	int i = 0; int ret = -1;
	for (i = 0; i < _ifmt_ctx_v->nb_streams; i++)
	{
		//Create output AVStream according to input AVStream  
		if (_ifmt_ctx_v->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			AVStream *in_stream = _ifmt_ctx_v->streams[i];
			AVStream *out_stream = avformat_new_stream(_ofmt_ctx, in_stream->codec->codec);
			_videoindex_v = i;
			if (!out_stream) {				
				ret = AVERROR_UNKNOWN;
				throw StreamExceptionPara("Muxer::findStream()->avformat_new_stream", _ofmt_ctx);
			}
			_videoindex_out = out_stream->index;
			//Copy the settings of AVCodecContext  
			if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {		
				throw StreamExceptionPara("Muxer::findStream()->avcodec_copy_context",in_stream->codec);
			}
			out_stream->codec->codec_tag = 0;
			if (_ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			break;
		}
	}

	for (i = 0; i < _ifmt_ctx_a->nb_streams; i++)
	{
		//Create output AVStream according to input AVStream  
		if (_ifmt_ctx_a->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			AVStream *in_stream = _ifmt_ctx_a->streams[i];
			AVStream *out_stream = avformat_new_stream(_ofmt_ctx, in_stream->codec->codec);
			_audioindex_a = i;
			if (!out_stream) {
				printf("Failed allocating output stream\n");
				ret = AVERROR_UNKNOWN;
				throw StreamExceptionPara("Muxer::findStream()->avformat_new_stream", _ofmt_ctx);
			}
			_audioindex_out = out_stream->index;
			//Copy the settings of AVCodecContext  
			if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
				//printf("Failed to copy context from input to output stream codec context\n");
				throw StreamExceptionPara("Muxer::findStream()->avcodec_copy_context", in_stream->codec);
			}
			out_stream->codec->codec_tag = 0;
			if (_ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

			break;
		}
	}
}

void Muxer::writeFream(int64_t& cur_pts_v, int64_t& cur_pts_a)
{
	//FIX  
	AVPacket pkt;
#if USE_H264BSF  
	AVBitStreamFilterContext* h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
#endif  
#if USE_AACBSF  
	AVBitStreamFilterContext* aacbsfc = av_bitstream_filter_init("aac_adtstoasc");
#endif  
	while (1)
	{
		AVFormatContext *ifmt_ctx;
		int stream_index = 0;
		AVStream *in_stream, *out_stream;

		//Get an AVPacket  
		if (av_compare_ts(cur_pts_v, _ifmt_ctx_v->streams[_videoindex_v]->time_base, cur_pts_a, _ifmt_ctx_a->streams[_audioindex_a]->time_base) <= 0)
		{
			ifmt_ctx = _ifmt_ctx_v;
			stream_index = _videoindex_out;

			if (av_read_frame(ifmt_ctx, &pkt) >= 0)
			{
				do
				{
					in_stream = ifmt_ctx->streams[pkt.stream_index];
					out_stream = _ofmt_ctx->streams[stream_index];

					if (pkt.stream_index == _videoindex_v){
						if (pkt.pts == AV_NOPTS_VALUE){
							//Write PTS  
							AVRational time_base1 = in_stream->time_base;
							//Duration between 2 frames (us)  
							int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
							//Parameters  
							pkt.pts = (double)(frame_index*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
							pkt.dts = pkt.pts;
							pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
							frame_index++;
						}

						cur_pts_v = pkt.pts;
						break;
					}
				} 
				while (av_read_frame(ifmt_ctx, &pkt) >= 0);
			}
			else{
				break;
			}
		}
		else
		{
			ifmt_ctx = _ifmt_ctx_a;
			stream_index = _audioindex_out;
			if (av_read_frame(ifmt_ctx, &pkt) >= 0){
				do{
					in_stream = ifmt_ctx->streams[pkt.stream_index];
					out_stream = _ofmt_ctx->streams[stream_index];

					if (pkt.stream_index == _audioindex_a){

						//FIX：No PTS  
						//Simple Write PTS  
						if (pkt.pts == AV_NOPTS_VALUE){
							//Write PTS  
							AVRational time_base1 = in_stream->time_base;
							//Duration between 2 frames (us)  
							int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
							//Parameters  
							pkt.pts = (double)(frame_index*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
							pkt.dts = pkt.pts;
							pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
							frame_index++;
						}
						cur_pts_a = pkt.pts;
						break;
					}
				} while (av_read_frame(ifmt_ctx, &pkt) >= 0);
			}
			else{
				break;
			}
		}

		//FIX:Bitstream Filter  
#if USE_H264BSF  
		av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif  
#if USE_AACBSF  
		av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif  

		//Convert PTS/DTS  
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index = stream_index;

		//printf("Write 1 Packet. size:%5d\tpts:%lld\n", pkt.size, pkt.pts);
		//Write  
		if (av_interleaved_write_frame(_ofmt_ctx, &pkt) < 0) {
			//printf("Error muxing packet\n");
			//break;
			av_packet_unref(&pkt);
			throw  WriteExceptionPara(_ofmt_ctx, &pkt);
		}
		av_packet_unref(&pkt);
	}

#if USE_H264BSF  
	av_bitstream_filter_close(h264bsfc);
#endif  
#if USE_AACBSF  
	av_bitstream_filter_close(aacbsfc);
#endif
}

