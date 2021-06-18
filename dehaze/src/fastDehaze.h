#pragma once
#include<iostream>
#include<opencv2\core\core.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\imgproc\imgproc.hpp>
#include<algorithm>
#include<ctime>
#include<vector>

void min_filter(cv::Mat &src_img, cv::Mat &res_img, int kernel_size);
cv::Mat min_BGR(cv::Mat &src_img);
cv::Mat max_BGR(cv::Mat &src_img);
cv::Mat getLx_A(cv::Mat &M_min, cv::Mat &img, float p, int kernel_size, float &A);
cv::Mat FastDehaze(cv::Mat &Img, const double p, const int kernel_size, float eps = 0.0001);
int mainFast();
