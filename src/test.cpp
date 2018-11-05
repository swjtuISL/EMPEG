/*
#include <iostream>
#include <thread>
extern "C"{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}
using namespace std;
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"swscale.lib")

static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}
void XSleep(int ms)
{
	//c++ 11
	chrono::milliseconds du(ms);
	this_thread::sleep_for(du);
}
int main(int argc, char *argv[]){
	cout << "Test Demux FFmpeg.club" << endl;
	const char *path = "czl.mp4";

	av_register_all();		//初始化封装库
	
	avformat_network_init();
	avcodec_register_all();			//注册解码器
	AVDictionary *opts = NULL;		//参数设置
	//设置rtsp流已tcp协议打开
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	//网络延时时间
	av_dict_set(&opts, "max_delay", "500", 0);

	//-----------设置结束--------------------------------
	//解封装上下文
	AVFormatContext *ic = NULL;
	int re = avformat_open_input(
		&ic,
		path,
		0,		// 0表示自动选择解封器
		&opts	//参数设置，比如rtsp的延时时间
		);
	if (re != 0){
		char buf[1024] = { 0 };		//存放错误信息
		av_strerror(re, buf, sizeof(buf)-1);
		cout << "open " << path << " failed! :" << buf << endl;
		getchar(); return -1;
	}
	cout << "open " << path << " success! " << endl;

	//--------------------------------------------------	
	re = avformat_find_stream_info(ic, 0);				//获取流信息 
	
	int totalMs = ic->duration / (AV_TIME_BASE / 1000);//总时长 毫秒
	cout << "totalMs = " << totalMs << endl;

	av_dump_format(ic, 0, path, 0);				//打印视频流详细信息

	//音视频索引，读取时区分音视频
	int videoStream = 0; int audioStream = 1;

	//获取音视频流信息(方法1遍历，函数获取)，遍历stream获取音视频信息
	for (int i = 0; i < ic->nb_streams; i++)
	{
		AVStream *as = ic->streams[i];		//含视频流、音频流
		cout << "codec_id = " << as->codecpar->codec_id << endl;
		cout << "format = " << as->codecpar->format << endl;

		//音频 AVMEDIA_TYPE_AUDIO
		if (as->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
			audioStream = i;
			cout << i << "音频信息" << endl;
			cout << "sample_rate = " << as->codecpar->sample_rate << endl;
			//AVSampleFormat;
			cout << "channels = " << as->codecpar->channels << endl;
			//一帧数据？？ 单通道样本数 
			cout << "frame_size = " << as->codecpar->frame_size << endl;
			//1024 * 2 * 2 = 4096  fps = sample_rate/frame_size			
		}
		//视频 AVMEDIA_TYPE_VIDEO
		else if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			videoStream = i;
			cout << i << "视频信息" << endl;
			cout << "width=" << as->codecpar->width << endl;
			cout << "height=" << as->codecpar->height << endl;
			//帧率 fps 分数转换
			cout << "video fps = " << r2d(as->avg_frame_rate) << endl;
		}
	}

	//---获取视频流信息（方法2）
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

	//--------------------------------------------------------
	//找到视频解码器，视频解码器打开		
	AVCodec *vcodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);
	if (!vcodec){
		cout << "can't find the codec id " << ic->streams[videoStream]->codecpar->codec_id;
		getchar(); return -1;
	}
	cout << "find the AVCodec " << ic->streams[videoStream]->codecpar->codec_id << endl;

	AVCodecContext *vc = avcodec_alloc_context3(vcodec);	//vcodec代表解码器，为解码后的数据vc申请空间

	///配置解码器上下文参数
	avcodec_parameters_to_context(vc, ic->streams[videoStream]->codecpar);
	vc->thread_count = 8;		//八线程解码

	///打开解码器上下文
	re = avcodec_open2(vc, 0, 0);
	if (re != 0){
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf)-1);
		cout << "avcodec_open2  failed! :" << buf << endl; getchar(); return -1;
	}
	cout << "video avcodec_open2 success!" << endl;


	//--音频解码器打开
	//AVCodec *vcodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);//视频解码
	
	AVCodec *acodec = avcodec_find_decoder(ic->streams[audioStream]->codecpar->codec_id);
	if (!acodec){
		cout << "can't find the codec id " << ic->streams[audioStream]->codecpar->codec_id;
		getchar(); return -1;
	}
	cout << "find the AVCodec " << ic->streams[audioStream]->codecpar->codec_id << endl;
	///创建解码器上下文
	AVCodecContext *ac = avcodec_alloc_context3(acodec);

	///配置解码器上下文参数
	avcodec_parameters_to_context(ac, ic->streams[audioStream]->codecpar);
	ac->thread_count = 8;	//八线程解码

	//打开解码器上下文
	re = avcodec_open2(ac, 0, 0);
	if (re != 0){
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf)-1);
		cout << "avcodec_open2  failed! :" << buf << endl;
		getchar(); return -1;
	}
	cout << "audio avcodec_open2 success!" << endl;
	

	//--------------解码器设置完成----------------------------------------
	///ic->streams[videoStream]
	//malloc AVPacket并初始化
	AVPacket *pkt = av_packet_alloc();
	AVFrame *frame = av_frame_alloc();
	
	SwsContext *vctx = NULL;
	unsigned char *rgb = NULL;

	for (;;)
	{
		int re = av_read_frame(ic, pkt);	//读取stream中的一个frame,包含视频和音频

		if (pkt->stream_index != videoStream)	continue;	
		AVFrame*  pFrame = av_frame_alloc();		
		int getPicture = 0;
		
		re = avcodec_decode_video2(vc, pFrame, &getPicture, pkt);
		if (re < 0)
			break;
		if (!getPicture)
			break;



		if (re != 0){
			//循环播放
			cout << "=================end=================" << endl;
			int ms = 3000; //三秒位置 根据时间基数（分数）转换
			long long pos = (double)ms / (double)1000 * r2d(ic->streams[pkt->stream_index]->time_base);
			av_seek_frame(ic, videoStream, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
			continue;
		}
		cout << "pkt->size = " << pkt->size << endl;	//显示的时间
		cout << "pkt->pts = " << pkt->pts << endl;

		//转换为毫秒，方便做同步
		cout << "pkt->pts ms = " << pkt->pts * (r2d(ic->streams[pkt->stream_index]->time_base) * 1000) << endl;
		cout << "pkt->dts = " << pkt->dts << endl;		//解码时间

		AVCodecContext *cc = 0;
		if (pkt->stream_index == videoStream){
			cout << "图像" << endl; cc = vc;	//vc处理过吗？
		}
		if (pkt->stream_index == audioStream){
			cout << "音频" << endl; cc = ac;
		}

		//-----------------解码视频---------------
		//发送packet到解码线程  send传NULL后调用多次receive取出所有缓冲帧
		re = avcodec_send_packet(cc, pkt);		//不区分视频、音频
		av_packet_unref(pkt);					//释放，引用计数-1 为0释放空间，send之后pkt就没用了

		if (re != 0){
			char buf[1024] = { 0 };
			av_strerror(re, buf, sizeof(buf)-1);
			cout << "avcodec_send_packet  failed! :" << buf << endl;
			continue;
		}
		
		for (;;)
		{
			//从线程中获取解码接口,一次send可能对应多次receive
			re = avcodec_receive_frame(cc, frame);			//AVFrame *frame = av_frame_alloc()，获取解码结果	
			if (re != 0) break;
			
			if (cc == vc)
			{
				vctx = sws_getCachedContext(
					vctx,	//传NULL会新创建
					frame->width, frame->height,		//输入的宽高
					(AVPixelFormat)frame->format,	//输入格式 YUV420p
					frame->width, frame->height,	//输出的宽高
					AV_PIX_FMT_RGBA,				//输入格式RGBA
					SWS_BILINEAR,					//尺寸变化的算法
					0, 0, 0);
				//if(vctx)
				//cout << "像素格式尺寸转换上下文创建或者获取成功！" << endl;
				//else
				//	cout << "像素格式尺寸转换上下文创建或者获取失败！" << endl;
				if (vctx)
				{
					if (!rgb) rgb = new unsigned char[frame->width*frame->height * 4];
					uint8_t *data[2] = { 0 };
					data[0] = rgb;
					int lines[2] = { 0 };
					lines[0] = frame->width * 4;

					re = sws_scale(vctx,
						frame->data,		//输入数据
						frame->linesize,	//输入行大小
						0,
						frame->height,		//输入高度
						data,				//输出数据和大小
						lines
						);
					cout << "sws_scale = " << re << endl;
				}
			}
		}	//一次receive解码完成

		//XSleep(500);
	}	//for (;;)

	av_frame_free(&frame);
	av_packet_free(&pkt);

	if (ic){
		//释放封装上下文，并且把ic置0
		avformat_close_input(&ic);
	}

	getchar();
	return 0;
}
*/


/*

#include <iostream>
#include<fstream>
#include <thread>
extern "C"{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include<libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
using namespace std;
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib, "avutil.lib")

static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}
void XSleep(int ms)
{
	//c++ 11
	chrono::milliseconds du(ms);
	this_thread::sleep_for(du);
}
int main(int argc, char *argv[]){
	cout << "Test Demux FFmpeg.club" << endl;
	const char *path = "czl.mp4";

	av_register_all();		//初始化封装库

	//初始化网络库 （可以打开rtsp rtmp http 协议的流媒体视频）
	avformat_network_init();
	avcodec_register_all();			//注册解码器
	AVDictionary *opts = NULL;		//参数设置
	//设置rtsp流已tcp协议打开
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	//网络延时时间
	av_dict_set(&opts, "max_delay", "500", 0);

	//-----------设置结束--------------------------------
	//解封装上下文
	AVFormatContext *ic = NULL;
	int re = avformat_open_input(
		&ic,
		path,
		0,		// 0表示自动选择解封器
		&opts	//参数设置，比如rtsp的延时时间
		);
	if (re != 0){
		char buf[1024] = { 0 };		//存放错误信息
		av_strerror(re, buf, sizeof(buf)-1);
		cout << "open " << path << " failed! :" << buf << endl;
		getchar(); return -1;
	}
	cout << "open " << path << " success! " << endl;

	//--------------------------------------------------	
	re = avformat_find_stream_info(ic, 0);				//获取流信息 

	int totalMs = ic->duration / (AV_TIME_BASE / 1000);//总时长 毫秒
	cout << "totalMs = " << totalMs << endl;

	av_dump_format(ic, 0, path, 0);				//打印视频流详细信息

	int videoStream = -1; int audioStream = -1;

	//获取音视频流信息(方法1遍历，函数获取)，遍历stream获取音视频信息
	for (int i = 0; i < ic->nb_streams; i++)
	{
		AVStream *as = ic->streams[i];		//含视频流、音频流
		cout << "codec_id = " << as->codecpar->codec_id << endl;
		cout << "format = " << as->codecpar->format << endl;

		if (as->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
			audioStream = i;
			cout << i << "音频信息" << endl;
			cout << "sample_rate = " << as->codecpar->sample_rate << endl;
			//AVSampleFormat;
			cout << "channels = " << as->codecpar->channels << endl;
			//一帧数据？？ 单通道样本数 
			cout << "frame_size = " << as->codecpar->frame_size << endl;
			//1024 * 2 * 2 = 4096  fps = sample_rate/frame_size			
		}

		else if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			videoStream = i;
			cout << i << "视频信息" << endl;
			cout << "width=" << as->codecpar->width << endl;
			cout << "height=" << as->codecpar->height << endl;
			//帧率 fps 分数转换
			cout << "video fps = " << r2d(as->avg_frame_rate) << endl;
		}
	}

	//---获取视频流信息（方法2）
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

	//--------------------------------------------------------
	//找到视频解码器，视频解码器打开	
	AVCodec *vcodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);
	if (!vcodec){
		cout << "can't find the codec id " << ic->streams[videoStream]->codecpar->codec_id;
		getchar(); return -1;
	}
	cout << "find the AVCodec " << ic->streams[videoStream]->codecpar->codec_id << endl;

	AVCodecContext *vc = avcodec_alloc_context3(vcodec);	//vcodec代表解码器，为解码后的数据vc申请空间

	///配置解码器上下文参数
	avcodec_parameters_to_context(vc, ic->streams[videoStream]->codecpar);
	vc->thread_count = 8;		//八线程解码

	///打开解码器上下文
	re = avcodec_open2(vc, 0, 0);
	if (re != 0){
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf)-1);
		cout << "avcodec_open2  failed! :" << buf << endl; getchar(); return -1;
	}
	cout << "video avcodec_open2 success!" << endl;


	//--音频解码器打开
	//AVCodec *vcodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);//视频解码
	AVCodec *acodec = avcodec_find_decoder(ic->streams[audioStream]->codecpar->codec_id);
	if (!acodec){
		cout << "can't find the codec id " << ic->streams[audioStream]->codecpar->codec_id;
		getchar(); return -1;
	}
	cout << "find the AVCodec " << ic->streams[audioStream]->codecpar->codec_id << endl;
	///创建解码器上下文
	AVCodecContext *ac = avcodec_alloc_context3(acodec);

	///配置解码器上下文参数
	avcodec_parameters_to_context(ac, ic->streams[audioStream]->codecpar);
	ac->thread_count = 8;	//八线程解码

	///打开解码器上下文
	re = avcodec_open2(ac, 0, 0);
	if (re != 0){
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf)-1);
		cout << "avcodec_open2  failed! :" << buf << endl;
		getchar(); return -1;
	}
	cout << "audio avcodec_open2 success!" << endl;

	//--------------解码器设置完成----------------------------------------
	///ic->streams[videoStream]
	//malloc AVPacket并初始化
	AVPacket *pkt  = av_packet_alloc();
	AVFrame *frame = av_frame_alloc();
	ofstream ofile("yuvfile.yuv", ios::binary);
	if (!ofile){
		cerr << "open out file of yuv error\n";
		abort();
	}


	//设置格式
	AVFrame *pFrame, *pFrameYUV;
	pFrameYUV = av_frame_alloc();
	uint8_t *out_buffer;
	AVCodecContext*  pCodecCtx = ic->streams[videoStream]->codec;
	//	out_buffer = new uint8_t[avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height)];
	out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));

	//	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

	//--转换AV_PIX_FMT_YUV420P
	struct SwsContext *img_convert_ctx = NULL;
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	
	//---读取frame-----
	while (1)
	{
		int re = av_read_frame(ic, pkt);	//读取stream中的一个frame,包含视频和音频
		if (re != 0){
			//循环播放			
			int ms = 3000; //三秒位置 根据时间基数（分数）转换
			long long pos = (double)ms / (double)1000 * r2d(ic->streams[pkt->stream_index]->time_base);
			av_seek_frame(ic, videoStream, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
			//continue;
			break;
		}
		cout << "pkt->size = " << pkt->size << endl;	//显示的时间
		cout << "pkt->pts = " << pkt->pts << endl;

		//转换为毫秒，方便做同步
		cout << "pkt->pts ms = " << pkt->pts * (r2d(ic->streams[pkt->stream_index]->time_base) * 1000) << endl;
		cout << "pkt->dts = " << pkt->dts << endl;		//解码时间

		AVCodecContext *cc = 0;
		if (pkt->stream_index == videoStream){
			cout << "图像" << endl; cc = vc;	//vc处理过吗？
		}
		if (pkt->stream_index == audioStream){
			cout << "音频" << endl; cc = ac;
			continue;	//不对音频处理
		}

		//-----------------解码视频---------------
		//发送packet到解码线程  send传NULL后调用多次receive取出所有缓冲帧
		/*
		re = avcodec_send_packet(cc, pkt);			//不区分视频、音频
		av_packet_unref(pkt);						//释放，引用计数-1 为0释放空间，send之后pkt就没用了
		if (re != 0){
			char buf[1024] = { 0 };
			av_strerror(re, buf, sizeof(buf)-1);
			cout << "avcodec_send_packet  failed! :" << buf << endl;
			continue;
		}
		for (;;){
			//从线程中获取解码接口,一次send可能对应多次receive
			re = avcodec_receive_frame(cc, frame);			//AVFrame *frame = av_frame_alloc()，获取解码结果	
			if (re != 0) break;
			cout << "recv frame " << frame->format << " " << frame->linesize[0] << endl;
		}	///

		if (pkt->stream_index != videoStream)	continue;
		pFrame = av_frame_alloc();
		int getPicture = 0;
		re = avcodec_decode_video2(vc, pFrame, &getPicture, pkt);
		if (re < 0)
			break;
		if (!getPicture)
			continue;

		//-将解码后的frame以YUV240的格式写入文件		
		sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

		//写YUV
		//fwrite(pFrameYUV->data[0], (pCodecCtx->width)*(pCodecCtx->height), 1, output);
		//fwrite(pFrameYUV->data[1], (pCodecCtx->width)*(pCodecCtx->height) / 4, 1, output);
		//fwrite(pFrameYUV->data[2], (pCodecCtx->width)*(pCodecCtx->height) / 4, 1, output);

		ofile.write((char*)pFrameYUV->data[0], (pCodecCtx->width)*(pCodecCtx->height));
		ofile.write((char*)pFrameYUV->data[1], (pCodecCtx->width)*(pCodecCtx->height) / 4);
		ofile.write((char*)pFrameYUV->data[2], (pCodecCtx->width)*(pCodecCtx->height) / 4);
		av_packet_free(&pkt);
		//写RGB  
		//fwrite(pFrameYUV->data[0], (pCodecCtx->width)*(pCodecCtx->height) * 3, 1, output);
		//XSleep(500);

	}	

	while (1)
	{
		if (pkt->stream_index != videoStream)	continue;
		pFrame = av_frame_alloc();
		int getPicture = 0;
		re = avcodec_decode_video2(vc, pFrame, &getPicture, pkt);
		if (re < 0)
			break;
		if (!getPicture)
			break;
		sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
		
		ofile.write((char*)pFrameYUV->data[0], (pCodecCtx->width)*(pCodecCtx->height));
		ofile.write((char*)pFrameYUV->data[1], (pCodecCtx->width)*(pCodecCtx->height) / 4);
		ofile.write((char*)pFrameYUV->data[2], (pCodecCtx->width)*(pCodecCtx->height) / 4);
		//av_packet_free(&pkt);
	}

	av_frame_free(&frame);
	av_packet_free(&pkt);
	ofile.close();
	if (ic){
		//释放封装上下文，并且把ic置0
		avformat_close_input(&ic);
	}

	getchar();
	return 0;
}

*/