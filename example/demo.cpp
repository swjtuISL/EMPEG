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
		//media = Emedia::generate("test.mp4");	//Wildlife	Wildlife.wmv
		
		//height = media->high();
		//fra = media->frames();
		//type= media->video_type();
		//int fps = media->fps();

		//media->demuxer("AVC_AAC.264", "AVC_AAC.aac");			//Í¨¹ý
		//media->xaudio("Wildlife.aac");
		//media->xvideo("test2.264");
		//media->xyuv(  "2020.yuv",1);
		
		media->combine("test_2.mp4","AVC_AAC.264");		
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
