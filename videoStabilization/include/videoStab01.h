#pragma once
#include<opencv2/opencv.hpp>
#include<opencv2/videostab.hpp>
#include<string>
#include<iostream>

void videoOutput(cv::Ptr<cv::videostab::IFrameSource> stabFrames, std::string outputPath);
void cacStabVideo(cv::Ptr<cv::videostab::IFrameSource> stabFrames, std::string srcVideoFile);
int stabilize01(std::string inputPath_, std::string outputPath_);