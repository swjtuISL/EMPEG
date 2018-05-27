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
	try{
		media = Emedia::generate("video_2018.mp4");	//Wildlife	
	}
	catch (ParamExceptionPara except){
		cout<<except.what()<<endl;
		cout<<except.where()<<endl;
		std::cout << "file type error\n"; 
		cin.get();
		return 1;
	}
	cout << "file is ok\n";

	Emedia::VideoType type= media->video_type();
	//media->demuxer("video.h264", "audio.aac");			//ͨ��
	//media->xaudio("audio.aac");
	//media->xvideo("video.h264");						//ͨ��
	//media->combine(videoPath, audioPath, mediaPath);

	cin.get();
	cout << "done!" << endl;
	return 0;
}