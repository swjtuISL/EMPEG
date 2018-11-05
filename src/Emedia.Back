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
	
	shared_ptr<Emedia> ptr = shared_ptr<Emedia>(new EmediaImpl(path));	
	
	ptr->_open_();			//throw
	return  ptr;
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
	_frame_index = -1;
}


Muxer::~Muxer(){
	avformat_close_input(&_ifmt_ctx_v);
	avformat_close_input(&_ifmt_ctx_a);
	/* close output */
	if (_ofmt_ctx && !(_ofmt->flags & AVFMT_NOFILE))
		avio_close(_ofmt_ctx->pb);
	avformat_free_context(_ofmt_ctx);
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
		ret = AVERROR_UNKNOWN;		
		throw OpenException("avformat_alloc_output_context2 error");
	}
	_ofmt = _ofmt_ctx->oformat;

	findStream();						//throw

	//Open output file  
	if (!(_ofmt->flags & AVFMT_NOFILE)) {
		if (avio_open(&_ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE) < 0) {
			throw OpenException("Could not open output file");			
		}
	}

	//Write file header  
	if (avformat_write_header(_ofmt_ctx, NULL) < 0) {		
		throw  OpenException("Error occurred when opening output file " + _mediaPath);
	}

	
	if (_audioPath.empty())
		writeFrame();
	else
		writeFrame(cur_pts_v, cur_pts_a);		//throw

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
	cout << "----combineVideoAudio end---------";
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
		throw OpenException("avformat_open_input error:" + std::string(buf));
	}
	if ((ret = avformat_find_stream_info(_ifmt_ctx_v, 0)) < 0) {
		throw OpenException("avformat_find_stream_info error;file is: " + _videoPath);
	}

	if (!(_audioPath.empty())){
		if ((ret = avformat_open_input(&_ifmt_ctx_a, in_filename_a, 0, 0)) < 0) {
			char buf[512] = { 0 };						//存放错误内容
			av_strerror(ret, buf, sizeof(buf)-1);
			throw OpenException("avformat_open_input error:" + std::string(buf));
		}
		if ((ret = avformat_find_stream_info(_ifmt_ctx_a, 0)) < 0) {
			throw OpenException("avformat_find_stream_info error;file is: " + _audioPath);
		}
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
				throw StreamExceptionPara("avformat_new_stream error", _ofmt_ctx);
			}
			_videoindex_out = out_stream->index;
			//Copy the settings of AVCodecContext  
			if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {		
				throw StreamExceptionPara("avcodec_copy_context",in_stream->codec);
			}
			out_stream->codec->codec_tag = 0;
			if (_ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			break;
		}
	}

	if (!_audioPath.empty()){
		for (i = 0; i < _ifmt_ctx_a->nb_streams; i++)
		{
			//Create output AVStream according to input AVStream  
			if (_ifmt_ctx_a->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				AVStream *in_stream = _ifmt_ctx_a->streams[i];
				AVStream *out_stream = avformat_new_stream(_ofmt_ctx, in_stream->codec->codec);
				_audioindex_a = i;
				if (!out_stream) {
					ret = AVERROR_UNKNOWN;
					throw StreamExceptionPara("findStream()->avformat_new_stream", _ofmt_ctx);
				}
				_audioindex_out = out_stream->index;
				//Copy the settings of AVCodecContext  
				if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
					throw StreamExceptionPara("avcodec_copy_context", in_stream->codec);
				}
				out_stream->codec->codec_tag = 0;
				if (_ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
					out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
				break;
			}
		}
	}
}

void Muxer::writeFrame(int64_t& cur_pts_v, int64_t& cur_pts_a)
{
	//FIX  
	AVPacket pkt;
	AVBitStreamFilterContext* h264bsfc;
	AVBitStreamFilterContext* aacbsfc;
	
	std::string fileType;
	fileType = (_mediaPath.substr(_mediaPath.find(".") + 1));	//获取文件类型
	if (fileType == "mp4" || fileType == "flv" || fileType == "mkv"){
		_isfilter=1;
		h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
		aacbsfc = av_bitstream_filter_init("aac_adtstoasc");
	}

/*
#if USE_H264BSF  
	AVBitStreamFilterContext* h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
#endif  
#if USE_AACBSF  
	AVBitStreamFilterContext* aacbsfc = av_bitstream_filter_init("aac_adtstoasc");
#endif  
*/
	while (1)
	{
		AVFormatContext *ifmt_ctx;
		int stream_index = 0;
		AVStream *in_stream, *out_stream;
		
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
							pkt.pts = (double)(_frame_index*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
							pkt.dts = pkt.pts;
							pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
							_frame_index++;
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
							pkt.pts = (double)(_frame_index*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
							pkt.dts = pkt.pts;
							pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
							_frame_index++;
						}
						cur_pts_a = pkt.pts;
						break;
					}
				} while (av_read_frame(ifmt_ctx, &pkt) >= 0);
			}
			else	break;			
		}

		//FIX:Bitstream Filter  
		if (_isfilter){
			av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);

			//--错误！！！！
			//av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
		}
/*
#if USE_H264BSF  
		av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif  
#if USE_AACBSF  
		av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif  
*/
		//Convert PTS/DTS  
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index = stream_index;

		//printf("Write 1 Packet. size:%5d\tpts:%lld\n", pkt.size, pkt.pts);
		//Write  
		if (av_interleaved_write_frame(_ofmt_ctx, &pkt) < 0) {			
			av_packet_unref(&pkt);
			throw  WriteExceptionPara("write frame frome AVFormatContext to AVPacket error",_ofmt_ctx, &pkt);
		}
		av_packet_unref(&pkt);
	}
	//if (_isfilter)
	av_bitstream_filter_close(h264bsfc);
	av_bitstream_filter_close(aacbsfc);
/*
#if USE_H264BSF  
	av_bitstream_filter_close(h264bsfc);
#endif  
#if USE_AACBSF  
	av_bitstream_filter_close(aacbsfc);
#endif
*/
}


//--只写入264文件
void Muxer::writeFrame(){		
	AVPacket pkt;
	AVBitStreamFilterContext* h264bsfc;
	AVBitStreamFilterContext* aacbsfc;

	std::string fileType;
	fileType = (_mediaPath.substr(_mediaPath.find(".") + 1));	//获取文件类型
	if (fileType == "mp4" || fileType == "flv" || fileType == "mkv"){
		_isfilter = 1;
		h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");		
	}

	while (1)
	{
		AVFormatContext *ifmt_ctx;
		int stream_index = 0;
		AVStream *in_stream, *out_stream;


		ifmt_ctx = _ifmt_ctx_v;
		stream_index = _videoindex_out;

		if (av_read_frame(ifmt_ctx, &pkt) >= 0)
		{
			do
			{
				in_stream = ifmt_ctx->streams[pkt.stream_index];
				out_stream = _ofmt_ctx->streams[stream_index];
				if (pkt.stream_index == _videoindex_v)
				{
					if (pkt.pts == AV_NOPTS_VALUE)
					{
						//Write PTS  
						AVRational time_base1 = in_stream->time_base;
						//Duration between 2 frames (us)  
						int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
						//Parameters  
						pkt.pts = (double)(_frame_index*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);
						pkt.dts = pkt.pts;
						pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);
						_frame_index++;
					}
					break;
				}
			} 
			while (av_read_frame(ifmt_ctx, &pkt) >= 0);
		}
		else break;

		if (_isfilter){
			av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);			
		}

		//Convert PTS/DTS  
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index = stream_index;

		//printf("Write 1 Packet. size:%5d\tpts:%lld\n", pkt.size, pkt.pts);
		//Write  
		if (av_interleaved_write_frame(_ofmt_ctx, &pkt) < 0) {
			av_packet_unref(&pkt);
			throw  WriteExceptionPara("write frame frome AVFormatContext to AVPacket error", _ofmt_ctx, &pkt);
		}
		av_packet_unref(&pkt);
	}
	if (_isfilter)
	av_bitstream_filter_close(h264bsfc);
}
