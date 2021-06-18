#pragma once

#include<opencv2/opencv.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/core/cuda.hpp>
#include<opencv2/videostab.hpp>
#include<opencv2/cudaoptflow.hpp>
#include <cassert>
#include <cmath>
#include <fstream>
#include <Windows.h>
#include<iostream>
#include<opencv2/cudaimgproc.hpp>
#include<opencv2/cudawarping.hpp>
#include <opencv2/cudaobjdetect.hpp>

struct TransformParam
{
	TransformParam() {}
	TransformParam(double _dx, double _dy, double _da) {
		dx = _dx;
		dy = _dy;
		da = _da;
	}

	double dx;
	double dy;
	double da; // angle
};

struct Trajectory
{
	Trajectory() {}
	Trajectory(double _x, double _y, double _a) {
		x = _x;
		y = _y;
		a = _a;
	}
	// "+"
	friend Trajectory operator+(const Trajectory& c1, const Trajectory& c2) {
		return Trajectory(c1.x + c2.x, c1.y + c2.y, c1.a + c2.a);
	}
	//"-"
	friend Trajectory operator-(const Trajectory& c1, const Trajectory& c2) {
		return Trajectory(c1.x - c2.x, c1.y - c2.y, c1.a - c2.a);
	}
	//"*"
	friend Trajectory operator*(const Trajectory& c1, const Trajectory& c2) {
		return Trajectory(c1.x * c2.x, c1.y * c2.y, c1.a * c2.a);
	}
	//"/"
	friend Trajectory operator/(const Trajectory& c1, const Trajectory& c2) {
		return Trajectory(c1.x / c2.x, c1.y / c2.y, c1.a / c2.a);
	}
	//"="
	Trajectory operator =(const Trajectory& rx) {
		x = rx.x;
		y = rx.y;
		a = rx.a;
		return Trajectory(x, y, a);
	}

	double x;
	double y;
	double a; // angle
};
const int HORIZONTAL_BORDER_CROP = 20;

using namespace cv;
using namespace std;
int gputest01(std::string );
int stablize04Gpu(std::string inputPath, std::string outputPath);


