# EMPEG
对FFMPEG作的封装，提供更为方便的接口。支持以下功能：
* 从*Media*中提取出*Audio*
* 从*Media*中提取出*Video*
* 从*Media*中提取*yuv*
* 将*Video*和*Audio*结合生成*Media*文件

注: *Media*指包含了音频和视频的音视频文件，如AVI, MP4, RMVB等文件。*Audio*指的是音频文件，如WAV, MP3等文件。*Video*指的是压缩视频文件，如264文件。*yuv*指的是像素级视频文件。

## 一、文件目录
```
├── example     测试代码
├── ffmpeg      ffmpeg的相关依赖文件(头文件/静态库/动态库)
├── inc         empeg的头文件
├── src         empeg的cpp文件
├── sln         visual studio工程配置文件
├── .gitignore
└── readme.md
```
## 二、环境配置
    1、ffmpeg官网下载地址：http://ffmpeg.org/download.html
        选择Architecture、Linking中的shared和Dev文件，static是静态连接库；
        Dev文件包含examples、include、lib；shared文件夹包含bin（dll）文件
    
    2、在visual studio下配置头文件和库文件：
       打开配置属性页面——>C/C++——>常规——>附含包含目录；添加Dev中的include头文件(如..\..\ffmpeg\include)
       配置属性——>链接器——>常规——>附加库目录；添加Dev中的lib库文件，(如..\..\ffmpeg\lib\win64) 根据PC系统选择win32\win64的库
       配置属性——>调试——>环境；添加shared文件中的dll文件(如path=%path%;..\..\ffmpeg\bin\win64) 根据PC系统选择win32\win64的库
    
## 三、最小实例
该库通过`Emedia`提供的静态函数来生成media对象，并通过操作该对象的接口以实现目标功能。
```c++
using namespace std;
int main(){
    shared_ptr<Emedia> pm = Emedia()
    cout << "视频高度:" << pm->high() << endl;
    cout << "视频宽度:" << pm->wdith() << endl;

    pm->xaudio("media_audio.mp3");      // 提取media文件中的音频
    pm->xvideo("media_video.264");      // 提取media文件中的视频
    pm->xyuv("media_yuv.yuv");          // 提取media文件中的yuv文件

}
```

## 四、接口说明
### *1.Emedia*
`Emedia`仅仅是一个抽象类，用于提供必要的接口与生成函数。
```c++
class Emedia{
public:
    virtual const std::string& where() = 0;             // 视频所在的绝对路径
    virtual int high() = 0;                             // 视频文件高
    virtual int width() = 0;                            // 视频文件宽
    virtual int frames() = 0;                           // 视频帧数
    virtual double fps() = 0;                           // 视频帧率

    virtual bool xaudio(const std::string& path) = 0;   // 提取音频文件到指定路径
    virtual bool xvideo(const std::string& path) = 0;   // 提取视频文件到指定路径
    virtual bool xyuv(const std::string& path) = 0;     // 提取视频文件中的yuv
	
public:
    // 生成Emedia实体
    static std::shared_ptr<Emedia> generate(const std::string& path);

    // 将视频和音频文件结合生成media文件
    static bool combine(const std::string& videoPath,   // 已经存在的视频文件
                        const std::string& audioPath,   // 已经存在的音频文件
                        const std::string& mediaPath);  // 生成的media文件路径
};
```
### *2.VideoType*
`VideoType`是一个Enum，用于说明media文件中的video类型，现支持如下类型：
```c++
enum VideoType{
	H264,
	MPEG4
};
```