#include <opencv2/opencv>

// img1&trans是要融合的两个图像， dst是结果， start是横向拼接起始位置，为0时，两张图像完全融合
void OptimizeSeamHor(cv::Mat& img1, cv::Mat& trans, cv::Mat& dst, int start)
{

    double processWidth = img1.cols - start;//重叠区域的宽度
    int rows = dst.rows;
    int cols = img1.cols; 
    double alpha = 1;//img1中像素的权重
    for (int i = 0; i < rows; i++)
    {
        uchar* p = img1.ptr<uchar>(i);  
        uchar* t = trans.ptr<uchar>(i);
        uchar* d = dst.ptr<uchar>(i);
        for (int j = start; j < cols; j++)
        {
            //如果遇到图像trans中无像素的黑点，则完全拷贝img1中的数据
            if (t[j * 3] == 0 && t[j * 3 + 1] == 0 && t[j * 3 + 2] == 0)
            {
                alpha = 1;
            }
            else
            {
                alpha = (processWidth - (j - start)) / processWidth;
            }
            int indexTemp = j * 3;
            int j_transIndex = (j - start)*3;
            d[indexTemp] = p[indexTemp] * alpha + t[j_transIndex] * (1 - alpha);
            d[indexTemp + 1] = p[indexTemp + 1] * alpha + t[j_transIndex + 1] * (1 - alpha);
            d[indexTemp + 2] = p[indexTemp + 2] * alpha + t[j_transIndex + 2] * (1 - alpha);
        }
        img1(cv::Rect(0, 0, start, rows)).copyTo(dst(cv::Rect(0, 0, start, rows)));
        trans(cv::Rect(cols - start, 0,start, rows)).copyTo(dst(cv::Rect(img1.cols, 0, start, rows)));
    }

}


void OptimizeSeamVer(cv::Mat& img1, cv::Mat& img2, cv::Mat& dst, int start)
{
    if (img1.empty() || img2.empty())
    {
        return;
    }
    else
    {
        //printf("Seaming ...\n");
    }
    double processWidth = img1.rows - start;//重叠区域的宽度
    int rows = img1.rows;
    int cols = dst.cols; //注意，是列数*通道数
    double alpha = 1;//img1中像素的权重
    for (int rowIndex = start; rowIndex < rows; rowIndex++) // 遍历列
    {
        // 获取当前列的首地址
        uchar* p = img1.ptr<uchar>(rowIndex);
        uchar* t = img2.ptr<uchar>(rowIndex-start);
        uchar* d = dst.ptr<uchar>(rowIndex);
        // 再获取行的坐标
        float alpha0 = (processWidth - (rowIndex - start)) / processWidth;
        for (int colIndex = 0; colIndex < cols; colIndex++)
        {
            int colIndex3N = colIndex * 3;
            if (t[colIndex3N] == 0 && t[colIndex3N + 1] == 0 && t[colIndex3N + 2] == 0)
            {
                alpha = 1;
            }
            else
            {
                alpha = alpha0;
            }
            d[colIndex3N] = p[colIndex3N] * alpha + t[colIndex3N] * (1 - alpha);
            d[colIndex3N+1] = p[colIndex3N+1] * alpha + t[colIndex3N+1] * (1 - alpha);
            d[colIndex3N+2] = p[colIndex3N+2] * alpha + t[colIndex3N+2] * (1 - alpha);
        }
        // 将融合之外的部分填充到dst中。
        img1(cv::Rect(0, 0, cols, start)).copyTo(dst(cv::Rect(0, 0, cols, start)));
        img2(cv::Rect(0, rows-start, cols, start)).copyTo(dst(cv::Rect(0, img1.rows, cols, start)));
    }
}