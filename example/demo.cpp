
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
		
<<<<<<< HEAD
		//media->demuxer("video.h264", "audio.aac");			//通锟斤拷
		media->xaudio("2019.aac",true);
		media->xvideo("2019.h264");							//通锟斤拷
		//media->xyuv(  "2019.yuv");
		//media->combine("2018.h264", "2018.aac", "combine.mp4");	
=======
		//height = media->high();
<<<<<<< HEAD
		//fra = media->frames();
>>>>>>> 8a59f6bb4e4d71b04010cfb9ee112047c5623726
		//type= media->video_type();
=======
		//fra    = media->frames();
		//type   = media->video_type();
>>>>>>> fa3e86a336865f081dbb849e74ca90524d7535e7
		//int fps = media->fps();
		//bool audio=media->isAudio();

		//media->xvideo("czl_2.264");
		//media->xvideo("czl_0.mpeg");
		//media->demuxer("czl_1.264", "czl_1.aac");			//通过
		//media->xaudio("czl_2.aac");		
		media->xyuv( "XVID_MPEG.yuv",1);

		//media->combine("MPEG_4_AAC_2.mp4","MPEG_4_AAC.mp4","music.mp3");		
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
