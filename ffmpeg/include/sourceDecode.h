#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavutil/hwcontext.h>
#ifdef __cplusplus
}
#include <plog/Log.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
// #include <RollingFileInitializer.h>
#include<thread>
void decodeCamera(const char*);
void hardDecode(const char* rtspString);
cv::Mat avframeToCvmat(const AVFrame* frame);
void showVideo(cv::Mat);
#endif
#define INBUF_SIZE 4096
