#include "../include/videoStab01.h"
#include "../include/videoStab02.h"
#include "../include/videoStab03.h"
#include "../include/videoStab04.h"
#include "../include/opencvGpu.h"


std::string inputFolder = "E:\\Code\\ImagePreprocess\\ImagePreProcess\\videoStab\\data\\input\\104_0611_cut1.mp4";
std::string outpuFolder = "E:\\Code\\ImagePreprocess\\ImagePreProcess\\videoStab\\data\\output";
int kalmanFilter();

int main(int argc, char** argv)
{
	// stabilize01();
	//stabilize02(inputFolder, outpuFolder);
	//stabilize03(inputFolder, outpuFolder);
	//stabilize04(inputFolder, outpuFolder);
	//gputest01(inputFolder);
	stablize04Gpu(inputFolder, outpuFolder);
}

int kalmanFilter()
{
	return 0;

}