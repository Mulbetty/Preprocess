#include "../include/opencvGpu.h"

//第一次测试用时平均每帧10ms

static void download(const cuda::GpuMat& d_mat, vector<Point2f>& vec)
{
	vec.resize(d_mat.cols);
	Mat mat(1, d_mat.cols, d_mat.type(), (void*)&vec[0]);
	d_mat.download(mat);
}
static void download(const cuda::GpuMat& d_mat, vector<uchar>& vec)
{
	vec.resize(d_mat.cols);
	Mat mat(1, d_mat.cols, CV_8UC1, (void*)&vec[0]);
	d_mat.download(mat);
}
int gputest01(std::string inputPath)
{
	int num_devices = cv::cuda::getCudaEnabledDeviceCount();
	std::cout << num_devices << std::endl;

	cv::VideoCapture cap(inputPath);
	cv::Mat frame;
	cv::Mat frameGray;
	cv::cuda::GpuMat gpuFrameGray;
	while (cap.read(frame))
	{
		clock_t cputime0 = clock();
		cv::cvtColor(frame, frameGray, cv::COLOR_BGR2GRAY);
		clock_t cputime1 = clock();
		std::cout << "cpu用时：" << cputime1 - cputime0 << std::endl;
		
		cv::cuda::GpuMat gpuFrame(frame);
		clock_t gputime0 = clock();
		cv::cuda::cvtColor(gpuFrame, gpuFrameGray, cv::COLOR_BGR2GRAY);
		clock_t gputime1 = clock();
		std::cout << "gpu用时" << gputime1 - gputime0 << std::endl;
		//cv::imshow("gray1", frameGray);
		//cv::imshow("gray2", gpuFrameGray);
	}
	system("pause");
	return 1;
}

int stablize04Gpu(std::string inputPath_, std::string outputPath_)
{
	//std::string inputPath = "E:\\Code\\ImagePreprocess\\ImagePreProcess\\videoStab\\data\\input\\Sam and Cocoa - shaky original.mp4";
	//std::string inputPath = "E:\\Code\\ImagePreprocess\\ImagePreProcess\\videoStab\\data\\input\\100_0611_1.mp4";
	//std::string outputPath = "E:\\Code\\ImagePreprocess\\ImagePreProcess\\videoStab\\data\\output\\Sam and Cocoa - shaky original-stab_04-1.mp4";
	//std::string outputPath = "E:\\Code\\ImagePreprocess\\ImagePreProcess\\videoStab\\data\\output\\100_0611_1_stab4_output.mp4";
	std::string inputPath = inputPath_;
	std::string outputPath = outputPath_;
	std::string fileName = inputPath_.substr(inputPath_.rfind('\\') + 1);
	if (outputPath.back() == '\\' | outputPath.back() == '/')
	{
		outputPath.pop_back();
	}
	std::string subfix = "_stab4_GpuOutput.mp4";
	size_t pos = fileName.rfind('.');
	fileName = fileName.replace(pos, fileName.size() - pos, subfix);
	outputPath = outputPath + "\\" + fileName;
	// For further analysis
	ofstream out_transform("prev_to_cur_transformation.txt");
	ofstream out_trajectory("trajectory.txt");
	ofstream out_smoothed_trajectory("smoothed_trajectory.txt");
	ofstream out_new_transform("new_prev_to_cur_transformation.txt");

	VideoCapture cap(inputPath);
	assert(cap.isOpened());


	Mat cur, cur_grey;
	Mat prev, prev_grey;

	cap >> prev;//get the first frame.ch
	cv::cuda::GpuMat prevGpu(prev), pre_greyGpu;
	cv::cvtColor(prev, prev_grey, CV_BGR2GRAY);
	cuda::cvtColor(prevGpu, pre_greyGpu, COLOR_BGR2GRAY);
	// Step 1 - Get previous to current frame transformation (dx, dy, da) for all frames
	vector <TransformParam> prev_to_cur_transform; // previous to current
	// Accumulated frame to frame transform
	double a = 0;
	double x = 0;
	double y = 0;
	// Step 2 - Accumulate the transformations to get the image trajectory
	vector <Trajectory> trajectory; // trajectory at all frames
	//
	// Step 3 - Smooth out the trajectory using an averaging window
	vector <Trajectory> smoothed_trajectory; // trajectory at all frames
	Trajectory X;//posteriori state estimate
	Trajectory	X_;//priori estimate
	Trajectory P;// posteriori estimate error covariance
	Trajectory P_;// priori estimate error covariance
	Trajectory K;//gain
	Trajectory	z;//actual measurement
	double pstd = 4e-3;//can be changed
	double cstd = 0.25;//can be changed
	Trajectory Q(pstd, pstd, pstd);// process noise covariance
	Trajectory R(cstd, cstd, cstd);// measurement noise covariance 
	// Step 4 - Generate new set of previous to current transform, such that the trajectory ends up being the same as the smoothed trajectory
	vector <TransformParam> new_prev_to_cur_transform;
	//
	// Step 5 - Apply the new transformation to the video
	//cap.set(CV_CAP_PROP_POS_FRAMES, 0);
	Mat T(2, 3, CV_64F);

	int vert_border = HORIZONTAL_BORDER_CROP * prev.rows / prev.cols; // get the aspect ratio correct
	VideoWriter outputVideo;
	if (!cap.read(cur))
	{
		MessageBox(NULL, "Wrong to read video file, please check your input file", "Wrong Message", 1);
	}
	cv::Size cursize(cur.cols * 2 + 10, cur.rows);
	if (cur.cols >= 1920)
	{
		cursize = cursize / 2;
	}
	if (!outputVideo.open(outputPath, CV_FOURCC('D', 'I', 'V', 'X'), 25, cursize))
	{
		MessageBox(NULL, "Open output file failed, Something error occurs.", "Write file failed", 1);
		std::cout << "打开文件时出现错误" << std::endl;
	}
	//
	int k = 1;
	int max_frames = cap.get(CV_CAP_PROP_FRAME_COUNT);
	Mat last_T;
	Mat prev_grey_, cur_grey_;
	// curGpu replace cur, cur_greyGpu replace cur_grey
	cv::cuda::GpuMat curGpu, cur_greyGpu;

	while (true) {

		cap >> cur;
		if (cur.data == NULL) {
			break;
		}
		// 将读取的图像转换成GPU格式的
		curGpu.upload(cur); // download: 将gpuMat下载到cpu； upload将cpuMat上传到gpuMat
		// gpu 实现颜色转换
		cuda::cvtColor(curGpu, cur_greyGpu, COLOR_BGR2GRAY);

		cuda::GpuMat prevCornerGpu, curCornerGpu, outputStatusGpu; // 角点，1行n列
		// 角点 Mat
		cv::Mat preCorner; 
		// vector from prev to cur
		vector <Point2f> prev_corner, cur_corner;
		vector <Point2f> prev_corner2, cur_corner2;
		vector <uchar> status;
		vector <float> err;
		clock_t startTime = clock();
		pre_greyGpu.upload(prev_grey);
		// 将goodFeaturesToTrack()函数修改为cuda加速的代码
		//goodFeaturesToTrack(prev_grey, prev_corner, 200, 0.01, 30);
		auto cornerDetection = cuda::createGoodFeaturesToTrackDetector(pre_greyGpu.type(), 200, 0.01, 30);
		cornerDetection->detect(pre_greyGpu, prevCornerGpu);
		prevCornerGpu.download(preCorner);
		std::cout << "cpu corners: "<<preCorner.size() << std::endl;
		std::cout << prevCornerGpu.size() << std::endl;
		
		auto opticalFlowDet = cuda::FarnebackOpticalFlow::create();
		auto sparseOFDetector = cuda::SparsePyrLKOpticalFlow::create();
		auto denseOFDetector = cuda::DensePyrLKOpticalFlow::create();
		
		sparseOFDetector->calc(pre_greyGpu, cur_greyGpu, prevCornerGpu, curCornerGpu, outputStatusGpu);
		cur_greyGpu.download(cur_grey);
		// 将计算结果中corners结果保存到向量中
		vector<Point2f> prePts(prevCornerGpu.cols);
		download(prevCornerGpu, prePts);
		vector<Point2f> nextPts(curCornerGpu.cols);
		download(curCornerGpu, nextPts);
		vector<uchar> status_(outputStatusGpu.cols);
		download(outputStatusGpu, status_);
	
		std::cout << (prev_corner == prePts) << std::endl;
		//calcOpticalFlowPyrLK(prev_grey, cur_grey, prev_corner, cur_corner, status, err);
		/*for (int i = 0; i < 100; i++)
		{
			cv::Point2f pt1 = prev_corner[i];
			cv::Point2f pt2 = prePts[i];
			cv::Point2f pt3 = cur_corner[i];
			cv::Point2f pt4 = nextPts[i];
			if (!(pt1.x == pt2.x && pt1.y == pt2.y&& pt3.x==pt4.x&&pt3.y==pt4.y))
			{
				return 0;
			}
		}*/
		// weed out bad matches
		for (size_t i = 0; i < status_.size(); i++) {
			// std::cout << status_[i] << std::endl;
			if (status_[i]) {
				prev_corner2.push_back(prePts[i]);
				cur_corner2.push_back(nextPts[i]);
			}
		}

		// translation + rotation only
		Mat T = estimateRigidTransform(prev_corner2, cur_corner2, false); // false = rigid transform, no scaling/shearing

		// in rare cases no transform is found. We'll just use the last known good transform.
		if (T.data == NULL) {
			last_T.copyTo(T);
		}

		T.copyTo(last_T);

		// decompose T
		double dx = T.at<double>(0, 2);
		double dy = T.at<double>(1, 2);
		double da = atan2(T.at<double>(1, 0), T.at<double>(0, 0));
		//
		//prev_to_cur_transform.push_back(TransformParam(dx, dy, da));

		out_transform << k << " " << dx << " " << dy << " " << da << endl;
		//
		// Accumulated frame to frame transform
		x += dx;
		y += dy;
		a += da;
		//trajectory.push_back(Trajectory(x,y,a));
		//
		out_trajectory << k << " " << x << " " << y << " " << a << endl;
		//
		z = Trajectory(x, y, a);
		//
		if (k == 1) {
			// intial guesses
			X = Trajectory(0, 0, 0); //Initial estimate,  set 0
			P = Trajectory(1, 1, 1); //set error variance,set 1
		}
		else
		{
			//time update（prediction）
			X_ = X; //X_(k) = X(k-1);
			P_ = P + Q; //P_(k) = P(k-1)+Q;
			// measurement update（correction）
			K = P_ / (P_ + R); //gain;K(k) = P_(k)/( P_(k)+R );
			X = X_ + K * (z - X_); //z-X_ is residual,X(k) = X_(k)+K(k)*(z(k)-X_(k)); 
			P = (Trajectory(1, 1, 1) - K) * P_; //P(k) = (1-K(k))*P_(k);
		}
		//smoothed_trajectory.push_back(X);
		out_smoothed_trajectory << k << " " << X.x << " " << X.y << " " << X.a << endl;
		//-
		// target - current
		double diff_x = X.x - x;//
		double diff_y = X.y - y;
		double diff_a = X.a - a;

		dx = dx + diff_x;
		dy = dy + diff_y;
		da = da + diff_a;

		//new_prev_to_cur_transform.push_back(TransformParam(dx, dy, da));
		//
		out_new_transform << k << " " << dx << " " << dy << " " << da << endl;
		//
		T.at<double>(0, 0) = cos(da);
		T.at<double>(0, 1) = -sin(da);
		T.at<double>(1, 0) = sin(da);
		T.at<double>(1, 1) = cos(da);

		T.at<double>(0, 2) = dx;
		T.at<double>(1, 2) = dy;

		cuda::GpuMat TGpu(T);

		Mat cur2;
		//cuda::GpuMat cur2Gpu, preGpu2(prev);
		
		cv::warpAffine(prev, cur2, T, cur.size());
		
		//cuda::warpAffine(preGpu2, cur2Gpu, T, curGpu.size());
		//cur2Gpu.download(cur2);
		clock_t endTime = clock();
		std::cout << "处理用时： " << endTime - startTime << std::endl;

		cur2 = cur2(Range(vert_border, cur2.rows - vert_border), Range(HORIZONTAL_BORDER_CROP, cur2.cols - HORIZONTAL_BORDER_CROP));

		// Resize cur2 back to cur size, for better side by side comparison
		cv::resize(cur2, cur2, cur.size());

		// Now draw the original and stablised side by side for coolness
		Mat canvas = Mat::zeros(cur.rows, cur.cols * 2 + 10, cur.type());

		prev.copyTo(canvas(Range::all(), Range(0, cur2.cols)));
		cur2.copyTo(canvas(Range::all(), Range(cur2.cols + 10, cur2.cols * 2 + 10)));

		// If too big to fit on the screen, then scale it down by 2, hopefully it'll fit :)
		if (canvas.cols >= 1920) {
			cv::resize(canvas, canvas, Size(canvas.cols / 2, canvas.rows / 2));
		}
		outputVideo << canvas;
		cv::imshow("before and after", canvas);

		cv::waitKey(10);
		//
		prev = cur.clone();//cur.copyTo(prev);
		cur_grey.copyTo(prev_grey);

		std::cout << "Frame: " << k << "/" << max_frames << " - good optical flow: " << prev_corner2.size() << endl;
		k++;

	}
	outputVideo.release();
	cap.release();
	return 0;
}
