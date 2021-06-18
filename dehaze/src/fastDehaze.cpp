#pragma once
#include "fastDehaze.h"
using namespace cv;
using std::cout;
using std::endl;
using std::vector;


//��Сֵ�˲���
void min_filter(cv::Mat &src_img, cv::Mat &res_img, int kernel_size)
{
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2 * kernel_size - 1, 2 * kernel_size - 1));
	cv::erode(src_img, res_img, element);
	//cv::imshow("dark_channel", res_img);
}


//��ȡͨ������Сֵ����ͨ��
cv::Mat min_BGR(cv::Mat &src_img)
{
	int rows = src_img.rows;
	int cols = src_img.cols;

	cv::Mat res = cv::Mat::zeros(cv::Size(cols, rows), CV_8UC1);
	//creat one channel image to save min_rgb
	int channel_num = src_img.channels();
	/*std::cout << channel_num << std::endl;*/
	if (channel_num <= 1)
	{
		std::cout << "channel is just 1, return zeros matrix!" << std::endl;
		return res;
	}


	for (int i = 0; i < rows; i++)
	{
		uchar *img_adress = src_img.ptr<uchar>(i);   //��ǰ�еĵ�ַ
		uchar *res_adress = res.ptr<uchar>(i);       //��ǰ�еĵ�ַ
		for (int j_i = 0, j_r = 0; j_r < cols, j_i<cols*channel_num; j_i += channel_num, j_r++)
		{
			res_adress[j_r] = std::min(std::min(img_adress[j_i], img_adress[j_i + 1]), img_adress[j_i + 2]);
		}

	}
	//std::cout << "done!"<<std::endl;
	return res;
}

//cv::Mat  min_BRG_32F(cv::Mat &img_32F)
//{
//	int rows = img_32F.rows;
//	int cols = img_32F.cols;
//
//	Mat res = Mat::zeros(Size(cols,rows), CV_32FC1);
//	//cout << img_32F.size()<<endl;
//	//cout << res.size() << endl;
//
//	for (int i = 0; i < rows; i++)
//	{
//		for (int j = 0; j < cols; j++)
//		{
//	
//			res.at<float>(i, j) = std::min(img_32F.at<Vec2f>(i, j)[0], img_32F.at<Vec2f>(i, j)[1]);
//		}
//	}
//	return res;
//}

//��ȡͨ�������ֵ����ͨ����
cv::Mat max_BGR(cv::Mat &src_img)
{
	int rows = src_img.rows;
	int cols = src_img.cols;

	cv::Mat res = cv::Mat::zeros(cv::Size(cols, rows), CV_8UC1);
	//creat one channel image to save min_rgb
	int channel_num = src_img.channels();
	/*std::cout << channel_num << std::endl;*/
	if (channel_num <= 1)
	{
		std::cout << "channel is just 1, return zeros matrix!" << std::endl;
		return res;
	}

	for (int i = 0; i < rows; i++)
	{
		uchar *img_adress = src_img.ptr<uchar>(i);   //��ǰ�еĵ�ַ
		uchar *res_adress = res.ptr<uchar>(i);       //��ǰ�еĵ�ַ
		for (int j_i = 0, j_r = 0; j_r < cols, j_i<cols*channel_num; j_i += channel_num, j_r++)
		{
			res_adress[j_r] = std::max(std::max(img_adress[j_i], img_adress[j_i + 1]), img_adress[j_i + 2]);
		}

	}
	//std::cout << "done!"<<std::endl;
	return res;
}

cv::Mat getLx_A(cv::Mat &M_min, cv::Mat &img, float p, int kernel_size, float &A)
{
	cv::Mat M_minfilter;

	blur(M_min, M_minfilter, Size(kernel_size, kernel_size));    //��ֵ�˲�
	Scalar m_av = cv::mean(M_min);              //��ͨ���ľ�ֵ������Ӧ���ǶԵ�

	float m_ave = m_av[0] / 255.0;                 //��ֵ������һ��Scalar,ȡ��һ������
	float m = std::min(m_ave*p, float(0.9));         //ȡ����������Сֵ
													 //cout << "m" <<m<< endl;             //����û���⣬���Թ���
													 //����ļ���ǣ������ม�����㣬��������ת����32F������

	cv::Mat M_minfilter_32f, M_min_32f;
	M_min.convertTo(M_min_32f, CV_32FC1);
	M_minfilter.convertTo(M_minfilter_32f, CV_32F);
	//�ĳ�32F����ȥ��������0.9��p*m_av����Сֵ

	//cout <<"��Сֵ�˲���" <<M_minfilter_32f(Range(0, 9), Range(0, 9));
	//cout << "��Сֵ����" << M_min_32f(Range(0, 9), Range(0, 9));
	//cout << M_minfilter_32f(Range(0, 9), Range(0, 9))*m;

	//vector<Mat> M_vector = { M_minfilter_32f*m,M_min_32f };

	////M_vector.push_back(M_minfilter_32f*m);
	////M_vector.push_back(M_min_32f);


	//Mat M_merge;
	//merge(M_vector, M_merge);
	//Mat res = min_BRG_32F(M_merge);     //ר�������������ȡ��Сֵ

	//��Ϊֻ��һ�Σ��������ﲻ��vector�ˣ��ٶ���һ���������
	M_minfilter_32f = M_minfilter_32f*m;

	int rows = M_minfilter_32f.rows;
	int cols = M_minfilter_32f.cols;
	cv::Mat res = cv::Mat::zeros(cv::Size(cols, rows), CV_32FC1);

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{

			res.at<float>(i, j) = std::min(M_minfilter_32f.at<float>(i, j), M_min_32f.at<float>(i, j));
		}
	}

	cv::Mat max_img = max_BGR(img);
	double max_max_img, max_ave_img;
	minMaxLoc(max_img, NULL, &max_max_img, NULL, NULL);
	minMaxLoc(M_minfilter, NULL, &max_ave_img, NULL, NULL);
	A = 0.5*(max_max_img + max_ave_img);
	return res;
}



//����һ�����ұ���С��255*255����[0-255][0-255]��������
//���ҷ�ʽΪ��  Table[(H[X]<<8)+L(X)]   �����ǳ˷�
vector<vector<uchar>> Function7_table(const float &A)
{
	vector<vector<uchar>> Table = vector<vector<uchar>>(256, vector<uchar>(256, 0));
	//cout << Table.size() << endl;
	double Value = 0;
	clock_t start = clock();
	for (int Y = 0; Y < 256; Y++)
	{
		for (int X = 0; X < 256; X++)
		{
			// Value = (Y - X) / (1 - X / A);       //function_7
			Value = A * (Y - X) / (A - X);
			if (Value > 255)
				Value = 255;
			else if (Value < 0)
				Value = 0;

			Table[Y][X] = Value;
			//printf("%d\t", Table[Y][X]);  ���Ǹ���ӣ�cout���uchar��ʱ���ǰ���ascii������ģ�һ��ʼ����Ϊ����
		}
	}
	clock_t end = clock();
	std::cout << "function7 time use:" << end - start << std::endl;
	return Table;
}

cv::Mat FastDehaze(Mat &Img, const double p, const int kernel_size, float eps)
{
	Mat Lx, ImgDark;
	ImgDark = min_BGR(Img);
	Mat ImgDark_32f, Img_32F;
	//Img.convertTo(Img_32F, CV_32F);
	ImgDark.convertTo(ImgDark_32f, CV_32F);
	float A;
	Lx = getLx_A(ImgDark, Img, p, kernel_size, A);

	//cout <<"A:" <<A << endl;
	//cout << Lx(Range(0,9),Range(0,9)) << endl;

	vector<vector<uchar>> Table;
	Table = Function7_table(A);

	Mat ImgDefog = Img.clone();

	int row_Num = ImgDark_32f.rows;
	int col_Num = ImgDark_32f.cols;
	//�������
	for (int i = 0; i < row_Num; i++)
	{
		for (int j = 0; j < col_Num; j++)
		{
			ImgDefog.at<Vec3b>(i, j)[0] = Table[Img.at<Vec3b>(i, j)[0]][int(Lx.at<float>(i, j))];
			ImgDefog.at<Vec3b>(i, j)[1] = Table[Img.at<Vec3b>(i, j)[1]][int(Lx.at<float>(i, j))];
			ImgDefog.at<Vec3b>(i, j)[2] = Table[Img.at<Vec3b>(i, j)[2]][int(Lx.at<float>(i, j))];
			//��������ͨ��
		}
	}
	return ImgDefog;
}

// ����ȥ���㷨������ͼƬ
int mainFast()
{
	// Mat img = imread("E:\\Code\\ImageDehazing\\scindapsus\\berman2016non-local\\dehaze2\\x64\\Release\\01.jpg");
	cv::Mat img = imread("F:\\Datasets\\VOCdevkit\\202104251753.jpg");
	cout << img.size() << endl;
	namedWindow("source_img", 2);
	namedWindow("defog", 2);
	imshow("source_img", img);
	cv::resize(img, img, cv::Size(512, 512));
	double start, end;
	int count = 5;
	//while (count--)
	clock_t startTime, endTime;
	startTime = clock();
	{
		start = static_cast<double>(getTickCount());
		auto mat = FastDehaze(img, 1.3, 15);
		cout << ((double)getTickCount() - start) / getTickFrequency() << endl;
		imshow("defog", mat);
		endTime = clock();
		std::cout << "time used:" << endTime - startTime << std::endl;
		waitKey(100);
		cv::resize(mat, img, cv::Size(1920, 1080));
		cv::imshow("resized Image", mat);
	}
	
	

	//imshow("defog", mat);

	waitKey();
	return 0;
}