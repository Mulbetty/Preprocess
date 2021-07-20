#include "../include/adaptiveGamma.h"

// 使用亮度分量公式以及retinex理论，进行伽马矫正. 这部分代码没有实现好
int adaptiveGama(std::string inputPath, std::string outputPath)
{

    cv::Mat frame = cv::imread(inputPath);
    frame.convertTo(frame, CV_32F);
    // cv::resize(frame, frame, cv::Size(), 0.5, 0.5);
    int col = frame.cols;
    int row = frame.rows;
    cv::imshow("", frame);
    if(frame.empty())
    {
        std::fprintf(stderr, "Error to read file");
        exit(1);
    }
    cv::Mat frameHSV(row, col, CV_32FC3);
    cv::cvtColor(frame, frameHSV, CV_BGR2HSV);
    frameHSV /= 255;
    std::vector<cv::Mat>  hsvChannels;
    cv::split(frameHSV, hsvChannels);
    cv::Mat v(hsvChannels[2]);
    cv::Mat h(hsvChannels[0]);
    cv::Mat s(hsvChannels[1]);
	cv::imshow("h", h);
	cv::imshow("s", s);
	cv::imshow("v", v);
    int HSIZE = fmin(frame.cols, frame.rows);
    float q = sqrt(2.0);
    v.convertTo(v, CV_32F);
    h.convertTo(h, CV_32F);
    s.convertTo(s, CV_32F);

    float SIGMA1 = 15;
    float SIGMA2 = 80;
    float SIGMA3 = 250;

    if(HSIZE%2==0)
    {
        HSIZE -=1;
    }

    cv::Mat gaus1, gaus2, gaus3;
    cv::GaussianBlur(v, gaus1, cv::Size(HSIZE, HSIZE), SIGMA1/q);
    cv::GaussianBlur(v, gaus2, cv::Size(HSIZE, HSIZE), SIGMA2/q);
    cv::GaussianBlur(v, gaus3, cv::Size(HSIZE, HSIZE), SIGMA3/q);
    cv::Mat gaus;
    gaus = (gaus1 + gaus2 + gaus3)/3;
    cv::imshow("gaus", gaus);
    // std::cout << mean(gaus) << std::endl;
    float average = mean(gaus)[0];
    std::cout<<"average:"<<average<<std::endl;

    cv::Mat out(row, col, CV_32FC1);
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            float gamma = powf(0.5, (average - gaus.at<float>(i, j)) / average);
            out.at<float>(i, j) = powf(v.at<float>(i, j), gamma);
        }
    }
    std::cout << "h type:" << h.type() << std::endl;
    std::cout << "s type:" << s.type() << std::endl;
    std::cout << "out type:" << out.type() << std::endl;
    // cv::imshow("output", out);
    
    std::vector<cv::Mat> hsvMerge;
    hsvMerge.push_back(h);
    hsvMerge.push_back(s);
    hsvMerge.push_back(out);
    cv::Mat merge_;
    cv::merge(hsvMerge, merge_);
    cv::Mat outputFrame;
    //outputFrame *= 255;
    cv::cvtColor(merge_, outputFrame, CV_HSV2BGR);
    
    //outputFrame.convertTo(outputFrame, CV_8UC3);
    cv::imshow("output;", outputFrame);
    cvWaitKey(0);
    
    cv::waitKey(0);
    return 0;
}


