#include <iostream>
#include <string>
#include <memory>
#include "Emedia.h"

using namespace std;

int main(){
	shared_ptr<Emedia> media = Emedia::generate("video_2018.mp4");	//Wildlife

	/*cout << media->where() << endl;		
	cout << media->high() << endl;
	cout << media->width() << endl;
	cout << media->frames() << endl;
	cout << media->fps() << endl;*/
	
	/*const std::string& videoPath ="video.h264" ;
	const std::string& audioPath="audio.aac";
	bool ret = media->demuxer(videoPath, audioPath);*/
	
	//media->xyuv("yuvfile.yuv");
	//cout<<"video_type:"<<media->video_type()<<endl;
	string videoPath = "video_2018_3.h264"; string audioPath = "Run.mp3";
	string mediaPath = "aaaaa.mp4";

	media->xaudio("audio.aac");
	//media->combine(videoPath, audioPath, mediaPath);
	cin.get();
	cout << "done!" << endl;
}