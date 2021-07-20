#include "../include/automaticGamma.h"

// 自适应gama矫正,这个函数用来控制像素值
unsigned char IM_ClampToByte(int Value)
{
    if (Value < 0)
        return 0;
    else if (Value > 255)
        return 255;
    else
        return (unsigned char)Value;
    //return ((Value | ((signed int)(255 - Value) >> 31)) & ~((signed int)Value >> 31));
}

// 获取图像的平均值
int IM_GetAverageValue(unsigned char* src, int Width, int Height, int Stride, int& AvgB, int& AvgG, int& AvgR, int& AvgA)
{
    int Channel = Stride / Width;
    if (Channel == 1)
    {
        int sumA = 0;
        for (long i = 0;i < Height;i++)
        {
            for (long j=0;j<Width;j++)
            {
                sumA += (unsigned int)src[i*j];
            }
        }
        AvgB = static_cast<unsigned int>(sumA*1.0f / (Width*Height));
    }
    else if(Channel==3)
    {
        int sumR = 0;
        int sumG = 0;
        int sumB = 0;
        int srcSize = Width*Height;
        for (long i = 0;i < Stride*Height;)
        {

            sumB += (unsigned int)src[i];
            sumG += (unsigned int)src[i + 1];
            sumR += (unsigned int)src[i + 2];
            i += 3;
        }
        AvgB = static_cast<unsigned int>(sumB*1.0f / srcSize);
        AvgG = static_cast<unsigned int>(sumG*1.0f / srcSize);
        AvgR = static_cast<unsigned int>(sumR*1.0f / srcSize);
        AvgA = static_cast<unsigned int>((AvgB + AvgG + AvgR) / 3.0);
        return 0;
    }
}

// 根据gamma表对图像进行调整
int IM_Curve(unsigned char* Src, unsigned char* Dest, int Width, int Height, int Stride, unsigned char* TableB, unsigned char* TableG, unsigned char* TableR)
{
    int Channel = Stride / Width;
    if (Channel == 1)
    {
        int sumA = 0;
        for (int i = 0;i < Stride*Height;i++)
        {
                Dest[i] = (unsigned char)TableB[(int)Src[i]];
        }
    }
    else if (Channel == 3)
    {
        for (int i=0;i<Height*Stride;)
        {
            Dest[i] = (unsigned char)TableB[(int)Src[i]];
            Dest[i+1]= (unsigned char)TableG[(int)Src[i+1]];
            Dest[i+2] = (unsigned char)TableR[(int)Src[i + 2]];
            i += 3;
        }
    }
    return 0;
}
int IM_AutoGammaCorrection(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride)
{
    int Channel = Stride / Width;
    if ((Src == NULL) || (Dest == NULL))                    return IM_STATUS_NULLREFRENCE;
    if ((Width <= 0) || (Height <= 0))                        return IM_STATUS_INVALIDPARAMETER;
    if ((Channel != 1) && (Channel != 3) && (Channel != 4))    return IM_STATUS_NOTSUPPORTED;
    int AvgB, AvgG, AvgR, AvgA;
    int Status = IM_GetAverageValue(Src, Width, Height, Stride, AvgB, AvgG, AvgR, AvgA); // 获取各个通道的平均像素值
    if (Status != IM_STATUS_OK)    return Status;
    if (Channel == 1)
    {
        float Gamma = -0.3 / (log10(AvgB / 256.0f));
        unsigned char Table[256];
        for (int Y = 0; Y < 256; Y++)        //    另外一种方式是：pow(Y / 255.0, 1.0 / Gamma）
        {
            Table[Y] = IM_ClampToByte((int)(pow(Y / 255.0f, Gamma) * 255.0f));
        }
        return IM_Curve(Src, Dest, Width, Height, Stride, Table, Table, Table);
    }
    else
    {
        float GammaB = -0.3 / (log10(AvgB / 256.0f));
        float GammaG = -0.3 / (log10(AvgG / 256.0f));
        float GammaR = -0.3 / (log10(AvgR / 256.0f));

        unsigned char TableB[256], TableG[256], TableR[256];
        for (int Y = 0; Y < 256; Y++)        //    另外一种方式是：pow(Y / 255.0, 1.0 / Gamma）
        {
            TableB[Y] = IM_ClampToByte((int)(pow(Y / 255.0f, GammaB) * 255.0f));
            TableG[Y] = IM_ClampToByte((int)(pow(Y / 255.0f, GammaG) * 255.0f));
            TableR[Y] = IM_ClampToByte((int)(pow(Y / 255.0f, GammaR) * 255.0f));
        }
        return IM_Curve(Src, Dest, Width, Height, Stride, TableB, TableG, TableR);
    }
}

void testGamma(std::string inputFile, std::string ouputFile)
{
    clock_t start, end;
    if (_access(inputFile.c_str(), 00) == -1)
    {
        std::cout << "图像文件不存在" << std::endl;
    }
    cv::namedWindow("before", 0);

    //std::ifstream fileRead(inputFile, std::ios::binary);
    cv::Mat img = cv::imread(inputFile);
    cv::imshow("before", img);
    cv::waitKey(1);
    start = clock();
    int width = img.cols;
    int height = img.rows;
    int channel = img.channels();
    unsigned char* Src = new unsigned char[width*height*channel];
    unsigned char* Dst = new unsigned char[width*height*channel];
    memset(Src, 0, width*height*channel);
    memcpy(Src, img.data, width*height*channel);
    IM_AutoGammaCorrection(Src, Dst, width, height, width*channel);
    reinterpret_cast<int *>(Dst);
    end = clock();
    std::cout << "处理用时：" << end - start << std::endl;
    cv::namedWindow("after", 0);
    img.data = Dst;

    delete[] Src;
    //delete[] Dst;
    cv::imshow("after", img);
    cv::waitKey(1);
    cv::waitKey(0);
    delete[] Dst;

}
