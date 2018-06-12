#ifndef __EMEDIA_H
#define __EMEDIA_H
#include <string>
#include <memory>
/*
enum VideoType{
	NONE,	//无
	H264,
	MPEG4,
	JPEG2000
};
*/
#ifndef _FFMPEG_H
#define _FFMPEG_H
extern "C"{
#include"libavformat/avformat.h"
#include"libavcodec/avcodec.h"
#include<libswscale/swscale.h>
#include"libavutil/imgutils.h"
}
#endif

//-----------------class Muxer----------------------------
class Muxer{
public:
	Muxer(const std::string& videoPath, const std::string& audioPath, const std::string& mediaPath);
	Muxer();
	~Muxer();
	void openInit();
	void findStream();
	void Muxer::writeFrame(int64_t& cur_pts_v, int64_t& cur_pts_a);
	void Muxer::writeFrame();
	Muxer::Muxer(const std::string& videoPath, const std::string& mediaPath);
	bool combineVideoAudio();

private:
	const std::string _videoPath;
	const std::string _audioPath;
	const std::string _mediaPath;

	AVFormatContext* _ifmt_ctx_v =nullptr;
	AVFormatContext* _ifmt_ctx_a = nullptr;
	AVFormatContext* _ofmt_ctx   = nullptr;

	AVOutputFormat* _ofmt;

	int _videoindex_v;
	int _audioindex_a;
	int _videoindex_out;
	int _audioindex_out;

	int _frame_index;

	bool _isfilter=0;
};

//--------------class Emedia-------------------------------
class Emedia{
public:
	enum VideoType{NONE, H264, MPEG4, JPEG2000};
	virtual ~Emedia(){};
	virtual const     std::string& where() = 0;				// 视频所在的绝对路径
	virtual int       high()   = 0;							// 视频文件高
	virtual int       width()  = 0;							// 视频文件宽
	virtual int64_t   frames() = 0;							// 视频帧数
	virtual double    fps()    = 0;							// 视频帧率
	virtual VideoType video_type() = 0;						// 视频格式

	virtual bool demuxer(const std::string& videoPath, const std::string& audioPath, bool isDebug = 0) = 0;	//提取音屏和视频文件
	virtual bool xaudio(const std::string& path, bool isDebug = 0) = 0;	// 提取音频文件到指定路径
	virtual bool xvideo(const std::string& path, bool isDebug = 0) = 0;	// 提取视频文件到指定路径
	virtual bool xyuv(  const std::string& path, bool isDebug=0) = 0;	// 提取视频文件中的yuv
	
protected:
	virtual bool _open_()=0;	
public:
	static std::shared_ptr<Emedia> generate(const std::string& path);
	//Muxer _muxer;
	static bool combine(const std::string& mediaPath, const std::string& videoPath, const std::string& audioPath=""){
		Muxer muxer(videoPath, audioPath, mediaPath);
		muxer.combineVideoAudio();				//throw	
		return 0;
	};

	static void getFileType(const std::string& filename,std::string& fileType){	//获取文件类型
		int loc = filename.find(".");
		fileType = filename.substr(loc + 1);
		//std::string fileType(filename.substr(loc + 1));		
	}
};

#endif