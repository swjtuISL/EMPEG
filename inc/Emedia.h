#ifndef __EMEDIA_H
#define __EMEDIA_H
#include <string>
#include <memory>

enum VideoType{
	NONE,	//无
	H264,
	MPEG4,
	JPEG2000
};

class Emedia{
public:
	virtual ~Emedia(){};
	virtual const std::string& where() = 0;				// 视频所在的绝对路径
	virtual int high() = 0;								// 视频文件高
	virtual int width() = 0;							// 视频文件宽
	virtual int64_t frames() = 0;						// 视频帧数
	virtual double fps() = 0;							// 视频帧率
	virtual VideoType video_type() = 0;					// 视频格式

	virtual bool demuxer(const std::string& videoPath, const std::string& audioPath)=0;	//提取音屏和视频文件
	virtual bool xaudio(const std::string& path) = 0;	// 提取音频文件到指定路径
	virtual bool xvideo(const std::string& path) = 0;	// 提取视频文件到指定路径
	virtual bool xyuv(const std::string& path) = 0;		// 提取视频文件中的yuv

protected:
	virtual bool __open__()=0;
	
public:
	static std::shared_ptr<Emedia> generate(const std::string& path);
	static bool combine(const std::string& videoPath, const std::string& audioPath, const std::string& mediaPath);
};

#endif