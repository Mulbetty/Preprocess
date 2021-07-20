#ifndef ADAPTIVEGAMMA_H
#define ADAPTIVEGAMMA_H
#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <string>

int adaptiveGama(std::string inputPath, std::string outputPath);

cv::Mat work(cv::Mat src) ;
#endif // ADAPTIVEGAMMA_H
