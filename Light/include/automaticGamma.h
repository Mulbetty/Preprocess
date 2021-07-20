#ifndef AUTOMATICGAMMA_H
#define AUTOMATICGAMMA_H
#pragma once
#include<opencv2\opencv.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\core\core.hpp>
#include<io.h>
#include<string>
#include<iostream>
#include<fstream>

enum IMSTATUS
{
    IM_STATUS_OK,
    IM_STATUS_NULLREFRENCE,
    IM_STATUS_INVALIDPARAMETER,
    IM_STATUS_NOTSUPPORTED
};
int IM_AutoGammaCorrection(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride);
int IM_Curve(unsigned char* Src, unsigned char* Dest, int Width, int Height, int Stride, unsigned char* TableB, unsigned char* TableG, unsigned char* TableR);
void testGamma(std::string input, std::string output);
#endif // AUTOMATICGAMMA_H
