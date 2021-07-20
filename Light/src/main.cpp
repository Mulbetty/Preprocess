#include "../include/adaptiveGamma.h"

int main(int argc, char *argv[])
{
    // std::string inputPath("E:\\Code\\ImagePreprocess\\ImagePreProcess\\Light\\data\\input\\02.jpg");
    std::string inputPath("C:\\Users\\ASUS\\Desktop\\02qq.jpg");
    std::string outputPath("E:\\Code\\ImagePreprocess\\ImagePreProcess\\Light\\data\\input");
    cv::Mat frame = cv::imread(inputPath);
    cv::namedWindow("before", 0);
    cv::imshow("before", frame);
    //adaptiveGama(inputPath, outputPath);
    cv::Mat result = work(frame);
    // cv::namedWindow("after", 0);
    cv::imshow("after", result);
    cv::waitKey(0);
    return 0;
}
