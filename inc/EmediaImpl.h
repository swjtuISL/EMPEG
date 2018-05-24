#ifndef __EMEDIA_IMPL_H
#define __EMEDIA_IMPL_H


#define USE_H264BSF 1	//'1': Use H.264 Bitstream Filter   
#define USE_AACBSF  0	//'1': Use AAC Bitstream Filter   

#include "Emedia.h"
#include<hash_map>
extern "C"{
#include"libavformat/avformat.h"
#include"libavcodec/avcodec.h"
#include<libswscale/swscale.h>
#include"libavutil/imgutils.h"
}

class EmediaImpl : public Emedia{
public:
	EmediaImpl(const std::string& path);
	const std::string& where() override;	
	int high() override;
	int width() override;
	int64_t frames() override;
	double fps() override ;
	VideoType video_type() override;

	bool demuxer(const std::string& videoPath, const std::string& audioPath);
	bool xaudio(const std::string& path) override;
	bool xvideo(const std::string& path) override;
	bool xyuv(const std::string& path) override;
	//bool combine(const std::string& videoPath, const std::string& audioPath, const std::string& mediaPath) override;
protected:
	bool __open__() override;
	bool _read_frame(AVPacket& pkt);
	bool _decode(AVPacket* pkt, AVFrame& yuv);
	//bool _yuv_rgb();
private:
	std::string __filePath;
	AVFormatContext* _formatCtx = NULL;
	
	int __videoStream; 
	int __audioStream;		//音视频索引，读取时区分音视频
	int _flag;

//    hash_map<AVCodecID, VideoType> _videoTypeMap;
};

#endif