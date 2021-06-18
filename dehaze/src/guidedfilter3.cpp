#include<opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
//#include<iostream>
using namespace cv;

/*
ʹ�õ����˲�ʵ��ȥ���㷨��
*/

typedef struct Pixel
{
	int x;
	int y;
	int value;
}Pixel;

inline bool cmp(Pixel a, Pixel b)
{
	return a.value>b.value;
}

//��Сֵ�˲�
//dark_img = minfliter(dark_img, windowsize);
Mat minfilter(Mat& D, int windowsize) {
	int dr = D.rows;
	int dc = D.cols;
	int r = (windowsize - 1) / 2; //windowsize������
	Mat dark_img(dr, dc, CV_8UC1);  //��ͨ��ͼ
	dark_img = D.clone();
	for (int i = r;i <= dr - r - 1;i++) {
		for (int j = r;j <= dc - r - 1;j++) {
			int min = 255;
			for (int m = i - r;m <= i + r;m++) {
				for (int n = j - r;n <= j + r;n++) {
					if (D.at<uchar>(m, n) < min)
						min = D.at<uchar>(m, n);
				}
			}
			dark_img.at<uchar>(i, j) = min;
		}
	}
	//std::cout <<"��ͨ��ͼ��"<< dark_img;
	imshow("��ͨ��ͼ", dark_img);
	return dark_img;
}

//��ͨ���ļ���
Mat Producedarkimg(Mat& I, int windowsize)
{
	int min = 255;
	Mat dark_img(I.rows, I.cols, CV_8UC1);  //��ͨ��
	int radius = (windowsize - 1) / 2;
	int nr = I.rows; // number of rows    
	int nc = I.cols;
	int b, g, r;
	if (I.isContinuous()) { //��ͼ����ͨʱ�����ǾͿ��԰�ͼ����ȫչ����������һ��
		nc = nr * nc;
		nr = 1;
	}
	for (int i = 0;i<nr;i++)
	{
		const uchar* inData = I.ptr<uchar>(i);
		uchar* outData = dark_img.ptr<uchar>(i);
		for (int j = 0;j<nc;j++)
		{
			b = *inData++;
			g = *inData++;
			r = *inData++;
			min = min>b ? b : min;  //Ѱ����ͨ���е���Сֵ
			min = min>g ? g : min;
			min = min>r ? r : min;
			*outData++ = min;
			min = 255;
		}
	}
	dark_img = minfilter(dark_img, windowsize);
	return dark_img;
}

//���������ֵA
int* getatmospheric_light(Mat& darkimg, Mat& srcimg, int windowsize)
{
	int radius = (windowsize - 1) / 2;
	int nr = darkimg.rows, nl = darkimg.cols;
	int darksize = nr*nl;
	int topsize = darksize / 1000;   //��ͨ��ͼ��0.1%�������صĸ���
	int *A = new int[3];
	int sum[3] = { 0,0,0 };
	Pixel *toppixels, *allpixels;
	toppixels = new Pixel[topsize];
	allpixels = new Pixel[darksize];


	for (int i = 0;i<nr;i++) {
		const uchar* outData = darkimg.ptr<uchar>(i);
		for (int j = 0;j<nl;j++)
		{
			allpixels[i*nl + j].value = *outData++;
			allpixels[i*nl + j].x = i;
			allpixels[i*nl + j].y = j;
		}
	}
	//std::qsort(allpixels,darksize,sizeof(Pixel),qcmp);  
	std::sort(allpixels, allpixels + darksize, cmp);

	memcpy(toppixels, allpixels, (topsize) * sizeof(Pixel)); //�ҵ�����darkimg��������0.1%��  

	int val0, val1, val2, avg, max = 0, maxi, maxj, x, y;
	for (int i = 0;i<topsize;i++)
	{
		x = allpixels[i].x;y = allpixels[i].y;  //��ͨ�����������ص�����
		const uchar* outData = srcimg.ptr<uchar>(x);
		outData += 3 * y;
		val0 = *outData++;
		val1 = *outData++;
		val2 = *outData++;
		avg = (val0 + val1 + val2) / 3;
		if (max<avg) { max = avg;maxi = x;maxj = y; }
	}
	for (int i = 0;i<3;i++)
	{
		A[i] = srcimg.at<Vec3b>(maxi, maxj)[i];
		//A[i]=srcimg.at<Vec4b>(maxi,maxj)[i];  
		//A[i]=A[i]>220?220:A[i];  
	}
	return A;  //����A��3��b,g,r����
}

//����ͼ�˲�
Mat guidedFilter(Mat& transmission, Mat& graymat, Mat& trans, int windowsize, double ap) {
	windowsize = 6 * windowsize;
	Mat mean_P(transmission.rows, transmission.cols, CV_32FC1);
	Mat mean_I(graymat.rows, graymat.cols, CV_32FC1);
	Mat corr_I(graymat.rows, graymat.cols, CV_32FC1);
	Mat corr_IP(graymat.rows, graymat.cols, CV_32FC1);
	Mat var_I(graymat.rows, graymat.cols, CV_32FC1);
	Mat cov_IP(graymat.rows, graymat.cols, CV_32FC1);
	Mat a(graymat.rows, graymat.cols, CV_32FC1);
	Mat b(graymat.rows, graymat.cols, CV_32FC1);
	Mat mean_a(graymat.rows, graymat.cols, CV_32FC1);
	Mat mean_b(graymat.rows, graymat.cols, CV_32FC1);
	blur(transmission, mean_P, Size(windowsize, windowsize));
	blur(graymat, mean_I, Size(windowsize, windowsize));
	corr_I = graymat.mul(graymat);
	corr_IP = graymat.mul(transmission);
	var_I = corr_I - mean_I.mul(mean_I);
	cov_IP = corr_IP - mean_I.mul(mean_P);
	a = cov_IP / (var_I + ap);
	b = mean_P - a.mul(mean_I);
	blur(a, mean_a, Size(windowsize, windowsize));
	blur(b, mean_b, Size(windowsize, windowsize));
	trans = mean_a.mul(graymat) + mean_b;
	return trans;
}

//����͸��ͼt����ϸ��
Mat getTransmission_dark(Mat& srcimg, Mat& darkimg, int *array, int windowsize)
{
	float test;
	float avg_A = (array[0] + array[1] + array[2]) / 3.0;
	// float w = 0.95; //
	float w = 0.6; //
	int radius = (windowsize - 1) / 2;
	int nr = srcimg.rows, nl = srcimg.cols;
	Mat transmission(nr, nl, CV_32FC1);

	for (int k = 0;k<nr;k++) {
		const uchar* inData = darkimg.ptr<uchar>(k);
		for (int l = 0;l<nl;l++)
		{
			transmission.at<float>(k, l) = 1 - w*(*inData++ / avg_A);
		}
	}
	Mat trans(nr, nl, CV_32FC1);
	Mat graymat(nr, nl, CV_8UC1);
	Mat graymat_32F(nr, nl, CV_32FC1);
	cvtColor(srcimg, graymat, CV_BGR2GRAY);
	for (int i = 0;i<nr;i++) {
		const uchar* inData = graymat.ptr<uchar>(i);
		for (int j = 0;j<nl;j++)
			graymat_32F.at<float>(i, j) = *inData++ / 255.0;
	}
	guidedFilter(transmission, graymat_32F, trans, 6 * windowsize, 0.001);


	return trans;
}

//����J��X��
Mat recover(Mat& srcimg, Mat& t, int *array, int windowsize)
{
	int test;
	int radius = (windowsize - 1) / 2;
	int nr = srcimg.rows, nl = srcimg.cols;
	float tnow = t.at<float>(radius, radius);
	float t0 = 0.1;
	Mat finalimg = Mat::zeros(nr, nl, CV_8UC3);
	int val = 0;
	for (int i = 0;i<3;i++) {
		for (int k = radius;k<nr - radius;k++) {
			const float* inData = t.ptr<float>(k);  inData += radius;
			const uchar* srcData = srcimg.ptr<uchar>(k);  srcData += radius * 3 + i;
			uchar* outData = finalimg.ptr<uchar>(k);  outData += radius * 3 + i;
			for (int l = radius;l<nl - radius;l++)
			{
				tnow = *inData++;
				tnow = tnow>t0 ? tnow : t0;
				val = (int)((*srcData - array[i]) / tnow + array[i]);
				srcData += 3;
				val = val<0 ? 0 : val;
				*outData = val>255 ? 255 : val;
				outData += 3;
			}
		}

	}
	return finalimg;
}

// �������
void raiseLight(cv::Mat &image1, float alpha, float beta) {

	cv::Mat gammaImg;
	cv::Mat image = image1;

	// imshow("���⴦����1", img_equalist);
	for (int i = 0;i<image1.rows;i++)
		for (int j = 0; j < image1.cols; j++) {
			for (int k = 0; k < 3; k++) {
				int tmp = (uchar)image1.at<Vec3b>(i, j)[k] * alpha + beta;
				if (tmp > 255)
					image1.at<Vec3b>(i, j)[k] = 2 * 255 - tmp;
				else
					image1.at<Vec3b>(i, j)[k] = tmp;
			}
		}

}

int mainGuidedFilter3() {
	int windowsize = 25;
	Mat srcimg = imread("E:\\Code\\ImageDehazing\\scindapsus\\berman2016non-local\\dehaze2\\x64\\Release\\01.jpg");
	namedWindow("ԭͼ", 0);
	imshow("ԭͼ", srcimg);
	Mat darkimg(srcimg.rows, srcimg.cols, CV_8UC1);
	Mat trans(srcimg.rows, srcimg.cols, CV_32FC1);
	Mat pic(srcimg.rows, srcimg.cols, CV_8UC3);
	int *A = new int[3];

	//��ͨ���ļ���
	clock_t start, end;
	start = clock();
	darkimg = Producedarkimg(srcimg, windowsize);
	//���������ֵA
	A = getatmospheric_light(darkimg, srcimg, windowsize);
	//����͸��ͼt����ϸ��
	trans = getTransmission_dark(srcimg, darkimg, A, windowsize);
	//����J��X��
	pic = recover(srcimg, trans, A, windowsize);
	// �������
	// raiseLight(pic, 1., 1.8);
	//��ʾȥ����ͼ��
	namedWindow("ȥ���", 0);
	imshow("ȥ���", pic);
	end = clock();
	std::cout << end - start << std::endl;
	waitKey(0);
}