#include<opencv2/opencv.hpp>
#include<opencv2/videoio.hpp>
#include<string>

// # brief\ 视频截取代码 \n
// para:
//     @fileName->string: 要截取的文件名
//     @cutTime->size_t: 要截取的时间，按s来算
void cutVideo(std::string fileName, size_t cutTime, std::string outputFilename)
{
	cv::VideoCapture videoReader;
	cv::VideoWriter videoWriter;
	videoReader.open(fileName);
	if (!videoReader.isOpened())
	{
		std::cout << "Reading Wrong" << std::endl;
	}
	size_t height = videoReader.get(cv::CAP_PROP_FRAME_HEIGHT);
	std::cout << "height:" << height << std::endl;
	size_t width = videoReader.get(cv::CAP_PROP_FRAME_WIDTH);
	std::cout << "width:" << width << std::endl;
	float fps = videoReader.get(cv::CAP_PROP_FPS);
	std::cout << "fps:" << fps << std::endl;
	// auto fourcc = videoReader.get(CV_FOURCC);
	size_t frameCount = videoReader.get(cv::CAP_PROP_POS_FRAMES);
	size_t cutTime_ = cutTime < frameCount ? cutTime : frameCount;
	
	videoWriter.open(outputFilename, CV_FOURCC('X', 'V', 'I', 'D'), fps, cv::Size(width, height));
	assert(videoWriter.isOpened());
	cv::Mat frameNow;
	for (int i=0;i<cutTime;i++)
	{
		videoReader.read(frameNow);
		videoWriter<<frameNow;
	}
	videoReader.release();
	videoWriter.release();
}

int call()
{
	std::string inputFileName = "F:\\Datasets\\Init ship Video\\20210612Video\\192.168.10.104_01_20210611090557771.mp4";
	std::string outputFileName = "F:\\Datasets\\Init ship Video\\20210612Video\\192.168.10.104_01_20210611090557771_cut1.mp4";
	size_t countCut = 1000;
	cutVideo(inputFileName, countCut, outputFileName);
	return 0;
}
