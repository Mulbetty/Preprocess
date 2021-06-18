#include<iostream>
#include<opencv2/opencv.hpp>
#include<opencv2/core.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/videoio.hpp>
#include<opencv2/imgproc.hpp>

cv::Mat GuildeFilter_1(cv::Mat& I, cv::Mat&p, int r, double eps);
int mainGuidedFilter1();