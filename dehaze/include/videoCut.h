#pragma once
#include<opencv2/opencv.hpp>
#include<opencv2/videoio.hpp>
#include<string>

void cutVideo(std::string fileName, size_t cutTime, std::string outputFilename);
int call();
