#pragma once

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cassert>
#include <cmath>
#include <fstream>

using namespace std;
using namespace cv;

// In frames. The larger the more stable the video, but less reactive to sudden panning �ƶ�ƽ���������ڴ�С
const int SMOOTHING_RADIUS = 50;

/**
 * @brief �˶���Ϣ�ṹ��
 *
 */
struct TransformParam
{
	TransformParam() {}
	//x����Ϣ��y����Ϣ���Ƕ���Ϣ
	TransformParam(double _dx, double _dy, double _da)
	{
		dx = _dx;
		dy = _dy;
		da = _da;
	}

	double dx;
	double dy;
	// angle
	double da;

	void getTransform(Mat& T)
	{
		// Reconstruct transformation matrix accordingly to new values �ؽ��任����
		T.at<double>(0, 0) = cos(da);
		T.at<double>(0, 1) = -sin(da);
		T.at<double>(1, 0) = sin(da);
		T.at<double>(1, 1) = cos(da);

		T.at<double>(0, 2) = dx;
		T.at<double>(1, 2) = dy;
	}
};

/**
 * @brief �켣�ṹ��
 *
 */
struct Trajectory
{
	Trajectory() {}
	Trajectory(double _x, double _y, double _a)
	{
		x = _x;
		y = _y;
		a = _a;
	}

	double x;
	double y;
	// angle
	double a;
};
vector<Trajectory> cumsum(vector<TransformParam>& transforms);
vector<Trajectory> smooth(vector<Trajectory>& trajectory, int radius);
void fixBorder(Mat& frame_stabilized);