#include "guidedFilter.hpp"
#include<opencv2/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/videoio.hpp>


cv::Mat GuildeFilter_1(cv::Mat& I, cv::Mat& p, int r, double eps) {
	int wsize = 2 * r + 1;
	//数据类型转换
	I.convertTo(I, CV_64F, 1.0 / 255.0);
	p.convertTo(p, CV_64F, 1.0 / 255.0);

	//meanI=fmean(I)
	cv::Mat mean_I;
	cv::boxFilter(I, mean_I, -1, cv::Size(wsize, wsize), cv::Point(-1, -1), true, cv::BORDER_REFLECT);//盒子滤波

																									  //meanP=fmean(P)
	cv::Mat mean_p;
	cv::boxFilter(p, mean_p, -1, cv::Size(wsize, wsize), cv::Point(-1, -1), true, cv::BORDER_REFLECT);//盒子滤波

																									  //corrI=fmean(I.*I)
	cv::Mat mean_II;
	mean_II = I.mul(I);
	cv::boxFilter(mean_II, mean_II, -1, cv::Size(wsize, wsize), cv::Point(-1, -1), true, cv::BORDER_REFLECT);//盒子滤波

																											 //corrIp=fmean(I.*p)
	cv::Mat mean_Ip;
	mean_Ip = I.mul(p);
	cv::boxFilter(mean_Ip, mean_Ip, -1, cv::Size(wsize, wsize), cv::Point(-1, -1), true, cv::BORDER_REFLECT);//盒子滤波

																											 //varI=corrI-meanI.*meanI
	cv::Mat var_I, mean_mul_I;
	mean_mul_I = mean_I.mul(mean_I);
	cv::subtract(mean_II, mean_mul_I, var_I);

	//covIp=corrIp-meanI.*meanp
	cv::Mat cov_Ip;
	cv::subtract(mean_Ip, mean_I.mul(mean_p), cov_Ip);

	//a=conIp./(varI+eps)
	//b=meanp-a.*meanI
	cv::Mat a, b;
	cv::divide(cov_Ip, (var_I + eps), a);
	cv::subtract(mean_p, a.mul(mean_I), b);

	//meana=fmean(a)
	//meanb=fmean(b)
	cv::Mat mean_a, mean_b;
	cv::boxFilter(a, mean_a, -1, cv::Size(wsize, wsize), cv::Point(-1, -1), true, cv::BORDER_REFLECT);//盒子滤波
	cv::boxFilter(b, mean_b, -1, cv::Size(wsize, wsize), cv::Point(-1, -1), true, cv::BORDER_REFLECT);//盒子滤波

																									  //q=meana.*I+meanb
	cv::Mat q;
	q = mean_a.mul(I) + mean_b;

	//数据类型转换
	I.convertTo(I, CV_8U, 255);
	p.convertTo(p, CV_8U, 255);
	q.convertTo(q, CV_8U, 255);

	return q;

}

int mainGuidedFilter1()
{
	cv::Mat src = cv::imread("E:\\Code\\ImageDehazing\\scindapsus\\berman2016non-local\\dehaze2\\x64\\Release\\01.jpg",1);
	if (src.empty())
	{
		return -1;
	}

	double t2 = (double)cv::getTickCount(); // 获得时间，用于计时
	cv::Mat dst1, src_input, I;
	src.copyTo(src_input);
	if (src.channels() > 1)
	{
		cv::cvtColor(src, I, CV_RGB2GRAY); // 转换成灰度图，作为引导图片
	}
	std::vector<cv::Mat> p, q;
	if (src.channels() > 1)
	{
		cv::split(src_input, p);
		for (int i = 0;i < src.channels(); ++i)
		{
			dst1 = GuildeFilter_1(I, p[i], 5, 0.1*0.1);
			q.push_back(dst1);
		}
		cv::merge(q, dst1);
	}
	else
	{
		src.copyTo(I);
		dst1 = GuildeFilter_1(I, src_input, 5, 0.1*0.1);

	}
	t2 = (double)cv::getTickCount() - t2;
	double time2 = (t2 *1000.) / ((double)cv::getTickFrequency());
	std::cout << "MyGuidedFilter_process=" << time2 << " ms. " << std::endl << std::endl;
	cv::namedWindow("GuidedImg", CV_WINDOW_NORMAL);
	cv::imshow("GuidedImg", I);
	cv::namedWindow("src", CV_WINDOW_NORMAL);
	cv::imshow("src", src);
	cv::namedWindow("GuidedFilter_box", CV_WINDOW_NORMAL);
	cv::imshow("GuidedFilter_box", dst1);
	cv::waitKey(0);
}
