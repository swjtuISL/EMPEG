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
	int high()  override;
	int width() override;
	int64_t frames()  override;
	double  fps()	  override;
	bool	audio_exists() override;
	VideoType video_type() override;
	

	bool demuxer(const std::string& videoPath, const std::string& audioPath, bool isDebug = 0);
	bool xaudio(const std::string& path, bool isDebug=0)   override;
	bool xvideo(const std::string& path, bool isDebug = 0) override;
	int xyuv(const std::string& path,bool isDebug=0)     override;

protected:
	void _openFormatCtx();							//������formatContext��find streams
	bool _open_() override;
	bool _read_frame(AVPacket& pkt);
	bool _decode    (AVPacket* pkt, AVFrame& yuv);

	void creatStream();
	//bool _yuv_rgb();
private:
	std::string _filePath;								 //Դ�ļ�
	std::string  _fileType;						 //�ļ�����(mp4..)
	
	AVFormatContext*   _formatCtx  = nullptr;               //Դ�ļ���ʽ
	AVFormatContext*   _ofmt_ctx_v = nullptr;              //�����Ƶ��ʽ
	AVFormatContext*   _ofmt_ctx_a = nullptr;              //�����Ƶ��ʽ
	AVOutputFormat*    _ofmt_v = nullptr;
	AVOutputFormat*    _ofmt_a = nullptr;
	AVCodecContext*    _encodecCtx = nullptr;
   
	int _videoStream; 
	int _audioStream;								 //����Ƶ��������ȡʱ��������Ƶ
	int _flag;
	int _ret = 0;
    //static hash_map<AVCodecID, VideoType> _videoTypeMap;
};

#endif