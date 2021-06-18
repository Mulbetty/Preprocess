#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cassert>
#include <cmath>
#include <fstream>
#include <Windows.h>
#include <cudaimgproc.hpp>
#include <cudaobjdetect.hpp>


int stabilize04(std::string inputPath_, std::string outputPath_);