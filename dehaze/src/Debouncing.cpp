#include<opencv2/opencv.hpp>
#include<opencv2/videostab.hpp>
#include<string>
#include<iostream>

std::string inputPath = "F:\\Datasets\\Init ship Video\\20210518Video\\100_1_cut1.mp4";
std::string outputPath = "F:\\Datasets\\Init ship Video\\20210518Video\\100_1debouncing.mp4";

// 视频稳定输出
void videoOutput(cv::Ptr<cv::videostab::IFrameSource> stabFrames, std::string outputPath)
{
	cv::VideoWriter writer;
	cv::Mat stabFrame;
	//cv::imshow("frame0", stabFrames->nextFrame());
	//cv::waitKey(0);
	cv::Size frameSize = stabFrames->nextFrame().size();
	int nFrames = 0;
	// 设置帧率
	double outputFps = 25;
	// 遍历搜索视频帧
	writer.open(outputPath, cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), outputFps, frameSize);
	while (!(stabFrame = stabFrames->nextFrame()).empty())
	{
		nFrames++;
		// 输出视频稳定帧
		if (!outputPath.empty())
		{
			if (!writer.isOpened())
			{
				printf("%s", "Writed file opened failed");
				return;
			}
			writer << stabFrame;
		}
		cv::imshow("stabFrame", stabFrame);
		// esc 退出
		char key = static_cast<char>(cv::waitKey(100));
		if (key == 27)
		{
			std::cout << std::endl;
			break;
		}
		std::cout << "nFrames" << nFrames << std::endl;
		std::cout << "finished" << std::endl;
	}
	writer.release();

}

void cacStabVideo(cv::Ptr<cv::videostab::IFrameSource> stabFrames, std::string srcVideoFile)
{
	try
	{
		cv::Ptr<cv::videostab::VideoFileSource> srcVideo 
			= cv::makePtr<cv::videostab::VideoFileSource>(inputPath);
		std::cout << "frame count:" << srcVideo->count() << std::endl;

		// 运动估计
		double estPara = 0.1;
		cv::Ptr<cv::videostab::MotionEstimatorRansacL2> est 
			= cv::makePtr<cv::videostab::MotionEstimatorRansacL2>(cv::videostab::MM_AFFINE);

		// Ransac参数设置
		cv::videostab::RansacParams ransac = est->ransacParams();
		ransac.size = 3;
		ransac.thresh = 5;
		ransac.eps = 0.5;

		// ransac计算
		est->setRansacParams(ransac);
		est->setMinInlierRatio(estPara);

		// Fast 特征检测
		cv::Ptr<cv::FastFeatureDetector> feature_detector = cv::FastFeatureDetector::create();

		// 运动估计关键点匹配
		cv::Ptr<cv::videostab::KeypointBasedMotionEstimator> montionEstBuilder 
			= cv::makePtr<cv::videostab::KeypointBasedMotionEstimator>(est);

		// 设置特征检测器
		montionEstBuilder->setDetector(feature_detector);
		cv::Ptr<cv::videostab::IOutlierRejector> outlierRejector
			= cv::makePtr<cv::videostab::NullOutlierRejector>();
		montionEstBuilder->setOutlierRejector(outlierRejector);

		// prepare the stablizer
		cv::videostab::StabilizerBase* stabilizer = 0;

		// 1st, prepare the one or two pass stablizer
		bool isTwoPass = 1;
		int radius_pass = 5;
		if (isTwoPass)
		{
			// with a two pass stabilizer
			bool est_trim = true;
			cv::videostab::TwoPassStabilizer* twoPassStabilizer = new cv::videostab::TwoPassStabilizer();
			twoPassStabilizer->setEstimateTrimRatio(est_trim);
			twoPassStabilizer->setMotionStabilizer(
				cv::makePtr<cv::videostab::GaussianMotionFilter>(radius_pass));
			stabilizer = twoPassStabilizer;
		}
		else
		{
			// with an one pass stabilizer
			cv::videostab::OnePassStabilizer* onePassStabilizer = new cv::videostab::OnePassStabilizer();
			onePassStabilizer->setMotionFilter(
				cv::makePtr<cv::videostab::GaussianMotionFilter>(radius_pass));
			stabilizer = onePassStabilizer;
		}

		//second , set up the parameters
		int radius = 15;
		double trim_ratio = 0.1;
		bool incl_constr = false;
		stabilizer->setFrameSource(srcVideo);
		stabilizer->setMotionEstimator(montionEstBuilder);
		stabilizer->setRadius(radius);
		stabilizer->setTrimRatio(trim_ratio);
		stabilizer->setCorrectionForInclusion(incl_constr);
		stabilizer->setBorderMode(cv::BORDER_REPLICATE);

		// cast stabilizer to simple frame 
		stabFrames.reset(dynamic_cast<cv::videostab::IFrameSource*>(stabilizer));
		videoOutput(stabFrames, outputPath);
		// cv::imshow("stabFrames", stabFrames);
		//cv::imshow("s", )
	}
	catch (const std::exception &e)
	{
		std::cout << "error:" << e.what() << std::endl;
		stabFrames.release();
	}
	
	
}

int main00(int argc, char* arv[])
{
	cv::Ptr<cv::videostab::IFrameSource> stabFrames;

	cacStabVideo(stabFrames, inputPath);
	stabFrames.release();
	return 0;
}