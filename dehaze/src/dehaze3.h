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


//��ʼȥ��
bool DoDefogging(std::string inputDir, std::string outputDir);

//ͼ��ȥ��
void ImageDefogging(Mat src, Mat& dst, int rectSize, double omega, double numt);

//��ͼ��ͨ��
Mat DarkChannel(Mat srcImg, int size);

// ʵ��argsort���ܣ�����ֵ��С���������ֵ
template<typename T> vector<int> argsort(const vector<T>& array);

//ȫ�������ֵA
void AtmLight(Mat src, Mat dark, float outA[3]);

//����͸����Ԥ��ֵ
Mat TransmissionEstimate(Mat src, float outA[3], int size, float omega);

//�����˲�
Mat Guidedfilter(Mat src, Mat te, int r, float eps);

//ͨ�������˲�����͸����
Mat TransmissionRefine(Mat src, Mat et);

//ͼ��ȥ��
Mat Defogging(Mat src, Mat t, float outA[3], float tx = 0.1);

//��ȡĿ¼�µ������ļ�
void GetFiles(string path, vector<string>& files, string fileType = "*.*");

//����ͼ������Ⱥͱ��Ͷ�
void ConvertRGB2HSL(Mat src, float &lightness, float &saturation, bool isNormalize = false);

int mainDehaze3();
int dehazeVideo(); // ȥ���㷨������Ƶ