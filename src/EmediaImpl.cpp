#include "EmediaImpl.h"
#include"EmpegException.h"
#include<iostream>
#include<fstream>
#include<hash_map>
using namespace std;
//--ffmpeg库和头文件
#ifndef _FFMPEG_H
#define _FFMPEG_H
extern "C"{
#include"libavformat/avformat.h"
#include"libavcodec/avcodec.h"
#include<libswscale/swscale.h>
#include"libavutil/imgutils.h"
}
#endif


EmediaImpl::EmediaImpl(const std::string& path){
	_filePath = path;
	_videoStream = -1;
	_audioStream = -1;
	_flag = -1;
	//formatCtx->filename
	//hash_map<AVCodecID, VideoType> _videoTypeMap;
	/*_videoTypeMap[AV_CODEC_ID_H264] = H264;
	_videoTypeMap[AV_CODEC_ID_MPEG4] = MPEG4;
	_videoTypeMap[AV_CODEC_ID_JPEG2000] = JPEG2000;*/
}

//检查打开是否成功
bool EmediaImpl::_open_(){
	av_register_all();								//初始化封装												
	avformat_network_init();						//初始化网络库 （可以打开rtsp rtmp http 协议的流媒体视频）
	avcodec_register_all();							//注册解码器
	AVDictionary *opts = NULL;						//参数设置
	av_dict_set(&opts, "rtsp_transport", "tcp", 0); //设置rtsp流已tcp协议打开
	av_dict_set(&opts, "max_delay", "500", 0);		//网络延时时间

	const char *pathTemp = _filePath.c_str();
	_flag = avformat_open_input(&_formatCtx, pathTemp, 0, &opts);

	if (_flag != 0){
		char buf[1024] = { 0 };		//存放错误信息
		av_strerror(_flag, buf, sizeof(buf)-1);
		throw OpenException("EmediaImpl::_open_()->avformat_open_input",buf);
	}

	if (avformat_find_stream_info(_formatCtx, 0) < 0){
		//cout<<"Failed to retrieve input stream information\n";
		//return false;
		throw OpenException("find stream fail call avformat_find_stream_info");
	}

	//--找视频流、音频流标准
	_videoStream = av_find_best_stream(_formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	_audioStream = av_find_best_stream(_formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

	//--音视频解码器创建及打开
	for (int i = 0; i < _formatCtx->nb_streams; i++)
	{
		AVCodecContext *enc = _formatCtx->streams[i]->codec;
		if (enc->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			AVCodec *codec = avcodec_find_decoder(enc->codec_id);
			if (!codec){
				//cout << "video code not find\n";
				//return false;
				throw OpenException("find decoder call avcodec_find_decoder");
			}
			int err = avcodec_open2(enc, codec, NULL);
			if (err != 0){
				char buf[1024] = { 0 };
				av_strerror(err, buf, sizeof(buf));
				//cout << buf << endl;	return 0;
				throw OpenException(buf);
			}
			cout << "open codec success by call XFFmpeg::open function\n";
		}
	}
	return true;
}


void EmediaImpl::func1(){
	AVFormatContext *ofmt_ctx;
	AVStream *in_stream = _formatCtx->streams[_videoStream];
	AVStream *out_stream = NULL;

	if (_formatCtx->streams[_videoStream]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO){
		throw OpenException("error _videoStream!=AVMEDIA_TYPE_VIDEO by  call xvideo in ");
	}
	out_stream = avformat_new_stream(_ofmt_ctx_v, in_stream->codec->codec);
	ofmt_ctx = _ofmt_ctx_v;

	if (!out_stream) {
		printf("Failed allocating output stream\n");
		_ret = AVERROR_UNKNOWN;
		//goto end;
		throw;
	}
	//Copy the settings of AVCodecContext
	if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
		printf("Failed to copy context from input to output stream codec context\n");
		//goto end;
		throw;
	}
	out_stream->codec->codec_tag = 0;
	if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
}

bool EmediaImpl::xvideo(const std::string& path){
	AVPacket pkt;
	int i = 0;
	int frame_index = 0;								//读取的packet个数
	//Output---ofmt_ctx_v存输出文件格式
	const char* out_filename_v = path.c_str();

	avformat_alloc_output_context2(&_ofmt_ctx_v, NULL, NULL, out_filename_v);
	if (!_ofmt_ctx_v) {
		printf("Could not create output context\n");
		_ret = AVERROR_UNKNOWN;
		//goto end;
		throw std::string("ssdas");
	}
	_ofmt_v = _ofmt_ctx_v->oformat;

	//--------------------------------------------------------------------	
	func1();

	//Open output file
	if (!(_ofmt_v->flags & AVFMT_NOFILE)) {
		if (avio_open(&_ofmt_ctx_v->pb, out_filename_v, AVIO_FLAG_WRITE) < 0) {
			printf("Could not open output file '%s'", out_filename_v);
			throw;
		}
	}

	//Write file header,有问题
	if (avformat_write_header(_ofmt_ctx_v, NULL) < 0) {
		printf("Error occurred when opening video output file\n");
		throw;
	}

#if USE_H264BSF
	AVBitStreamFilterContext* h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
#endif

	while (1)
	{
		AVFormatContext *ofmt_ctx;
		AVStream *in_stream, *out_stream;

		if (av_read_frame(_formatCtx, &pkt) < 0)
			break;
		in_stream = _formatCtx->streams[pkt.stream_index];

		if (pkt.stream_index == _videoStream){
			out_stream = _ofmt_ctx_v->streams[_videoStream];
			ofmt_ctx   = _ofmt_ctx_v;
			printf("Write Video Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
#if USE_H264BSF
			av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
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
			printf("Error muxing packet\n");
			//return false;
			throw;
		}

		av_packet_unref(&pkt);
		frame_index++;
	}

#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);
#endif
	//Write file trailer
	av_write_trailer(_ofmt_ctx_v);
	return true;
}

//----提取音频和视频
bool EmediaImpl::demuxer(const std::string& videoPath, const std::string& audioPath){
	AVOutputFormat *ofmt_a = NULL, *ofmt_v = NULL;
	//（Input AVFormatContext and Output AVFormatContext）
	AVFormatContext *ofmt_ctx_a = NULL, *ofmt_ctx_v = NULL;
	AVPacket pkt;
	int ret = 0, i = 0;
	int videoindex = -1, audioindex = -1;
	int frame_index = 0;

	//Output---ofmt_ctx_v存输出文件格式
	const char* out_filename_v = videoPath.c_str();
	const char* out_filename_a = audioPath.c_str();
	avformat_alloc_output_context2(&ofmt_ctx_v, NULL, NULL, out_filename_v);
	if (!ofmt_ctx_v) {
		printf("Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt_v = ofmt_ctx_v->oformat;

	avformat_alloc_output_context2(&ofmt_ctx_a, NULL, NULL, out_filename_a);
	if (!ofmt_ctx_a) {
		printf("Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt_a = ofmt_ctx_a->oformat;
	//--------------------------------------------------------------------	
	for (i = 0; i < _formatCtx->nb_streams; i++)
	{
		//Create output AVStream according to input AVStream		
		AVFormatContext *ofmt_ctx;
		AVStream *in_stream = _formatCtx->streams[i];
		AVStream *out_stream = NULL;
		if (_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			videoindex = i;
			out_stream = avformat_new_stream(ofmt_ctx_v, in_stream->codec->codec);
			ofmt_ctx = ofmt_ctx_v;
		}
		else if (_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
			audioindex = i;
			out_stream = avformat_new_stream(ofmt_ctx_a, in_stream->codec->codec);
			ofmt_ctx = ofmt_ctx_a;
		}
		else	break;

		if (!out_stream) {
			printf("Failed allocating output stream\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}
		//Copy the settings of AVCodecContext
		if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
			printf("Failed to copy context from input to output stream codec context\n");
			goto end;
		}
		out_stream->codec->codec_tag = 0;

		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}

	//Open output file
	if (!(ofmt_v->flags & AVFMT_NOFILE)) {
		if (avio_open(&ofmt_ctx_v->pb, out_filename_v, AVIO_FLAG_WRITE) < 0) {
			printf("Could not open output file '%s'", out_filename_v);
			goto end;
		}
	}

	if (!(ofmt_a->flags & AVFMT_NOFILE)) {
		if (avio_open(&ofmt_ctx_a->pb, out_filename_a, AVIO_FLAG_WRITE) < 0) {
			printf("Could not open output file '%s'", out_filename_a);
			goto end;
		}
	}

	//Write file header,有问题
	if (avformat_write_header(ofmt_ctx_v, NULL) < 0) {
		printf("Error occurred when opening video output file\n");
		goto end;
	}
	if (avformat_write_header(ofmt_ctx_a, NULL) < 0) {
		printf("Error occurred when opening audio output file\n");
		goto end;
	}
#if USE_H264BSF
	AVBitStreamFilterContext* h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
#endif
	while (1)
	{
		AVFormatContext *ofmt_ctx;
		AVStream *in_stream, *out_stream;
		//Get an AVPacket
		if (av_read_frame(_formatCtx, &pkt) < 0)	break;
		in_stream = _formatCtx->streams[pkt.stream_index];

		if (pkt.stream_index == videoindex){
			out_stream = ofmt_ctx_v->streams[0];
			ofmt_ctx = ofmt_ctx_v;
			printf("Write Video Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
#if USE_H264BSF
			av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
		}
		else if (pkt.stream_index == audioindex){
			out_stream = ofmt_ctx_a->streams[0];
			ofmt_ctx = ofmt_ctx_a;
			printf("Write Audio Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
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
			printf("Error muxing packet\n");
			break;
		}
		//printf("Write %8d frames to output file\n",frame_index);
		av_packet_unref(&pkt);
		frame_index++;
	}

#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);
#endif

	//Write file trailer
	av_write_trailer(ofmt_ctx_a);
	av_write_trailer(ofmt_ctx_v);
end:
	avformat_close_input(&_formatCtx);
	/* close output */
	if (ofmt_ctx_a && !(ofmt_a->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx_a->pb);

	if (ofmt_ctx_v && !(ofmt_v->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx_v->pb);

	avformat_free_context(ofmt_ctx_a);
	avformat_free_context(ofmt_ctx_v);

	if (ret < 0 && ret != AVERROR_EOF) {
		printf("Error occurred.\n");
		return false;
	}
	return true;
}

bool EmediaImpl::xaudio(const std::string& path){
	AVOutputFormat*  ofmt_a = NULL;
	AVFormatContext* ofmt_ctx_a = NULL;
	AVPacket pkt;
	int ret = 0, i = 0;
	int frame_index = 0;						//统计读取的packet的个数

	//Output---ofmt_ctx_v存输出文件格式
	const char* out_filename_a = path.c_str();

	avformat_alloc_output_context2(&ofmt_ctx_a, NULL, NULL, out_filename_a);
	if (!ofmt_ctx_a) {
		printf("Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		//goto end;
		throw;
	}
	ofmt_a = ofmt_ctx_a->oformat;
	//--------------------------------------------------------------------	

	//Create output AVStream according to input AVStream		
	AVFormatContext *ofmt_ctx;
	AVStream *in_stream = _formatCtx->streams[_audioStream];
	AVStream *out_stream = NULL;

	if (_formatCtx->streams[_audioStream]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
		out_stream = avformat_new_stream(ofmt_ctx_a, in_stream->codec->codec);
		ofmt_ctx = ofmt_ctx_a;
	}

	if (!out_stream) {
		printf("Failed allocating output stream\n");
		ret = AVERROR_UNKNOWN;
		throw;
	}
	//Copy the settings of AVCodecContext
	if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
		printf("Failed to copy context from input to output stream codec context\n");
		throw;
	}
	out_stream->codec->codec_tag = 0;

	if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

	if (!(ofmt_a->flags & AVFMT_NOFILE)) {
		if (avio_open(&ofmt_ctx_a->pb, out_filename_a, AVIO_FLAG_WRITE) < 0) {
			printf("Could not open output file '%s'", out_filename_a);
			throw;
		}
	}

	//Write file header,有问题
	if (avformat_write_header(ofmt_ctx_a, NULL) < 0) {
		printf("Error occurred when opening audio output file\n");
		throw;
	}

	while (1)
	{
		//AVFormatContext *ofmt_ctx;
		//AVStream *in_stream, *out_stream;
		if (av_read_frame(_formatCtx, &pkt) < 0)
			break;
		in_stream = _formatCtx->streams[pkt.stream_index];

		if (pkt.stream_index == _audioStream){
			out_stream = ofmt_ctx_a->streams[_audioStream];
			ofmt_ctx = ofmt_ctx_a;
			printf("Write Audio Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
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
			printf("Error muxing packet\n");
			//break;
			throw;
		}

		av_packet_unref(&pkt);
		frame_index++;
	}

	av_write_trailer(ofmt_ctx_a);

	//avformat_close_input(&_formatCtx);
	/* close output */

	try{}
	catch (...){
		cout << "error by call xaudio\n";
	}
	if (ofmt_ctx_a && !(ofmt_a->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx_a->pb);

	avformat_free_context(ofmt_ctx_a);

	if (ret < 0 && ret != AVERROR_EOF) {
		printf("Error occurred.\n");
		return false;
	}
	cout << "-----xaudio end--------\n";
	return true;
}

bool EmediaImpl::_read_frame(AVPacket& pkt){
	char errorbuf[512] = { 0 };
	int err = av_read_frame(_formatCtx, &pkt);
	if (err != 0){
		av_strerror(err, errorbuf, sizeof(errorbuf));
		throw OpenException(errorbuf);
	}
	//memset(pkt, 0, sizeof(AVPacket));
	return true;
}

//----解码一个packet
bool EmediaImpl::_decode(AVPacket* pkt, AVFrame& yuv){
	_formatCtx->streams[pkt->stream_index]->codecpar;
	int re = avcodec_send_packet(_formatCtx->streams[pkt->stream_index]->codec, pkt);	//涉及解码器
	if (re != 0){
		//cout << "error in avcodec_send_packet\n";
		//return false;
		throw OpenException("error in avcodec_send_packet");
	}
	re = avcodec_receive_frame(_formatCtx->streams[pkt->stream_index]->codec, &yuv);
	if (re != 0){
		//cout << "error in avcodec_receive_frame\n";
		//return false;
		throw OpenException("error in avcodec_send_packet");
	}
	return true;
}

//--获取yuv
bool EmediaImpl::xyuv(const std::string& path){
	//-将解码后的frame以YUV240的格式写入文件
	//	AVFrame* pFrame;
	ofstream ofile(path, ios::binary);	//yuv文件
	if (!ofile){
		cerr << "open out file of yuv error\n";
		abort();
	}

	AVFrame* pFrameYUV = av_frame_alloc();
	uint8_t* out_buffer;
	AVCodecContext* pCodecCtx = _formatCtx->streams[_videoStream]->codec;
	out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

	//设置格式
	//out_buffer = new uint8_t[avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height)];
	//avpicture_fill( (AVPicture *)pFrameYUV, out_buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

	//--转换
	struct SwsContext *img_convert_ctx = NULL;
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	while (1)
	{
		AVPacket* pkt = av_packet_alloc();
		AVFrame* frame = av_frame_alloc();

		//_read_frame(*pkt);
		int err = av_read_frame(_formatCtx, pkt);
		if (pkt->size == 0){
			cout << "------------读完全部的pkt---------\n";
			break;
		}
		//不是视频packet,*********************************
		if (pkt->stream_index != _videoStream){
			av_packet_unref(pkt);	//释放空间 
			continue;
		}

		EmediaImpl::_decode(pkt, *frame);
		av_packet_unref(pkt);	//释放空间 

		//-------------------------------------------------------------------
		sws_scale(img_convert_ctx, (const uint8_t* const*)frame->data, frame->linesize, 0,
			pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
		//写YUV
		ofile.write((char*)pFrameYUV->data[0], (pCodecCtx->width)*(pCodecCtx->height));
		ofile.write((char*)pFrameYUV->data[1], (pCodecCtx->width)*(pCodecCtx->height) / 4);
		ofile.write((char*)pFrameYUV->data[2], (pCodecCtx->width)*(pCodecCtx->height) / 4);
		cout << "--------------write----------------\n";
	}

	ofile.clear();
	return true;
}


EmediaImpl::~EmediaImpl(){
	avformat_close_input(&_formatCtx);
	/* close output */
	if (_ofmt_ctx_v && !(_ofmt_v->flags & AVFMT_NOFILE))
		avio_close(_ofmt_ctx_v->pb);
	avformat_free_context(_ofmt_ctx_v);

	if (_ret < 0 && _ret != AVERROR_EOF) {
		printf("Error occurred.\n");
	}
	cout << "---------xvideo end-----\n";
}

















// 只读函数
const string& EmediaImpl::where(){
	return _filePath;
}

int EmediaImpl::high(){
	return _formatCtx->streams[_videoStream]->codecpar->height;
}
int EmediaImpl::width(){
	return _formatCtx->streams[_videoStream]->codecpar->width;
}

int64_t EmediaImpl::frames(){
	return _formatCtx->streams[_videoStream]->nb_frames;
}

double EmediaImpl::fps(){
	AVRational R = _formatCtx->streams[_videoStream]->avg_frame_rate;
	return R.num == 0 | R.den == 0 ? 0.0 : (double)R.num / (double)R.den;
}

VideoType EmediaImpl::video_type(){
	//if (_videoTypeMap.find(_formatCtx->streams[_videoStream]->codecpar->codec_id) != _videoTypeMap.end)
	//	return _videoTypeMap[_formatCtx->streams[_videoStream]->codecpar->codec_id];
	//else
	return NONE;
}