#include"dehaze3.h"
#include<direct.h>
#include<opencv2/opencv.hpp>
#define div_255_fast(x) (((x) + 1 + (((x) + 1) >> 8)) >> 8)
int dehazeVideo() // 去雾算法处理视频
{
	clock_t startTime, endTime;
	double omega = 0.5; // 0.95
	double numt = 0.3; //0.3
	int rectSize = 15; //15
	cv::VideoCapture cap("F:\\Datasets\\Init ship Video\\20210518Video\\100_1.mp4");
	cv::Mat frame;
	const int frameHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
	const int frameWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
	cv::Mat dst;cv::Mat dst1(frameHeight, frameWidth, CV_32FC3);
	while (cap.read(frame))
	{
		// cv::resize(frame, frame, cv::Size(), 0.5, 0.5);
		startTime = clock();
		ImageDefogging(frame, dst, rectSize, omega, numt);
		// frame.copyTo(dst);
		endTime = clock();
		std::string putText;
		stringstream text;
		text << "cost:" << endTime - startTime;
		text >> putText;
		cv::putText(frame, putText, cv::Point(50, 50), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 255, 0));
		//namedWindow("src", 0);
		//cv::imshow("src", frame);
		// ------------------
		// dst /= 255;
		cv::normalize(dst, dst, 1, 0, CV_MINMAX);
		namedWindow("dst", 0);
		cv::imshow("dst", dst);
		cv::waitKey(1);
	}
	return 0;
}

int guideFilter1()
{
	double omega = 0.5; // 0.95
	double numt = 0.3; //0.3
	int rectSize = 15; //15

	clock_t startTime, endTime;
	startTime = clock();//计时开始
	char pathNow[80] = { 0 };
	_getcwd(pathNow, sizeof(pathNow));
	std::cout << pathNow << std::endl;
	//Mat src = imread("../x64/Release/1.png");
	Mat src = imread("E:\\Code\\ImageDehazing\\scindapsus\\berman2016non-local\\dehaze2\\x64\\Release\\01.jpg");
	cv::imshow("src", src);
	cv::waitKey(1);
}
 // 视频去雾测试
int main03() 
{
	dehazeVideo();
	system("pause");
}
int mainDehaze3()
{
	//效果参数
	double omega = 0.5; // 0.95
	double numt = 0.4; //0.3
	int rectSize = 15; //15

	// clock_t startTime, endTime;
	// startTime = clock();//计时开始
	char pathNow[80] = { 0 };
	_getcwd(pathNow, sizeof(pathNow));
	std::cout << pathNow << std::endl;
	//Mat src = imread("../x64/Release/1.png");
	Mat src = imread("E:\\Code\\ImageDehazing\\scindapsus\\berman2016non-local\\dehaze2\\x64\\Release\\01.jpg");
	// cv::cv2eigen()
	//cv::resize(src, src, cv::Size(512, 512));
	cv::Mat downImg1;
	src.copyTo(downImg1);
	// cv::pyrDown(src, downImg1, cv::Size(960, 540));
	// cv::resize(src, downImg1, cv::Size(512, 512));
	Mat dst;//(src.rows, src.cols, CV_32FC3)
	Mat dst1(src.rows, src.cols, CV_32FC3);
	clock_t startTime, endTime;
	startTime = clock();//计时开始

	ImageDefogging(downImg1, dst, rectSize, omega, numt);

	

	imwrite("dst.jpg", dst);
	imwrite("dst1.jpg", dst1);

	/*float scale = 0.15;
	resize(src, src, Size(src.cols*scale, src.rows*scale));
	resize(dst, dst, Size(dst.cols*scale, dst.rows*scale));
	resize(dst1, dst1, Size(dst1.cols*scale, dst1.rows*scale));*/
	std::cout << dst.at<cv::Vec3f>(24, 23)[0] << std::endl;
	dst /= 255;
	normalize(dst, dst, 1, 0, CV_MINMAX);
	std::cout << dst.at<cv::Vec3f>(24, 23)[0] << std::endl;
	dst1 /= 255;
	// cv::pyrUp(downImg1, src, cv::Size(1920, 1080));
	// cv::pyrUp(dst, dst, cv::Size(1920, 1080));
//	cv::resize(dst, dst, cv::Size(1920, 1080));
	endTime = clock();				//计时结束
	cout << "The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
	cvNamedWindow("src", 0);
	imshow("src", src);
	cv::namedWindow("dst", 0);
	imshow("dst", dst);
	imshow("dst1", dst1);

	waitKey();
	return 0;
}

void ImageDefogging(Mat src, Mat& dst, int rectSize, double omega, double numt)
{
	//对原图进行归一化
	Mat I;
	src.convertTo(I, CV_32F);
	// ----------------------------
	//I /= 255;
	normalize(I, I, 1, 0, CV_MINMAX);

	float A[3] = { 0 };
	Mat dark = DarkChannel(I, rectSize);
	AtmLight(I, dark, A);

	Mat te = TransmissionEstimate(I, A, rectSize, omega);
	Mat t = TransmissionRefine(src, te);
	dst = Defogging(I, t, A, numt);
}

Mat DarkChannel(Mat srcImg, int size)
{
	vector<Mat> chanels;
	split(srcImg, chanels);

	//求RGB三通道中的最小像像素值
	Mat minChannel = (cv::min)((cv::min)(chanels[0], chanels[1]), chanels[2]);
	Mat kernel = getStructuringElement(MORPH_RECT, Size(size, size));

	Mat dark(minChannel.rows, minChannel.cols, CV_32FC1);
	erode(minChannel, dark, kernel);	//图像腐蚀
	return dark;
}

template<typename T> vector<int> argsort(const vector<T>& array)
{
	const int array_len(array.size());
	vector<int> array_index(array_len, 0);
	for (int i = 0; i < array_len; ++i)
		array_index[i] = i;

	sort(array_index.begin(), array_index.end(),
		[&array](int pos1, int pos2) {return (array[pos1] < array[pos2]); });

	return array_index;
}

void AtmLight(Mat src, Mat dark, float outA[3])
{
	int row = src.rows;
	int col = src.cols;
	int imgSize = row*col;

	//将暗图像和原图转为列向量
	vector<int> darkVector = dark.reshape(1, imgSize);
	Mat srcVector = src.reshape(3, imgSize);

	//按照亮度的大小取前0.1%的像素（亮度高）
	int numpx = int(max(floor(imgSize / 1000), 1.0));
	vector<int> indices = argsort(darkVector);
	vector<int> dstIndices(indices.begin() + (imgSize - numpx), indices.end());

	for (int i = 0; i < numpx; ++i)
	{
		outA[0] += srcVector.at<Vec3f>(dstIndices[i], 0)[0];
		outA[1] += srcVector.at<Vec3f>(dstIndices[i], 0)[1];
		outA[2] += srcVector.at<Vec3f>(dstIndices[i], 0)[2];
	}
	// 取平均
	outA[0] /= numpx;
	outA[1] /= numpx;
	outA[2] /= numpx;

}

Mat TransmissionEstimate(Mat src, float outA[3], int size, float omega)
{
	Mat imgA = Mat::zeros(src.rows, src.cols, CV_32FC3);

	vector<Mat> chanels;
	split(src, chanels);
	for (int i = 0; i < 3; ++i)
	{
		chanels[i] = chanels[i] / outA[i];
	}

	merge(chanels, imgA);
	Mat transmission = 1 - omega*DarkChannel(imgA, size);	//计算透射率预估值
	return transmission;
}

Mat Guidedfilter(Mat src, Mat te, int r, float eps)
{
	Mat meanI, meanT, meanIT, meanII, meanA, meanB;
	boxFilter(src, meanI, CV_32F, Size(r, r));
	boxFilter(te, meanT, CV_32F, Size(r, r));
	boxFilter(src.mul(te), meanIT, CV_32F, Size(r, r));
	Mat covIT = meanIT - meanI.mul(meanT);

	boxFilter(src.mul(src), meanII, CV_32F, Size(r, r));
	Mat varI = meanII - meanI.mul(meanI);

	Mat a = covIT / (varI + eps);
	Mat b = meanT - a.mul(meanI);
	boxFilter(a, meanA, CV_32F, Size(r, r));
	boxFilter(b, meanB, CV_32F, Size(r, r));

	Mat t = meanA.mul(src) + meanB;

	return t;
}

Mat TransmissionRefine(Mat src, Mat te)
{
	Mat gray;
	cvtColor(src, gray, CV_BGR2GRAY);
	gray.convertTo(gray, CV_32F);
	gray /= 255;

	int r = 60;
	float eps = 0.0001;
	Mat t = Guidedfilter(gray, te, r, eps);
	return t;
}

Mat Defogging(Mat src, Mat t, float outA[3], float tx)
{
	Mat dst = Mat::zeros(src.rows, src.cols, CV_32FC3);
	t = (cv::max)(t, tx);				//设置阈值当投射图t 的值很小时，会导致图像整体向白场过度

	vector<Mat> chanels;
	split(src, chanels);
	for (int i = 0; i < 3; ++i)
	{
		chanels[i] = (chanels[i] - outA[i]) / t + outA[i];
	}
	merge(chanels, dst);

	dst *= 255;				//归一化还原
	return dst;
}

void GetFiles(string path, vector<string>& files, string fileType)
{
	intptr_t   hFile = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\" + fileType).c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					GetFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);

		//文件排序
		sort(files.begin(), files.end(),
			[](const string &a, const string &b) {
			return a.length() < b.length() || a.length() == b.length() && a < b;
		});
	}
}

