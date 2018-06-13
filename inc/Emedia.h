#ifndef __EMEDIA_H
#define __EMEDIA_H
#include <string>
#include <memory>
/*
enum VideoType{
	NONE,	//��
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
	virtual const     std::string& where() = 0;				// ��Ƶ���ڵľ���·��
	virtual int       high()   = 0;							// ��Ƶ�ļ���
	virtual int       width()  = 0;							// ��Ƶ�ļ���
	virtual int64_t   frames() = 0;							// ��Ƶ֡��
	virtual double    fps()    = 0;							// ��Ƶ֡��
	virtual VideoType video_type() = 0;						// ��Ƶ��ʽ
	virtual bool	  audio_exists() = 0;

	virtual bool demuxer(const std::string& videoPath, const std::string& audioPath, bool isDebug = false) = 0;	//��ȡ��������Ƶ�ļ�
	virtual bool xaudio(const std::string& path, bool isDebug = false) = 0;	// ��ȡ��Ƶ�ļ���ָ��·��
	virtual bool xvideo(const std::string& path, bool isDebug = false) = 0;	// ��ȡ��Ƶ�ļ���ָ��·��
	virtual bool xyuv(  const std::string& path, bool isDebug = false) = 0;	// ��ȡ��Ƶ�ļ��е�yuv
	
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

	static void getFileType(const std::string& filename,std::string& fileType){	//��ȡ�ļ�����
		int loc = filename.find(".");
		fileType = filename.substr(loc + 1);
		//std::string fileType(filename.substr(loc + 1));		
	}
};

#endif