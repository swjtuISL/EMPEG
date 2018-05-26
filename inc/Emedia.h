#ifndef __EMEDIA_H
#define __EMEDIA_H
#include <string>
#include <memory>

enum VideoType{
	NONE,	//��
	H264,
	MPEG4,
	JPEG2000
};

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
	void EmideaClose();
	void openInit();
	void findStream();
	void Muxer::writeFream(int64_t& cur_pts_v, int64_t& cur_pts_a);
	bool combineVideoAudio();

private:
	const std::string _videoPath;
	const std::string _audioPath;
	const std::string _mediaPath;

	AVFormatContext* _ifmt_ctx_v;
	AVFormatContext* _ifmt_ctx_a;
	AVFormatContext* _ofmt_ctx;

	AVOutputFormat* _ofmt;

	int _videoindex_v;
	int _audioindex_a;
	int _videoindex_out;
	int _audioindex_out;

	int frame_index;
};

//--------------class Emedia-------------------------------------
class Emedia{
public:
	virtual ~Emedia(){};
	virtual const std::string& where() = 0;				// ��Ƶ���ڵľ���·��
	virtual int high() = 0;								// ��Ƶ�ļ���
	virtual int width() = 0;							// ��Ƶ�ļ���
	virtual int64_t frames() = 0;						// ��Ƶ֡��
	virtual double fps() = 0;							// ��Ƶ֡��
	virtual VideoType video_type() = 0;					// ��Ƶ��ʽ

	virtual bool demuxer(const std::string& videoPath, const std::string& audioPath)=0;	//��ȡ��������Ƶ�ļ�
	virtual bool xaudio(const std::string& path) = 0;	// ��ȡ��Ƶ�ļ���ָ��·��
	virtual bool xvideo(const std::string& path) = 0;	// ��ȡ��Ƶ�ļ���ָ��·��
	virtual bool xyuv(const std::string& path) = 0;		// ��ȡ��Ƶ�ļ��е�yuv

protected:
	virtual bool _open_()=0;
	
public:
	static std::shared_ptr<Emedia> generate(const std::string& path);
	//Muxer _muxer;
	static bool combine(const std::string& videoPath, const std::string& audioPath, const std::string& mediaPath);
};

//------------------class EmediaException-open error-----------------------------
class EmediaException :public std::exception
{
public:
	EmediaException(std::string s){ _s = s; };
	virtual const std::string& what(){ return _s; };

private:
	std::string _s;
};

//------------------class EmediaException---parament error--------------------------
class EmediaExceptionPara :public std::exception
{
public:
	EmediaExceptionPara(std::string s) :_s(s){};
	virtual const std::string& what(){ return _s; };

private:
	std::string _s;
};
#endif