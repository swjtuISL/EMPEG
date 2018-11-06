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
	int numFrame=0;
	try{
		media = Emedia::generate("../../input/768_432_30F.mp4");	
		
		//height = media->high();
		//fra = media->frames();
		//type= media->video_type();
		//int fps = media->fps();
		//bool audio=media->audio_exists();

		//media->xvideo("qcif_suzie_30F.264",1);
		//media->xvideo("czl_0.mpeg");
		//media->demuxer("czl_1.264", "czl_1.aac");			//Í¨¹ý
		//media->xaudio("czl_2.aac");		
		numFrame=media->xyuv("../../output/768_432_30F.yuv", 1);

		//media->combine("czl.avi","czl.264");		
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