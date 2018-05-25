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


static void getFileType(string& filename){
	int loc = filename.find(".");
	string fileType(filename.substr(loc + 1));
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

static void openInit(const string& videoPath, const string& audioPath, AVFormatContext* ifmt_ctx_v, AVFormatContext* ifmt_ctx_a)
{
	const char *in_filename_v = videoPath.c_str();
	const char *in_filename_a = audioPath.c_str();


	int ret = -1;
	if ((ret = avformat_open_input(&ifmt_ctx_v, in_filename_v, 0, 0)) < 0) {
		throw Emedia::EmediaException("Failed to retrieve input stream information");
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx_v, 0)) < 0) {
		throw Emedia::EmediaException("Failed to retrieve input stream information");
	}

	if ((ret = avformat_open_input(&ifmt_ctx_a, in_filename_a, 0, 0)) < 0) {
		//cout << "Could not open input file.\n";
		throw Emedia::EmediaException("Failed to retrieve input stream information");
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx_a, 0)) < 0) {
		//cout << "Failed to retrieve input stream information\n";
		throw Emedia::EmediaException("Failed to retrieve input stream information");
	}



}

static void findStream(AVFormatContext* ifmt_ctx_v, AVFormatContext* ifmt_ctx_a, AVFormatContext* ofmt_ctx,
	int& videoindex_v, int& audioindex_a, int& videoindex_out, int& audioindex_out)
{
	int i = 0; int ret = -1;
	for (i = 0; i < ifmt_ctx_v->nb_streams; i++)
	{
		//Create output AVStream according to input AVStream  
		if (ifmt_ctx_v->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
			AVStream *in_stream = ifmt_ctx_v->streams[i];
			AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
			videoindex_v = i;
			if (!out_stream) {
				printf("Failed allocating output stream\n");
				ret = AVERROR_UNKNOWN;
				throw Emedia::EmediaException("Failed allocating output stream");
			}
			videoindex_out = out_stream->index;
			//Copy the settings of AVCodecContext  
			if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
				printf("Failed to copy context from input to output stream codec context\n");
				throw Emedia::EmediaException("Failed to copy context from input to output stream codec context");
			}
			out_stream->codec->codec_tag = 0;
			if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			break;
		}
	}

	for (i = 0; i < ifmt_ctx_a->nb_streams; i++)
	{
		//Create output AVStream according to input AVStream  
		if (ifmt_ctx_a->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
			AVStream *in_stream = ifmt_ctx_a->streams[i];
			AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
			audioindex_a = i;
			if (!out_stream) {
				printf("Failed allocating output stream\n");
				ret = AVERROR_UNKNOWN;
				throw Emedia::EmediaException("Failed allocating output stream");
			}
			audioindex_out = out_stream->index;
			//Copy the settings of AVCodecContext  
			if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
				printf("Failed to copy context from input to output stream codec context\n");
				throw Emedia::EmediaException("Failed to copy context from input to output stream codec context");
			}
			out_stream->codec->codec_tag = 0;
			if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

			break;
		}
	}
}


static void writeFream(int64_t& cur_pts_v, int64_t& cur_pts_a, AVFormatContext* ifmt_ctx_v, AVFormatContext* ifmt_ctx_a, AVFormatContext* ofmt_ctx,
	int& videoindex_v, int& audioindex_a, int& videoindex_out, int& audioindex_out, int& frame_index)
{

	/*	int videoindex_v = -1, videoindex_out = -1;
	int audioindex_a = -1, audioindex_out = -1;
	int frame_index = 0;
	int64_t cur_pts_v = 0, cur_pts_a = 0;
	*/
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
		if (av_compare_ts(cur_pts_v, ifmt_ctx_v->streams[videoindex_v]->time_base, cur_pts_a, ifmt_ctx_a->streams[audioindex_a]->time_base) <= 0)
		{
			ifmt_ctx = ifmt_ctx_v;
			stream_index = videoindex_out;

			if (av_read_frame(ifmt_ctx, &pkt) >= 0){
				do{
					in_stream = ifmt_ctx->streams[pkt.stream_index];
					out_stream = ofmt_ctx->streams[stream_index];

					if (pkt.stream_index == videoindex_v){
						//FIX：No PTS (Example: Raw H.264)  
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

						cur_pts_v = pkt.pts;
						break;
					}
				} while (av_read_frame(ifmt_ctx, &pkt) >= 0);
			}
			else{
				break;
			}
		}
		else
		{
			ifmt_ctx = ifmt_ctx_a;
			stream_index = audioindex_out;
			if (av_read_frame(ifmt_ctx, &pkt) >= 0){
				do{
					in_stream = ifmt_ctx->streams[pkt.stream_index];
					out_stream = ofmt_ctx->streams[stream_index];

					if (pkt.stream_index == audioindex_a){

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

		printf("Write 1 Packet. size:%5d\tpts:%lld\n", pkt.size, pkt.pts);
		//Write  
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			//printf("Error muxing packet\n");
			//break;
			av_free_packet(&pkt);
			throw Emedia::EmediaException("Error muxing packet by call func3");
		}
		av_free_packet(&pkt);

	}

#if USE_H264BSF  
	av_bitstream_filter_close(h264bsfc);
#endif  
#if USE_AACBSF  
	av_bitstream_filter_close(aacbsfc);
#endif
}


bool Emedia::combine(const string& videoPath, const string& audioPath, const string& mediaPath){


	AVOutputFormat *ofmt = NULL;
	//Input AVFormatContext and Output AVFormatContext  
	AVFormatContext *ifmt_ctx_v = NULL, *ifmt_ctx_a = NULL, *ofmt_ctx = NULL;
	//AVPacket pkt;
	int ret;
	int videoindex_v = -1, videoindex_out = -1;
	int audioindex_a = -1, audioindex_out = -1;
	int frame_index = 0;
	int64_t cur_pts_v = 0, cur_pts_a = 0;

	const char *out_filename = mediaPath.c_str();//Output file URL  

	av_register_all();

	//--指针处理
	try{
		openInit(videoPath, audioPath, ifmt_ctx_v, ifmt_ctx_a);	//Input  
	}
	catch (Emedia::EmediaException){
		EmideaClose(ifmt_ctx_v, ifmt_ctx_a, ofmt_ctx, ofmt); throw ret;
	}

	//Output  
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
	if (!ofmt_ctx) {
		printf("Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		EmideaClose(ifmt_ctx_v, ifmt_ctx_a, ofmt_ctx, ofmt); throw ret;
	}
	ofmt = ofmt_ctx->oformat;

	try{
		findStream(ifmt_ctx_v, ifmt_ctx_a, ofmt_ctx, videoindex_v, audioindex_a, videoindex_out, audioindex_out);
	}
	catch (Emedia::EmediaException){
		EmideaClose(ifmt_ctx_v, ifmt_ctx_a, ofmt_ctx, ofmt); throw 1;
	}

	//Open output file  
	try{
		if (!(ofmt->flags & AVFMT_NOFILE)) {
			if (avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE) < 0) {
				throw Emedia::EmediaException("Could not open output file");
				//printf("Could not open output file '%s'", out_filename);
			}
		}
	}
	catch (Emedia::EmediaException){
		EmideaClose(ifmt_ctx_v, ifmt_ctx_a, ofmt_ctx, ofmt); return 1;
	}

	//Write file header  
	try{
		if (avformat_write_header(ofmt_ctx, NULL) < 0) {
			//printf("Error occurred when opening output file\n");
			//return 1;
			throw Emedia::EmediaException("Error occurred when opening output file");
		}
	}
	catch (Emedia::EmediaException){
		EmideaClose(ifmt_ctx_v, ifmt_ctx_a, ofmt_ctx, ofmt); return 1;
	}


	try{
		writeFream(cur_pts_v, cur_pts_a, ifmt_ctx_v, ifmt_ctx_a, ofmt_ctx,
		videoindex_v, audioindex_a, videoindex_out, audioindex_out, frame_index);
	}
	catch (Emedia::EmediaException){
		throw Emedia::EmediaException("error by call combine");
	}


	//Write file trailer  
	av_write_trailer(ofmt_ctx);

	/*
	#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);
	#endif
	#if USE_AACBSF
	av_bitstream_filter_close(aacbsfc);
	#endif
	*/
	//end:
	EmideaClose(ifmt_ctx_v, ifmt_ctx_a, ofmt_ctx, ofmt);

	cout << "----end---------";
	return 0;
}

