#include<string>
//class string;
#include<exception>
//class string;					//声明会出问题
#ifndef _EMPEGEXCEPTION_H
#define _EMPEGEXCEPTION_H

class testExcept:public std::exception{
public:
	testExcept(const std::string& w) :_w(w){};

private:
	std::string _w;
	AVFormatContext* _avformatCtx = nullptr;
};
//--------封装层异常---------------------------
class OpenException :public std::exception
{
public:
	//OpenException(const std::string& w) :_where(w){};
	OpenException(const std::string r = "\0") : _reson(r){};
	OpenException(const std::string  w, std::string r ) :_where(w), _reson(r){};
	OpenException(const std::string  w, AVFormatContext* avformatCtx) :_avformatCtx(avformatCtx), _where(w){};
	OpenException(const std::string  w, AVCodecContext*  avcodecCtc)  :_avcodecCtc(avcodecCtc), _where(w){};
	OpenException(const std::string  w, AVCodec       *  avcodec)     :_avcodec(avcodec), _where(w){};
	
	virtual const std::string& what() { return _reson; };		//异常原因
	virtual void what(const AVFormatContext* avformatCtx, const AVCodecContext* avcodecCtc){
		_avformatCtx=avformatCtx;
		_avcodecCtc = avcodecCtc;
	};		
	virtual const std::string& where(){ return _where; };		//异常位置
private:
	std::string _reson;
	std::string _where;
	const AVFormatContext* _avformatCtx = nullptr;
	const AVCodecContext*  _avcodecCtc  = nullptr;
	const AVCodec*		   _avcodec     = nullptr;
};

//-------数据流层异常--------------------------
class StreamExceptionPara :public std::exception
{
public:
	StreamExceptionPara(std::string r = "\0") :_reson(r){};
	StreamExceptionPara(const std::string w, std::string r ) :_reson(r), _where(w){};
	StreamExceptionPara(const std::string w, AVFormatContext* avformatCtx) :_avformatCtx(avformatCtx), _where(w){};
	StreamExceptionPara(const std::string w, AVCodecContext*  avcodecCtc)  :_avcodecCtc(avcodecCtc)  ,   _where(w){};
	StreamExceptionPara(const std::string w, AVStream*        avstream)    :_avstream(avstream)      , _where(w){};
	virtual const std::string& what() { return _reson; };		//异常原因
	virtual const std::string& where(){ return _where; };		//异常位置
	
	virtual void what(AVFormatContext* avformatCtx, AVCodecContext* avcodecCtc) {
		_avformatCtx = avformatCtx;	_avcodecCtc = avcodecCtc;
	};
	virtual void what(AVFormatContext* avformatCtx, AVStream* avstream) {
		_avformatCtx = avformatCtx;	_avstream = avstream;
	};
private:
	std::string _reson;
	std::string _where;
	const AVFormatContext* _avformatCtx = nullptr;
	const AVCodecContext*  _avcodecCtc  = nullptr;
	const AVStream*		 _avstream    = nullptr;
};

//-------写数据异常--------------------------
class WriteExceptionPara :public std::exception
{
public:
	WriteExceptionPara(const std::string w, std::string r = "\0") :_reson(r), _where(w){};
	WriteExceptionPara(const std::string w, AVFormatContext* avformatCtx) :_avformatCtx(avformatCtx), _where(w){};
	WriteExceptionPara(const std::string w, AVCodecContext*  avcodecCtc)  :_avcodecCtc(avcodecCtc), _where(w){};
	WriteExceptionPara(AVFormatContext* avformatCtx, AVPacket* packet)    :_avformatCtx(avformatCtx), _packet(packet){};
	WriteExceptionPara(std::string r, AVFormatContext* avformatCtx, AVPacket* packet) :_reson(r),_avformatCtx(avformatCtx), _packet(packet){};
	virtual const std::string& what() { return _reson; };		//异常原因
	virtual const std::string& where(){ return _where; };		//异常位置

private:
	std::string _reson;
	std::string _where;
	AVFormatContext* _avformatCtx;// = nullptr;
	AVCodecContext*  _avcodecCtc;// = nullptr;
	AVPacket*		 _packet;// = nullptr;
};

//-------解码层异常--------------------------
class DecodeExceptionPara :public std::exception
{
public:
	DecodeExceptionPara(std::string r ) :_reson(r){};
	DecodeExceptionPara( std::string w, std::string r ) :_reson(r), _where(w){};
	DecodeExceptionPara(AVCodecContext*  avcodecCtc, AVPacket*	packet) :_avcodecCtc(avcodecCtc), _packet(packet){};
	DecodeExceptionPara(AVCodecContext*  avcodecCtc, AVFrame*  frame)   :_avcodecCtc(avcodecCtc), _frame(frame){};
	//DecodeExceptionPara(std::string w, std::string r = '\0') :_reson(r), _where(w){};
	virtual const std::string& what() { return _reson; };		//异常原因
	virtual const std::string& where(){ return _where; };		//异常位置

private:
	std::string _reson;
	std::string _where;
	AVCodecContext*  _avcodecCtc;// = nullptr;
	AVPacket*		 _packet;// = nullptr;
	AVFrame*		 _frame;
};

//-------参数异常--------------------------
class ParamExceptionPara :public std::exception
{
public:
	//, std::string r = "\0"			_reson(r), 
	ParamExceptionPara(const std::string   w) :_where(w){};
	ParamExceptionPara(const std::string src, const std::string dst) :_src(src), _dst(dst){};
	virtual const std::string& what() { return _dst; };								//异常原因
	virtual const std::string& where(){ return _where; };							//异常位置

private:
	std::string _reson;
	std::string _where;
	std::string _src;
	std::string _dst;
};
#endif