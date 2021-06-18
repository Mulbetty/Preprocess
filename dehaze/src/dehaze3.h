#pragma once

#include<Eigen\Core>
#include<Eigen\Dense>
#include<opencv.hpp>
#include<opencv2/core/eigen.hpp>
// #include <opencv2/ximgproc.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<highgui\highgui.hpp>

#include<ximgproc.hpp>
#include<iostream>
#include<ctime>
#include<io.h>
#include<sstream>


using namespace std;
using namespace cv;


//开始去雾
bool DoDefogging(std::string inputDir, std::string outputDir);

//图像去雾
void ImageDefogging(Mat src, Mat& dst, int rectSize, double omega, double numt);

//求图像暗通道
Mat DarkChannel(Mat srcImg, int size);

// 实现argsort功能：数组值从小到大的索引值
template<typename T> vector<int> argsort(const vector<T>& array);

//全球大气光值A
void AtmLight(Mat src, Mat dark, float outA[3]);

//计算透射率预估值
Mat TransmissionEstimate(Mat src, float outA[3], int size, float omega);

//导向滤波
Mat Guidedfilter(Mat src, Mat te, int r, float eps);

//通过导向滤波计算透射率
Mat TransmissionRefine(Mat src, Mat et);

//图像去雾
Mat Defogging(Mat src, Mat t, float outA[3], float tx = 0.1);

//获取目录下的所有文件
void GetFiles(string path, vector<string>& files, string fileType = "*.*");

//计算图像的亮度和饱和度
void ConvertRGB2HSL(Mat src, float &lightness, float &saturation, bool isNormalize = false);

int mainDehaze3();
int dehazeVideo(); // 去雾算法处理视频