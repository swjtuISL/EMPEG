/*
#include <iostream>
#include <string>
#include <memory>
#include "Emedia.h"
#include"EmpegException.h"

using namespace std;
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib, "avutil.lib")

int main(){
	shared_ptr<Emedia> media = nullptr;
	Emedia::VideoType type;
	int height = 0;
	int64_t fra = 0;
	try{
		media = Emedia::generate("czl.mp4");	//Wildlife	Wildlife.wmv
		
		//height = media->high();
		//fra = media->frames();
		//type= media->video_type();
		//int fps = media->fps();
		//bool audio=media->isAudio();

		//media->xvideo("czl_2.264");
		//media->xvideo("czl_0.mpeg");
		//media->demuxer("czl_1.264", "czl_1.aac");			//ͨ��
		//media->xaudio("czl_2.aac");
		
		media->xyuv(  "czl.yuv",1);
		//media->combine("test_2.mp4","AVC_AAC.264");		
	}
	catch (OpenException except){
		cout << except.what() << endl;
		cout << except.where() << endl;
		cin.get();
		return 1;
	}
	catch (ParamExceptionPara except){
		cout<<except.what()<<endl;
		cout<<except.where()<<endl;
		cin.get();
		return 1;
	}
	catch (StreamExceptionPara except){
		cout << except.what() << endl;
		cout << except.where() << endl;
		cin.get();
		return 1;
	}
	catch (WriteExceptionPara except){
		cout << except.what() << endl;
		cout << except.where() << endl;
		cin.get();
		return 1;
	}
	catch (DecodeExceptionPara except){
		cout << except.what() << endl;
		cout << except.where() << endl;
		cin.get();
		return 1;
	}
	
	cout << "-end-!" << endl;
	cin.get();	
	return 0;
}
*/