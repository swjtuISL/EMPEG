#include "EmediaImpl.h"
#include"EmpegException.h"
#include<iostream>
#include<fstream>
#include<hash_map>
#include<windows.h>

#include <stdint.h>
#include<time.h>
using namespace std;
//--ffmpeg????????????????????????????
#ifndef _FFMPEG_H
#define _FFMPEG_H
extern "C"{
#include"libavformat/avformat.h"
#include"libavcodec/avcodec.h"
#include<libswscale/swscale.h>
#include"libavutil/imgutils.h"
}
#endif

int av_usleep(unsigned usec)
{
#if HAVE_NANOSLEEP
	struct timespec ts = { usec / 1000000, usec % 1000000 * 1000 };
	while (nanosleep(&ts, &ts) < 0 && errno == EINTR);
	return 0;
#elif HAVE_USLEEP
	return usleep(usec);
#elif HAVE_SLEEP
	Sleep(usec / 1000);
	return 0;
#else
	return AVERROR(ENOSYS);
#endif
}

EmediaImpl::EmediaImpl(const std::string& path){
	av_register_all();								//?????????????????????????????????												
	avformat_network_init();						//?????????????????????????????????????????????????????????????????????????rtsp rtmp http ??????????????????????????????????????????????????????????
	avcodec_register_all();							//??????????????????????????????????

	_filePath = path;
	_videoStream = -1;
	_audioStream = -1;
	_flag = -1;
	//formatCtx->filename
	
	_fileType = (path.substr(path.find(".") + 1));	//??????????????????????????????????????????

	/*hash_map<AVCodecID, VideoType> _videoTypeMap;
	_videoTypeMap[AV_CODEC_ID_H264] = H264;
	_videoTypeMap[AV_CODEC_ID_MPEG4] = MPEG4;
	_videoTypeMap[AV_CODEC_ID_JPEG2000] = JPEG2000;*/
}

//?????????????????????????????????????
bool EmediaImpl::_open_(){	
	AVDictionary *opts = NULL;						//????????????????????????????????????
	av_dict_set(&opts, "rtsp_transport", "tcp", 0); //??????????????????rtsp??????????????????tcp????????????????
	av_dict_set(&opts, "max_delay", "500", 0);		//??????????????????????????????????????????

	const char *pathTemp = _filePath.c_str();
	_flag = avformat_open_input(&_formatCtx, pathTemp, 0, &opts);
	if (_flag != 0){
		char buf[512] = { 0 };						//?????????????????????????????????????	
		av_strerror(_flag, buf, sizeof(buf)-1);		
		throw OpenException("avformat_open_input error:"+std::string(buf));	
	}

	if (avformat_find_stream_info(_formatCtx, 0) < 0){
		throw OpenException("avformat_find_stream_info error;file is: " + _filePath);
	}
	
	//--????????????????????????????????????????????????????????????????????????
	_videoStream = av_find_best_stream(_formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	_audioStream = av_find_best_stream(_formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if ((_formatCtx->nb_streams>1) && (_videoStream + _audioStream<1))	throw StreamExceptionPara("avformat_find_stream_info error file is:" + _filePath);

	
	return true;
}

void EmediaImpl::_openFormatCtx(){
	AVDictionary *opts = NULL;						//????????????????????????????????????
	av_dict_set(&opts, "rtsp_transport", "tcp", 0); //??????????????????rtsp??????????????????tcp????????????????
	av_dict_set(&opts, "max_delay", "500", 0);		//??????????????????????????????????????????

	const char *pathTemp = _filePath.c_str();
	_flag = avformat_open_input(&_formatCtx, pathTemp, 0, &opts);

	if (_flag != 0){
		char buf[1024] = { 0 };		//?????????????????????????????????????
		av_strerror(_flag, buf, sizeof(buf)-1);
		throw OpenException("EmediaImpl::_open_()->avformat_open_input"+std::string( buf));
	}

	if (avformat_find_stream_info(_formatCtx, 0) < 0){
		throw OpenException("find stream fail call avformat_find_stream_info frome the file " + _filePath);
	}
}

void EmediaImpl::creatStream(){
	AVFormatContext *ofmt_ctx;
	AVStream		*in_stream = _formatCtx->streams[_videoStream];
	AVStream		*out_stream = nullptr;

	if (_formatCtx->streams[_videoStream]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO){
		throw OpenException("not find videoStream in " + _filePath);
	}
	out_stream = avformat_new_stream(_ofmt_ctx_v, in_stream->codec->codec);
	ofmt_ctx = _ofmt_ctx_v;

	if (!out_stream) {		
		_ret = AVERROR_UNKNOWN;		
		throw StreamExceptionPara("creat out_stream call avformat_new_stream fail", out_stream);
	}
	//Copy the settings of AVCodecContext
	if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
		std::cout<<"Failed to copy context from input to output stream codec context\n";		
		throw StreamExceptionPara("copy out_stream call avcodec_copy_context fail", out_stream);
	}
	out_stream->codec->codec_tag = 0;
	if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
}

bool EmediaImpl::xvideo(const std::string& path,bool isDebug){
	std::string outFileType = (path.substr(path.find(".") + 1));	//??????????????????????????????????????????
	/*if ( outFileType != "h264" )
		throw OpenException("output file error", path);*/
	
	AVStream		*in_stream, *out_stream;
	AVPacket		pkt;
	AVBitStreamFilterContext* h264bsfc = nullptr;
	int		 i = 0;
	int		 frame_index = 0;								//?????????????????????packet??????????????????
	//Output---ofmt_ctx_v??????????????????????????????????????????????
	const char* out_filename_v = path.c_str();

	if (!_formatCtx){
		_openFormatCtx();
	}
	
	AVCodec *vcodec = avcodec_find_decoder(_formatCtx->streams[_videoStream]->codecpar->codec_id);
	avformat_alloc_output_context2(&_ofmt_ctx_v, NULL, vcodec->name, NULL);			// vcodec->name
	if (!_ofmt_ctx_v)
		avformat_alloc_output_context2(&_ofmt_ctx_v, NULL, NULL, out_filename_v);			// vcodec->name
	if (!_ofmt_ctx_v)
	{
		std::cout<<"Could not create output context\n";
		_ret = AVERROR_UNKNOWN;		
		throw OpenException("call avformat_alloc_output_context2 error", _ofmt_ctx_v);
	}
	_ofmt_v = _ofmt_ctx_v->oformat;

	//----------------------------------------------
	creatStream();

	//Open output file
	if (!(_ofmt_v->flags & AVFMT_NOFILE)) {
		if (avio_open(&_ofmt_ctx_v->pb, out_filename_v, AVIO_FLAG_WRITE) < 0) {
			std::cout << "Could not open output file" << out_filename_v<<std::endl;
			throw OpenException(out_filename_v);
		}
	}

	//Write file header,???????????????????????????,	_ofmt_ctx_v?????????????????????????
	if (avformat_write_header(_ofmt_ctx_v, NULL) < 0) {
		std::cout<<"Error occurred when opening video output file\n";
		throw WriteExceptionPara("avformat_write_header fail", _ofmt_ctx_v);
	}

//#if USE_H264BSF
	//mp4?????????flv?????????????????????????????????????????????
	
	if (_fileType == "mp4" || _fileType == "flv")
		h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
//#endif


	while (1)
	{
		if (av_read_frame(_formatCtx, &pkt) < 0)
			break;
		in_stream = _formatCtx->streams[pkt.stream_index];

		if (pkt.stream_index == _videoStream){
			//out_stream = _ofmt_ctx_v->streams[_videoStream];
			out_stream = _ofmt_ctx_v->streams[0];
			if (isDebug)	std::cout << "Write Video Packet. size:" << pkt.size << "   pkt.pts:" << pkt.pts << std::endl;			
//#if USE_H264BSF
			if (_fileType == "mp4" || _fileType == "flv")
				av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
//#endif
		}
		else	continue;

		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index = 0;
		//Write
		if (av_interleaved_write_frame(_ofmt_ctx_v, &pkt) < 0) {
			std::cout<<"Error muxing packet\n";			
			throw WriteExceptionPara(_ofmt_ctx_v, &pkt);
		}
		
		av_packet_unref(&pkt);
		frame_index++;
	}

//#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);
//#endif
	//Write file trailer
	av_write_trailer(_ofmt_ctx_v);
	
	avformat_close_input(&_formatCtx);
	//av_seek_frame(_formatCtx, _videoStream, _formatCtx->streams[_videoStream]->start_time, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	//av_seek_frame(_formatCtx, _audioStream, _formatCtx->streams[_audioStream]->start_time, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	return true;
}

//----?????????????????????????????????????????????
bool EmediaImpl::demuxer(const std::string& videoPath, const std::string& audioPath,bool isDebug){		
	AVPacket pkt;
	int ret = 0, i = 0;
	int frame_index = 0;

	AVFormatContext *ofmt_ctx;
	AVStream *in_stream = nullptr;
	AVStream *out_stream = nullptr;
	//Output---ofmt_ctx_v??????????????????????????????????????????????
	const char* out_filename_v = videoPath.c_str();
	const char* out_filename_a = audioPath.c_str();
	AVBitStreamFilterContext* h264bsfc = nullptr;

	if (!_formatCtx){
		_openFormatCtx();
	}
	avformat_alloc_output_context2(&_ofmt_ctx_v, NULL, NULL, out_filename_v);
	if (!_ofmt_ctx_v) {
		std::cout<<"Could not create output context\n";
		ret = AVERROR_UNKNOWN;		
		throw OpenException("call avformat_alloc_output_context2 error", _ofmt_ctx_v);
	}
	_ofmt_v = _ofmt_ctx_v->oformat;

	avformat_alloc_output_context2(&_ofmt_ctx_a, NULL, NULL, out_filename_a);
	if (!_ofmt_ctx_a) {
		std::cout<<"Could not create output context\n";
		ret = AVERROR_UNKNOWN;
		throw OpenException("call avformat_alloc_output_context2 error", _ofmt_ctx_a);
	}
	_ofmt_a = _ofmt_ctx_a->oformat;

	//--------------------------------------------------------------------		
	for (i = 0; i < _formatCtx->nb_streams; i++)
	{
		in_stream = _formatCtx->streams[i];
		if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			out_stream = avformat_new_stream(_ofmt_ctx_v, in_stream->codec->codec);
			ofmt_ctx = _ofmt_ctx_v;
		}
		else if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
			out_stream = avformat_new_stream(_ofmt_ctx_a, in_stream->codec->codec);
			ofmt_ctx = _ofmt_ctx_a;
		}
		else	break;

		if (!out_stream) {
			std::cout<<"Failed allocating output stream\n";
			ret = AVERROR_UNKNOWN;
			throw StreamExceptionPara("call avformat_new_stream error", out_stream);
		}
		//Copy the settings of AVCodecContext
		if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
			std::cout<<"Failed to copy context from input to output stream codec context\n";
			throw StreamExceptionPara("call avcodec_copy_context error", out_stream);
		}
		out_stream->codec->codec_tag = 0;

		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}

	//Open output file
	if (!(_ofmt_v->flags & AVFMT_NOFILE)) {
		if (avio_open(&_ofmt_ctx_v->pb, out_filename_v, AVIO_FLAG_WRITE) < 0) {
			std::cout<<"Could not open output file\n";
			throw OpenException("call avio_open error", out_filename_v);
		}
	}
	if (!(_ofmt_a->flags & AVFMT_NOFILE)) {
		if (avio_open(&_ofmt_ctx_a->pb, out_filename_a, AVIO_FLAG_WRITE) < 0) {
			std::cout << "Could not open output file\n";
			throw OpenException("call avio_open error", out_filename_a);
		}
	}

	//Write file header,???????????????????????????
	if (avformat_write_header(_ofmt_ctx_v, NULL) < 0) {		
		throw WriteExceptionPara("call avformat_write_header fail", _ofmt_ctx_v);
	}
	if (avformat_write_header(_ofmt_ctx_a, NULL) < 0) {		
		throw WriteExceptionPara("call avformat_write_header fail", _ofmt_ctx_a);
	}
	
	if (_fileType == "mp4" || _fileType == "flv")
		h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
	while (1)
	{
		//AVFormatContext *ofmt_ctx;
		//AVStream *in_stream, *out_stream;
		
		if (av_read_frame(_formatCtx, &pkt) < 0)	break;
		in_stream = _formatCtx->streams[pkt.stream_index];

		if (pkt.stream_index == _videoStream)
		{
			out_stream = _ofmt_ctx_v->streams[0];
			ofmt_ctx = _ofmt_ctx_v;
			printf("Write Video Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
			if (_fileType == "mp4" || _fileType == "flv")
				av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
		}
		else if (pkt.stream_index == _audioStream){
			out_stream = _ofmt_ctx_a->streams[0];
			ofmt_ctx = _ofmt_ctx_a;
			if (isDebug)	std::cout << "Write Audio Packet. size:" << pkt.size << "   pkt.pts:" << pkt.pts << std::endl;			
			//printf("Write Audio Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
		}
		else	continue;

		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index = 0;
		//Write
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			std::cout << "Error muxing packet\n";
			throw WriteExceptionPara(ofmt_ctx, &pkt);
		}
		//printf("Write %8d frames to output file\n",frame_index);
		av_packet_unref(&pkt);
		frame_index++;
	}

	av_bitstream_filter_close(h264bsfc);
	
	av_write_trailer(_ofmt_ctx_a);
	av_write_trailer(_ofmt_ctx_v);
//end:	
	//?????????????????????????????????????????????????????????????????????
	/*if (_ofmt_ctx_a && !(_ofmt_a->flags & AVFMT_NOFILE))
		avio_close(_ofmt_ctx_a->pb);
	if (_ofmt_ctx_v && !(_ofmt_v->flags & AVFMT_NOFILE))
		avio_close(_ofmt_ctx_v->pb);
	avformat_free_context(_ofmt_ctx_a);
	avformat_free_context(_ofmt_ctx_v);*/

	if (ret < 0 && ret != AVERROR_EOF) {
		std::cout<<"Error occurred.\n";
		return false;
	}	

	avformat_close_input(&_formatCtx);
	//av_seek_frame(ic, videoStream, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	//av_seek_frame(_formatCtx, _videoStream, _formatCtx->streams[_videoStream]->start_time, 0);
	//av_seek_frame(_formatCtx, _audioStream, _formatCtx->streams[_audioStream]->start_time, 0);
	return true;
}


bool EmediaImpl::xaudio(const std::string& path, bool isDebug){
	std::string outFileType = (path.substr(path.find(".") + 1));	//??????????????????????????????????????????,???????????????????????????????????????????
	/*if (outFileType != "aac")
		throw OpenException("output file error", path);*/

	//AVOutputFormat*  ofmt_a = nullptr;
	//AVFormatContext* ofmt_ctx_a = nullptr;
	AVPacket pkt;
	
	AVFormatContext* ofmt_ctx = nullptr;
	AVStream* in_stream  = nullptr;
	AVStream* out_stream = nullptr;

	int ret = 0, i = 0;
	int frame_index = 0;						//???????????????????????????packet?????????????????????
	//Output---ofmt_ctx_v??????????????????????????????????????????????
	const char* out_filename_a = path.c_str();

	//--?????????????????????_formatCtx
	if (!_formatCtx){
		_openFormatCtx();
	}	

	in_stream = _formatCtx->streams[_audioStream];

	AVCodec *vcodec = avcodec_find_decoder(_formatCtx->streams[_audioStream]->codecpar->codec_id);
	//avformat_alloc_output_context2???????????????????????????????????????????????????????????????????????????VFormatContext??????????????????		vcodec->name
	avformat_alloc_output_context2(&_ofmt_ctx_a, NULL, NULL, out_filename_a);
	//avformat_alloc_output_context2(&_ofmt_ctx_a, NULL, NULL, out_filename_a);
	if (!_ofmt_ctx_a) {
		//std::cout<<"Could not create output context\n";
		ret = AVERROR_UNKNOWN;
		throw OpenException("call avformat_alloc_output_context2 error", _ofmt_ctx_a);
	}
	_ofmt_a = _ofmt_ctx_a->oformat;
	//--------------------------------------------------------------------	

	//Create output AVStream according to input AVStream		
	

	if (_formatCtx->streams[_audioStream]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
		out_stream = avformat_new_stream(_ofmt_ctx_a, in_stream->codec->codec);
		ofmt_ctx = _ofmt_ctx_a;
	}

	if (!out_stream) {
		std::cout<<"Failed allocating output stream\n";
		ret = AVERROR_UNKNOWN;
		throw StreamExceptionPara("creat out_stream error", out_stream);
	}
	//Copy the settings of AVCodecContext
	if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
		std::cout<<"Failed to copy context from input to output stream codec context\n";
		throw StreamExceptionPara("call avcodec_copy_context error", out_stream->codec);
	}
	out_stream->codec->codec_tag = 0;

	if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

	if (!(_ofmt_a->flags & AVFMT_NOFILE)) {
		if (avio_open(&_ofmt_ctx_a->pb, out_filename_a, AVIO_FLAG_WRITE) < 0) {
			throw OpenException("call avio_open error", _ofmt_ctx_a);
		}
	}

	//Write file header,???????????????????????????
	if (avformat_write_header(_ofmt_ctx_a, NULL) < 0) {
		printf("Error occurred when opening audio output file\n");
		throw WriteExceptionPara("call avformat_write_header error", _ofmt_ctx_a);;
	}

	while (1)
	{
		//AVFormatContext *ofmt_ctx;
		//AVStream *in_stream, *out_stream;
		if (av_read_frame(_formatCtx, &pkt) < 0)
			break;
		in_stream = _formatCtx->streams[pkt.stream_index];

		if (pkt.stream_index == _audioStream){
			//out_stream = ofmt_ctx_a->streams[_audioStream];
			out_stream = _ofmt_ctx_a->streams[0];
			ofmt_ctx = _ofmt_ctx_a;
			
			if (isDebug)	std::cout << "pkt.size:" << pkt.size << "  pkt.pts: " << pkt.pts << std::endl;			
		}
		else continue;

		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index = 0;
		//Write
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {						
			throw WriteExceptionPara(ofmt_ctx, &pkt);
		}

		av_packet_unref(&pkt);
		frame_index++;
	}

	av_write_trailer(_ofmt_ctx_a);

	/*if (_ofmt_ctx_a && !(_ofmt_a->flags & AVFMT_NOFILE))
		avio_close(_ofmt_ctx_a->pb);

	avformat_free_context(_ofmt_ctx_a);*/

	if (ret < 0 && ret != AVERROR_EOF) {
		if(isDebug)	std::cout<<"Error occurred.\n";
		return false;
	}

	avformat_close_input(&_formatCtx);
	//av_seek_frame(_formatCtx, _videoStream, _formatCtx->streams[_videoStream]->start_time, 0);
	//av_seek_frame(_formatCtx, _audioStream, _formatCtx->streams[_audioStream]->start_time, 0);
	if(isDebug)	cout << "-----xaudio end--------\n";
	return true;
}

//--??????yuv
int EmediaImpl::xyuv(const std::string& path,bool isDebug){
	//-???????????????frame???YUV240?????????????????????	
	ofstream ofile(path, ios::binary);	//yuv??????
	int numFrame = 0;					//?????????????????????
	if (!ofile){
		throw OpenException("open file error call ofstream ofile", path);
	}
	
	if (!_formatCtx){
		_openFormatCtx();
	}

	//--????????????????????????????????????????????????????????????????????????????????????
	for (int i = 0; i < _formatCtx->nb_streams; i++)
	{
		_encodecCtx = _formatCtx->streams[i]->codec;
		if (_encodecCtx->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			AVCodec *codec = avcodec_find_decoder(_encodecCtx->codec_id);
			if (!codec){				
				throw OpenException("find decoder call avcodec_find_decoder", codec);
			}
			int err = avcodec_open2(_encodecCtx, codec, NULL);
			if (err != 0){
				char buf[1024] = { 0 };
				av_strerror(err, buf, sizeof(buf));				
				throw OpenException(buf);
			}			
		}
	}

	AVFrame* pFrameYUV = av_frame_alloc();
	uint8_t* out_buffer;
	AVCodecContext* pCodecCtx = _formatCtx->streams[_videoStream]->codec;
	int dstW = floor(pCodecCtx->width / 16) * 16;
	int dstH = floor(pCodecCtx->height / 16) * 16;	
	//out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
	//av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
	out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, dstW, dstH, 1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, dstW, dstH, 1);

	//????????????????????????
	//out_buffer = new uint8_t[avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height)];
	//avpicture_fill( (AVPicture *)pFrameYUV, out_buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

	//--??????
	struct SwsContext *img_convert_ctx = NULL;	
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, dstW, dstH, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	
	//--??????frame????????????
	while (1)
	{
		AVPacket* pkt = av_packet_alloc();		
		AVFrame* frame = av_frame_alloc();
		av_parser_parse2;
		int err = av_read_frame(_formatCtx, pkt);
		//????????????
		/*
		if (pkt->size == 0)
		{
			while (true){
				int re = avcodec_send_packet(_encodecCtx, pkt);
				if (re != 0) break;
				av_packet_unref(pkt);
				while (true)
				{
					re = avcodec_receive_frame(_formatCtx->streams[pkt->stream_index]->codec, frame);			//#define EAGAIN       11						
					if (re == AVERROR(EAGAIN)){
						re = 0;
						if (isDebug)	std::cout << "call avcodec_receive_frame return AVERROR(EAGAIN))\n";
						//avcodec_flush_buffers(_formatCtx->streams[pkt->stream_index]->codec);								
						break;
					}
					if (re != 0)	break;
					sws_scale(img_convert_ctx, (const uint8_t* const*)frame->data, frame->linesize, 0,
						pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
					//???YUV
					ofile.write((char*)pFrameYUV->data[0], dstW*dstH);
					ofile.write((char*)pFrameYUV->data[1], dstW*dstH / 4);
					ofile.write((char*)pFrameYUV->data[2], dstW*dstH / 4);
					++numFrame;
					if (isDebug)	std::cout << numFrame << std::endl;
				}
			}
			av_packet_unref(pkt);	//???????????? 		
			Sleep(25);

			if (isDebug)	std::cout << "------------???????????????pkt---------\n";	break;
		}
		*/

		//????????????packet,*********************************
		if (pkt->stream_index != _videoStream){
			av_packet_unref(pkt);				//?????????????????? 
			continue;
		}		
		//int re = avcodec_send_packet(_formatCtx->streams[pkt->stream_index]->codec, pkt);	//bug
		int re = avcodec_send_packet(_encodecCtx, pkt);								
		if (re != 0){
			char buf[512] = { 0 };
			av_strerror(re, buf, sizeof(buf)-1);
			if(isDebug)		std::cout << "avcodec_send_packet error! :" << buf << std::endl;
			continue;
		}		
		
		//??????????????????????????????,????????????send???????????????????????????????????????eceive
		while(true)		
		{			
			//avcodec_receive_packet
			//re = avcodec_receive_frame(_formatCtx->streams[pkt->stream_index]->codec, frame);			//#define EAGAIN       11
			re = avcodec_receive_frame(_encodecCtx, frame);
			if (re == AVERROR(EAGAIN)){
				//ff_yield();
				//continue;
				//av_usleep(100);
				//Sleep(10000);
				re = 0;
				if (isDebug)	std::cout << "call avcodec_receive_frame return AVERROR(EAGAIN))\n";
				//avcodec_flush_buffers(_formatCtx->streams[pkt->stream_index]->codec);								
				break;
			}
			if (re != 0)	break;			
			sws_scale(img_convert_ctx, (const uint8_t* const*)frame->data, frame->linesize, 0,
				pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
			
			//???YUV			
			ofile.write((char*)pFrameYUV->data[0], dstW*dstH);
			ofile.write((char*)pFrameYUV->data[1], dstW*dstH / 4);
			ofile.write((char*)pFrameYUV->data[2], dstW*dstH / 4);
			++numFrame;
			if (isDebug)	std::cout << numFrame << std::endl;
		}		
		av_packet_unref(pkt);	//???????????? 		
		Sleep(25);
	}	
	ofile.clear();
	avformat_close_input(&_formatCtx);
	//av_seek_frame(_formatCtx, _videoStream, _formatCtx->streams[_videoStream]->start_time, 0);
	//av_seek_frame(_formatCtx, _audioStream, _formatCtx->streams[_audioStream]->start_time, 0);
	return numFrame;
}

bool EmediaImpl::_read_frame(AVPacket& pkt){
	char errorbuf[512] = { 0 };
	int err = av_read_frame(_formatCtx, &pkt);
	if (err != 0){
		av_strerror(err, errorbuf, sizeof(errorbuf));
		throw ParamExceptionPara(errorbuf);
	}
	//memset(pkt, 0, sizeof(AVPacket));
	return true;
}

//----??????????????????????????????packet
bool EmediaImpl::_decode(AVPacket* pkt, AVFrame& yuv){	
	_formatCtx->streams[pkt->stream_index]->codecpar;
	int re = avcodec_send_packet(_formatCtx->streams[pkt->stream_index]->codec, pkt);	//????????????????????????????????????
	if (re != 0){		
		throw DecodeExceptionPara("avcodec_send_packet error");
	}
	re = avcodec_receive_frame(_formatCtx->streams[pkt->stream_index]->codec, &yuv);
	if (re != 0){
		//cout << "error in avcodec_receive_frame\n";
		//return false;
		throw DecodeExceptionPara("avcodec_receive_frame error");
	}
	return true;
}


EmediaImpl::~EmediaImpl(){
	if (_ofmt_ctx_v && !(_ofmt_v->flags & AVFMT_NOFILE))
		avio_close(_ofmt_ctx_v->pb);

	if (_ofmt_ctx_a && !(_ofmt_a->flags & AVFMT_NOFILE))
		avio_close(_ofmt_ctx_a->pb);

	avformat_free_context(_ofmt_ctx_v);
	avformat_free_context(_ofmt_ctx_a);
	avformat_close_input(&_formatCtx);
}

// ??????????????????????????????
const string& EmediaImpl::where(){	
	return _filePath;
}

int EmediaImpl::high(){
	if (!_formatCtx){
		_openFormatCtx();
	}
	return _formatCtx->streams[_videoStream]->codecpar->height;
}
int EmediaImpl::width(){
	if (!_formatCtx){
		_openFormatCtx();
	}
	return _formatCtx->streams[_videoStream]->codecpar->width;
}

int64_t EmediaImpl::frames(){
	if (!_formatCtx){
		_openFormatCtx();
	}
	
	//nb_frames???????????????????????????????????????????????????
	int64_t frame_t = (fps()*(_formatCtx->duration / AV_TIME_BASE));
	return _formatCtx->streams[_videoStream]->nb_frames != 0 ? _formatCtx->streams[_videoStream]->nb_frames:(fps()*(_formatCtx->duration / AV_TIME_BASE));
}

double EmediaImpl::fps(){
	if (!_formatCtx){
		_openFormatCtx();
	}		
	AVRational R = _formatCtx->streams[_videoStream]->avg_frame_rate;	
	return R.num == 0 | R.den == 0 ? 0.0 : (double)R.num / (double)R.den;
}

EmediaImpl::VideoType EmediaImpl::video_type(){	
	/*if (_videoTypeMap.find(_formatCtx->streams[_videoStream]->codecpar->codec_id) != _videoTypeMap.end)
		return _videoTypeMap[_formatCtx->streams[_videoStream]->codecpar->codec_id];
	else*/

	if (!_formatCtx){
		_openFormatCtx();
	}
	return NONE;
}

bool EmediaImpl::audio_exists(){
	if (!_formatCtx){
		_openFormatCtx();
	}
	return _audioStream > -1 ? true : false;
}