#pragma once
#ifndef VIDEOSTAB02_H
#define VIDEOSTAB02_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/features2d/features2d.hpp"
//#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/flann/flann.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <Windows.h>
#include <cmath>
#include <fstream>
#include <time.h>
#include <io.h>

using namespace cv;
using namespace std;

class VideoStab
{
public:
	VideoStab();
	VideoCapture capture;

	Mat frame2;
	Mat frame1;

	int k;

	const int HORIZONTAL_BORDER_CROP = 30;

	Mat smoothedMat;
	Mat affine;

	Mat smoothedFrame;

	double dx;
	double dy;
	double da;
	double ds_x;
	double ds_y;

	double sx;
	double sy;

	double scaleX;
	double scaleY;
	double thetha;
	double transX;
	double transY;

	double diff_scaleX;
	double diff_scaleY;
	double diff_transX;
	double diff_transY;
	double diff_thetha;

	double errscaleX;
	double errscaleY;
	double errthetha;
	double errtransX;
	double errtransY;

	double Q_scaleX;
	double Q_scaleY;
	double Q_thetha;
	double Q_transX;
	double Q_transY;

	double R_scaleX;
	double R_scaleY;
	double R_thetha;
	double R_transX;
	double R_transY;

	double sum_scaleX;
	double sum_scaleY;
	double sum_thetha;
	double sum_transX;
	double sum_transY;

	Mat stabilize(Mat frame_1, Mat frame_2);
	void Kalman_Filter(double* scaleX, double* scaleY, double* thetha, double* transX, double* transY);
};

#endif // VIDEOSTAB02_H

int stabilize02(std::string inputPath_, std::string outputPath_);
