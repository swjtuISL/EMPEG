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
	try{
		shared_ptr<Emedia> media = Emedia::generate("video_2018.mp4");	//Wildlife
	}
	catch (ParamExceptionPara){
		std::cout << "file type error\n"; return 1;
	}
	//if (media == NULL)	return 1;

	//media->demuxer("video.h264", "audio.aac");			//通过
	//media->xaudio("audio.aac");
	//media->xvideo("video.h264");						//通过
	//media->combine(videoPath, audioPath, mediaPath);

	cin.get();
	cout << "done!" << endl;
	return 0;
}