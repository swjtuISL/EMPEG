#ifndef __EMEDIA_IMPL_H
#define __EMEDIA_IMPL_H

#include "Emedia.h"
#include<hash_map>

#define USE_H264BSF 1			//'1': Use H.264 Bitstream Filter   
#define USE_AACBSF  0			//'1': Use AAC Bitstream Filter 

class EmediaImpl : public Emedia{
public:
	~EmediaImpl();
	EmediaImpl(const std::string& path);
	const std::string& where() override;	
	int high() override;
	int width() override;
	int64_t frames() override;
	double fps() override ;
	VideoType video_type() override;

	bool demuxer(const std::string& videoPath, const std::string& audioPath);
	bool xaudio (const std::string& path) override;
	bool xvideo (const std::string& path) override;
	bool xyuv   (const std::string& path) override;
	//bool combine(const std::string& videoPath, const std::string& audioPath, const std::string& mediaPath) override;
protected:
	bool _open_() override;
	bool _read_frame(AVPacket& pkt);
	bool _decode    (AVPacket* pkt, AVFrame& yuv);

	void creatStream();
	//bool _yuv_rgb();
private:
	std::string _filePath;								 //源文件
	std::string  _fileType;						 //文件类型(mp4..)
	
	AVFormatContext*   _formatCtx = nullptr;               //源文件格式
	AVFormatContext*   _ofmt_ctx_v = nullptr;              //输出视频格式
	AVFormatContext*   _ofmt_ctx_a = nullptr;              //输出音频格式
	AVOutputFormat*    _ofmt_v = nullptr;
	AVOutputFormat*    _ofmt_a = nullptr;
	AVCodecContext*    _encodecCtx = nullptr;
   
	int _videoStream; 
	int _audioStream;								 //音视频索引，读取时区分音视频
	int _flag;
	int _ret = 0;
    //static hash_map<AVCodecID, VideoType> _videoTypeMap;
};

#endif