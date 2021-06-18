// video_stabilization.cpp : ���ļ����� "main" ����������ִ�н��ڴ˴���ʼ��������
//

//#include "pch.h"
#include "../include/videoStab05.h"

/**
 * @brief �켣�ۻ�
 *
 * @param transforms �˶���Ϣ�ṹ��
 * @return vector<Trajectory> �켣�ṹ��
 */
vector<Trajectory> cumsum(vector<TransformParam>& transforms)
{
	// trajectory at all frames ����֡���˶��켣
	vector<Trajectory> trajectory;
	// Accumulated frame to frame transform �ۼӼ���x,y�Լ�a���Ƕȣ�
	double a = 0;
	double x = 0;
	double y = 0;

	//�ۼ�
	for (size_t i = 0; i < transforms.size(); i++)
	{
		x += transforms[i].dx;
		y += transforms[i].dy;
		a += transforms[i].da;

		trajectory.push_back(Trajectory(x, y, a));
	}

	return trajectory;
}

/**
 * @brief ƽ���˶��켣
 *
 * @param trajectory �˶��켣
 * @param radius �����С
 * @return vector<Trajectory>
 */
vector<Trajectory> smooth(vector<Trajectory>& trajectory, int radius)
{
	//ƽ������˶��켣
	vector<Trajectory> smoothed_trajectory;
	//�ƶ���������
	for (size_t i = 0; i < trajectory.size(); i++)
	{
		double sum_x = 0;
		double sum_y = 0;
		double sum_a = 0;
		int count = 0;

		for (int j = -radius; j <= radius; j++)
		{
			if (i + j >= 0 && i + j < trajectory.size())
			{
				sum_x += trajectory[i + j].x;
				sum_y += trajectory[i + j].y;
				sum_a += trajectory[i + j].a;

				count++;
			}
		}

		double avg_a = sum_a / count;
		double avg_x = sum_x / count;
		double avg_y = sum_y / count;

		smoothed_trajectory.push_back(Trajectory(avg_x, avg_y, avg_a));
	}

	return smoothed_trajectory;
}

/**
 * @brief
 *
 * @param frame_stabilized
 */
void fixBorder(Mat& frame_stabilized)
{
	//��ԭͼ����Ϊ1.04����Ȼ���ȡԭͼ�ߴ���ȴ�С����
	Mat T = getRotationMatrix2D(Point2f(frame_stabilized.cols / 2, frame_stabilized.rows / 2), 0, 1.04);
	//����任
	warpAffine(frame_stabilized, frame_stabilized, T, frame_stabilized.size());
}

int stablize05(string inputPath_, string outputPath_)
{
	// Read input video ��ȡ��Ƶ
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
	VideoCapture cap(inputPath);

	// Get frame count ��ȡ��Ƶ��֡��
	int n_frames = int(cap.get(CAP_PROP_FRAME_COUNT));
	// Our test video may be wrong to read the frame after frame 1300
	n_frames = n_frames>1000?1000:n_frames;

	// Get width and height of video stream ��ȡ��Ƶͼ����
	int w = int(cap.get(CAP_PROP_FRAME_WIDTH));
	int h = int(cap.get(CAP_PROP_FRAME_HEIGHT));

	// Get frames per second (fps) ��ȡ��Ƶÿ��֡��
	double fps = cap.get(CV_CAP_PROP_FPS);

	// Set up output video ���������Ƶ
	VideoWriter out("video_out.avi", CV_FOURCC('M', 'J', 'P', 'G'), fps, Size(2 * w, h));

	// Define variable for storing frames ����洢֡����ر���
	//��ǰ֡RGBͼ��ͻҶ�ͼ
	Mat curr, curr_gray;
	//ǰһ֡RGBͼ��ͻҶ�ͼ
	Mat prev, prev_gray;

	// Read first frame �����Ƶһ��ͼ��
	cap >> prev;

	// Convert frame to grayscale ת��Ϊ�Ҷ�ͼ
	cvtColor(prev, prev_gray, COLOR_BGR2GRAY);

	// Pre-define transformation-store array ����仯�����ṹ��
	vector<TransformParam> transforms;

	//previous transformation matrix ��һ��ͼ��ķ������
	Mat last_T;
	//�ӵڶ�֡��ʼѭ��������Ƶ����֡
	for (int i = 1; i < n_frames; i++)
	{
		// Vector from previous and current feature points ǰһ֡�ǵ�vector����ǰ֡�ǵ�vector
		vector<Point2f> prev_pts, curr_pts;

		// Detect features in previous frame ��ȡǰһ֡�Ľǵ�
		//ǰһ֡�Ҷ�ͼ��ǰһ֡�ǵ�vector, ���ǵ�������⵽�Ľǵ�������ȼ��������ǵ�֮�����С����
		goodFeaturesToTrack(prev_gray, prev_pts, 200, 0.01, 30);

		// Read next frame ��ȡ��ǰ֡ͼ��
		bool success = cap.read(curr);
		if (!success)
		{
			break;
		}

		// Convert to grayscale ����ǰ֡ͼ��ת��Ϊ�Ҷ�ͼ
		cvtColor(curr, curr_gray, COLOR_BGR2GRAY);

		// Calculate optical flow (i.e. track feature points) ������׷Ѱ������
		//���״̬ʸ��(Ԫ�����޷���char���ͣ�uchar)������ڵ�ǰ֡����ǰһ֡�ǵ���������Ϊ1������Ϊ0
		vector<uchar> status;
		//������ʸ��
		vector<float> err;
		//��������
		//ǰһ֡�Ҷ�ͼ�񣬵�ǰ֡�Ҷ�ͼ��ǰһ֡�ǵ㣬��ǰ֡�ǵ㣬״̬���������
		calcOpticalFlowPyrLK(prev_gray, curr_gray, prev_pts, curr_pts, status, err);

		// Filter only valid points ��ȡ������������Ч�Ľǵ�
		//�����ǵ�
		auto prev_it = prev_pts.begin();
		auto curr_it = curr_pts.begin();
		for (size_t k = 0; k < status.size(); k++)
		{
			if (status[k])
			{
				prev_it++;
				curr_it++;
			}
			//ɾ����Ч�ǵ�
			else
			{
				prev_it = prev_pts.erase(prev_it);
				curr_it = curr_pts.erase(curr_it);
			}
		}

		// Find transformation matrix ��ñ任����
		//false��ʾ������Լ���ķ���任��true����ȫ����仯��TΪ�任����
		Mat T = estimateRigidTransform(prev_pts, curr_pts, false);

		// In rare cases no transform is found.
		// We'll just use the last known good transform.
		//������������Ҳ����任����ȡ��һ���任Ϊ��ǰ�仯����
		//��Ȼ��һ�μ���û�ҵ���������㷨������⣬�������ʺܵ�
		if (T.data == NULL)
		{
			last_T.copyTo(T);
		}
		T.copyTo(last_T);

		// Extract traslation ��ȡ����仯���
		double dx = T.at<double>(0, 2);
		double dy = T.at<double>(1, 2);

		// Extract rotation angle ��ȡ�Ƕ�
		double da = atan2(T.at<double>(1, 0), T.at<double>(0, 0));

		// Store transformation �洢����仯����
		transforms.push_back(TransformParam(dx, dy, da));

		// Move to next frame ������һ�μ��׼��
		curr_gray.copyTo(prev_gray);

		cout << "Frame: " << i << "/" << n_frames << " -  Tracked points : " << prev_pts.size() << endl;
	}

	// Compute trajectory using cumulative sum of transformations ��ȡ�ۼӹ켣
	vector<Trajectory> trajectory = cumsum(transforms);

	// Smooth trajectory using moving average filter ��ȡƽ����Ĺ켣
	vector<Trajectory> smoothed_trajectory = smooth(trajectory, SMOOTHING_RADIUS);

	//ƽ������˶���Ϣ�ṹ��
	vector<TransformParam> transforms_smooth;

	//ԭʼ�˶���Ϣ�ṹ��
	for (size_t i = 0; i < transforms.size(); i++)
	{
		// Calculate difference in smoothed_trajectory and trajectory ����ƽ����Ĺ켣��ԭʼ�켣����
		double diff_x = smoothed_trajectory[i].x - trajectory[i].x;
		double diff_y = smoothed_trajectory[i].y - trajectory[i].y;
		double diff_a = smoothed_trajectory[i].a - trajectory[i].a;

		// Calculate newer transformation array ����ƽ������˶���Ϣ�ṹ������
		double dx = transforms[i].dx + diff_x;
		double dy = transforms[i].dy + diff_y;
		double da = transforms[i].da + diff_a;

		transforms_smooth.push_back(TransformParam(dx, dy, da));
	}

	//��λ��ǰ֡Ϊ��1֡
	cap.set(CV_CAP_PROP_POS_FRAMES, 0);
	//ƽ����ı仯����
	Mat T(2, 3, CV_64F);
	Mat frame, frame_stabilized, frame_out;

	//������֡���б仯�õ�������
	//������һ֡
	cap.read(frame);
	for (int i = 0; i < n_frames - 1; i++)
	{
		bool success = cap.read(frame);
		if (!success)
		{
			break;
		}
		// Extract transform from translation and rotation angle. ��ȡƽ����ķ���仯����
		transforms_smooth[i].getTransform(T);

		// Apply affine wrapping to the given frame Ӧ�÷���仯
		warpAffine(frame, frame_stabilized, T, frame.size());

		// Scale image to remove black border artifact ȥ���ڱ�
		fixBorder(frame_stabilized);

		// Now draw the original and stablised side by side for coolness ��ԭͼ�ͱ仯���ͼ���������������Ƶ
		hconcat(frame, frame_stabilized, frame_out);

		// If the image is too big, resize it.
		if (frame_out.cols > 1920)
		{
			resize(frame_out, frame_out, Size(frame_out.cols / 2, frame_out.rows / 2));
		}

		//imshow("Before and After", frame_out);
		out.write(frame_out);
		cout << "out frame��" << i << endl;
		//waitKey(10);
	}

	// Release video
	cap.release();
	out.release();
	// Close windows
	destroyAllWindows();

	return 0;
}