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
		media = Emedia::generate("XVID_MPEG.avi");	//Wildlife	Wildlife.wmv
		
		//media->demuxer("video.h264", "audio.aac");			//Í¨¹ý
		//media->xaudio("Wildlife.aac");
		
		//height = media->high();
		fra = media->frames();

		media->xvideo("MPEG_4_AAC");
		//media->xyuv(  "2020.yuv",1);
		//media->combine("2018.h264", "2018.aac", "combine.mp4");	
		//type= media->video_type();
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
