#include"Emedia.h"
#include"EmediaImpl.h"
#include<iostream>
using namespace std;

shared_ptr<Emedia> Emedia::generate(const string& path){
	//EmediaImpl *p = new EmediaImpl(path);
	shared_ptr<Emedia> ptr = shared_ptr<Emedia>(new EmediaImpl(path));

	try{
		ptr->__open__();
	}
	catch (EmediaException& reson){
		cout << reson.what() << endl;
		cin.get();
		return NULL;
		//---
	}
	return  ptr;
}

//--指针处理
static void EmideaClose(AVFormatContext* ifmt_ctx_v, AVFormatContext* ifmt_ctx_a, AVFormatContext* ofmt_ctx, AVOutputFormat* ofmt)
{
	avformat_close_input(&ifmt_ctx_v);
	avformat_close_input(&ifmt_ctx_a);
	/* close output */
	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);
	int ret = -1;
	if (ret < 0 && ret != AVERROR_EOF) {
		printf("Error occurred.\n");
		//return -1;
	}
}
/*
static void func1(const string& videoPath, const string& audioPath, AVFormatContext* ifmt_ctx_v, AVFormatContext* ifmt_ctx_a)
{
	const char *in_filename_v = videoPath.c_str();
	const char *in_filename_a = audioPath.c_str();


	int ret = -1;
	if ((ret = avformat_open_input(&ifmt_ctx_v, in_filename_v, 0, 0)) < 0) {
		cout << "Could not open input file.\n";
		EmideaClose(ifmt_ctx_v, ifmt_ctx_a, ofmt_ctx, ofmt); return 1;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx_v, 0)) < 0) {
		cout << "Failed to retrieve input stream information\n";
		EmideaClose(ifmt_ctx_v, ifmt_ctx_a, ofmt_ctx, ofmt); return 1;
	}

	if ((ret = avformat_open_input(&ifmt_ctx_a, in_filename_a, 0, 0)) < 0) {
		cout << "Could not open input file.\n";
		EmideaClose(ifmt_ctx_v, ifmt_ctx_a, ofmt_ctx, ofmt); return 1;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx_a, 0)) < 0) {
		cout << "Failed to retrieve input stream information\n";
		EmideaClose(ifmt_ctx_v, ifmt_ctx_a, ofmt_ctx, ofmt); return 1;
	}



}

bool Emedia::combine(const string& videoPath, const string& audioPath, const string& mediaPath){
	AVOutputFormat *ofmt = NULL;
	//Input AVFormatContext and Output AVFormatContext  
	AVFormatContext *ifmt_ctx_v = NULL, *ifmt_ctx_a = NULL, *ofmt_ctx = NULL;
	AVPacket pkt;
	int ret, i;
	int videoindex_v = -1, videoindex_out = -1;
	int audioindex_a = -1, audioindex_out = -1;
	int frame_index = 0;
	int64_t cur_pts_v = 0, cur_pts_a = 0;
	//const char *in_filename_v = videoPath.c_str();
	//const char *in_filename_a = audioPath.c_str();
	const char *out_filename = mediaPath.c_str();//Output file URL  

	av_register_all();

	//--指针处理
	func1(videoPath, audioPath, ifmt_ctx_v, ifmt_ctx_a);	//Input  
}
*/