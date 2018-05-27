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
	try{
		media = Emedia::generate("video_2018.mp4");	//Wildlife	

		media->xvideo("audio.h264");						//通过
		//media->demuxer("video.h264", "audio.aac");			//通过
		media->xaudio("audio.aac");
		//media->xyuv("yuv.yuv");
		//media->combine("audio.h264", "audio.aac", "combine.mp4");	
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
	
	cout << "done!" << endl;
	cin.get();	
	return 0;
}