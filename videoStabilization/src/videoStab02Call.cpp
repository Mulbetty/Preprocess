
#include "../include/videoStab02.h"

using namespace std;
using namespace cv;

// This class redirects cv::Exception to our process so that we can catch it and handle it accordingly.
class cvErrorRedirector {
public:
	int cvCustomErrorCallback()
	{
		std::cout << "A cv::Exception has been caught. Skipping this frame..." << std::endl;
		return 0;
	}

	cvErrorRedirector() {
		cvRedirectError((cv::ErrorCallback)cvCustomErrorCallback(), this);
	}
};

const int HORIZONTAL_BORDER_CROP = 30;

int stabilize02(std::string inputPath_, std::string outputPath_)
{
	//std::string inputPath = "E:\\Code\\ImagePreprocess\\ImagePreProcess\\videoStab\\data\\input\\100_0611_1.mp4";
	//std::string inputPath = "E:\\Code\\ImagePreprocess\\ImagePreProcess\\videoStab\\data\\input\\Sam and Cocoa - shaky original.mp4";
	// std::string outputPath = "E:\\Code\\ImagePreprocess\\ImagePreProcess\\videoStab\\data\\output\\100_0611_1_stab2_output.mp4";
	//std::string outputPath = "E:\\Code\\ImagePreprocess\\ImagePreProcess\\videoStab\\data\\output\\Sam and Cocoa - shaky original-stab-KalmanFlowFilter.mp4";
	std::string inputPath = inputPath_;
	std::string outputPath = outputPath_;
	std::string fileName = inputPath_.substr(inputPath_.rfind('\\') + 1);
	if (outputPath.back() == '\\' | outputPath.back() == '/')
	{
		outputPath.pop_back();
	}
	std::string subfix = "_stab3_output.mp4";
	size_t pos = fileName.rfind('.');
	fileName = fileName.replace(pos, fileName.size() - pos, subfix);
    outputPath = outputPath + "\\" + fileName;
	cvErrorRedirector redir;

	//Create a object of stabilization class
	VideoStab stab;

	//Initialize the VideoCapture object
	// VideoCapture cap(0);
	VideoCapture cap(inputPath);
	if (_access(inputPath.c_str(), 00)==-1)
	{
		std::cout << "文件不存在" << std::endl;
	}
	Mat frame_2, frame2;
	Mat frame_1, frame1;

	if (!cap.isOpened())
	{
		std::cout << "文件打开失败" << std::endl;
	}
	// cap.read(frame_1);
	if (!cap.read(frame_1))
	{
		std::cout << "wrong input path" << std::endl;
		LPCTSTR errorHead("Error of input file or filepath");
		MessageBox(NULL, errorHead , "File Wrong", MB_OK);
		return 1;
	}
	cvtColor(frame_1, frame1, COLOR_BGR2GRAY);

	Mat smoothedMat(2, 3, CV_64F);

	VideoWriter outputVideo;
	cv::Size concatSize(frame_1.cols * 2 + 10, frame_1.rows);
	cv::Size frameSize(frame_1.cols>=1920? concatSize/2:concatSize);
	outputVideo.open(outputPath, CV_FOURCC('X', 'V', 'I', 'D'), 30, frameSize);
	int countFrame = 0;
	while (true)
	{
		try {
			cap >> frame_2;
			std::cout << countFrame++ << std::endl;;
			if (frame_2.data == NULL)
			{
				break;
			}

			cvtColor(frame_2, frame2, COLOR_BGR2GRAY);
			Mat concatFrame = Mat::zeros(concatSize, frame_1.type());
			Mat smoothedFrame;
			clock_t start = clock();
			smoothedFrame = stab.stabilize(frame_1, frame_2);
			clock_t end = clock();
			frame_1.copyTo(concatFrame(Range::all(), Range(0, frame_1.cols)));
			smoothedFrame.copyTo(concatFrame(Range::all(), Range(frame_1.cols + 10, concatSize.width)));
			char TextInfo[128] = { 0 };
			sprintf_s(TextInfo, "perFrame: %d,  fps: %f", end - start, 1000. / (end - start));
			cv::putText(concatFrame, TextInfo, cv::Point(100, 100), CV_FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 256, 128));
			if (frame_1.cols>=1920)
			{
				cv::resize(concatFrame, concatFrame, frameSize);
			}
			outputVideo.write(concatFrame);

			imshow("Stabilized Video", concatFrame);

			waitKey(10);

			frame_1 = frame_2.clone();
			frame2.copyTo(frame1);
		}
		catch (cv::Exception& e) {
			cap >> frame_1;
			cvtColor(frame_1, frame1, COLOR_BGR2GRAY);
		}

	}
	outputVideo.release();
	cap.release();

	return 0;
}
