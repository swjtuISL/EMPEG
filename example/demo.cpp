#include <iostream>
#include <string>
#include <memory>
#include "Emedia.h"

using namespace std;

int main(){
	shared_ptr<Emedia> media = Emedia::generate("video_2018.mp4");	//Wildlife
	if (media == NULL)	return 1;

	media->demuxer("video.h264", "audio.aac");			//通过
	//media->xaudio("audio.aac");
	//media->xvideo("video.h264");						//通过
	//media->combine(videoPath, audioPath, mediaPath);

	cin.get();
	cout << "done!" << endl;
	return 0;
}