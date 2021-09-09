#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>
#include "device_types.h"
#include <assert.h>
#include "helper_cuda.h"
#include "helper_functions.h"
#include "cudaMelt.cuh"
#include <opencv2/opencv.hpp>


__global__ void matrixMeltTable(unsigned char* A, unsigned char* B, unsigned char* C, int start,
int wA, int wB, int size,  float* alphaTable)
{
	// block index
	int bx = blockIdx.x;
	int by = blockIdx.y;

	// thread index
	int tx = threadIdx.x;
	int ty = threadIdx.y;

	int idx = (by*gridDim.x+blockIdx.x)*blockDim.x*blockDim.y+threadIdx.y*blockDim.x+threadIdx.x;
	if (idx > size)
	{
		return;
	}
	float alpha = alphaTable[idx/3];
	int idxIndex = idx / 3;

	int alphaIdx = idxIndex % 10;
	
	//printf("alpha[%d] is %f\n", idx, alpha);
	//C[idx] = A[idx] * alphaTable[alphaIdx] + B[idx] * (1 - alpha);
	C[idx] = A[idx]*alphaTable[alphaIdx]+ (int)B[idx];
	printf("��ǰ�߳�����idxΪ��%d, ��õ�alphaIdxΪ��%d, A[%d]��ֵΪ��%d, C[%d]��ֵΪ%d, alphaTable[alphaIdx] = %f\n", idxIndex, alphaIdx,idx, A[idx], idx, C[idx], alphaTable[alphaIdx]);
}

__global__ void matrixMeltHor(unsigned char* A, unsigned char* B, unsigned char* C,
	                        int wA, int wB, int size)
{
	// block index
	int bx = blockIdx.x;
	int by = blockIdx.y;

	// thread index
	int tx = threadIdx.x;
	int ty = threadIdx.y;

	int idx = (by * gridDim.x + blockIdx.x) * blockDim.x * blockDim.y + threadIdx.y * blockDim.x + threadIdx.x;
	if (idx > size)
	{
		return;
	}
	//float alpha = alphaTable[idx / 3];
	int idxIndex = idx / 3;

	int alphaIdx = idxIndex % 10;

	//printf("alpha[%d] is %f\n", idx, alpha);
	//C[idx] = A[idx] * alphaTable[alphaIdx] + B[idx] * (1 - alpha);
	float alpha = alphaIdx*1.0/wA;
	C[idx] = A[idx] * alpha + B[idx]*(1-alpha);
	//printf("��ǰ�߳�����idxΪ��%d, ��õ�alphaIdxΪ��%d, A[%d]��ֵΪ��%d, C[%d]��ֵΪ%d, alpha = %f\n", idxIndex, alphaIdx, idx, A[idx], idx, C[idx], alpha);

}

__global__ void matrixMeltVer(uchar3* A, uchar3* B, uchar3* C,
	                          int wA, int hA, int size)
{
	// block index
	int bx = blockIdx.x;
	int by = blockIdx.y;

	// thread index
	int tx = threadIdx.x;
	int ty = threadIdx.y;

	int idx = (by * gridDim.x + blockIdx.x) * blockDim.x * blockDim.y + threadIdx.y * blockDim.x + threadIdx.x;
	if (idx > size)
	{
		return;
	}
	//float alpha = alphaTable[idx / 3];

	int alphaIdx = idx / wA;

	//printf("alpha[%d] is %f\n", idx, alpha);
	//C[idx] = A[idx] * alphaTable[alphaIdx] + B[idx] * (1 - alpha);
	float alpha = alphaIdx * 1.0 / hA;
	// C[idx] = A[idx] * alpha + B[idx] * (1 - alpha);
	C[idx].x = A[idx].x + B[idx].x;
	C[idx].y = A[idx].y + B[idx].y;
	C[idx].z = A[idx].z + B[idx].z;
	//printf("��ǰ�߳�����idxΪ��%d, ��õ�alphaIdxΪ��%d, A[%d]��ֵΪ��%d, C[%d]��ֵΪ%d, alpha = %f\n", idxIndex, alphaIdx, idx, A[idx], idx, C[idx], alpha);

}


// \brief: ʵ��ͼ������ֱ�����ϵĽ����ںϡ�����ʵ�ֻҶ�ͼ�Ͷ�ͨ��ͼ�񡣶�ͨ����ʱ��length = wA*hA*channels
// \para: img1, img2���������ͼ��imgres�������� wA, hA�ֱ���ͼ��Ŀ�͸ߣ� length�����ݳ���
__global__ void MeltVer(cv::cuda::PtrStepSz<uchar> img1, cv::cuda::PtrStepSz<uchar>  img2, cv::cuda::PtrStepSz<uchar>  imgres, int wA, int hA, int length) {
	int tx = blockIdx.x * blockDim.x + threadIdx.x;
	int ty = blockIdx.y * blockDim.y + threadIdx.y;
	if (tx >= length ||ty >= hA)
	{
		return;
	}
	float alpha = 1.0*ty/hA;
	int did = ty*imgres.step + tx;
	int sid1= ty*img1.step + tx;
	int sid2 = ty*img2.step + tx;
	imgres[did] = img1[sid1]*(1-alpha) + img2[sid2]*(alpha);
		// printf("%d   ", tid);
	
}

// \brief: ʵ��ͼ���ں������ϵĽ����ںϡ�����ʵ�ֻҶ�ͼ�Ͷ�ͨ��ͼ�񡣶�ͨ����ʱ��length = wA*hA*channels
// \para: img1, img2���������ͼ��imgres�������� wA, hA�ֱ���ͼ��Ŀ�͸ߣ� length�����ݳ���
__global__ void MeltHor(cv::cuda::PtrStepSz<uchar> img1, cv::cuda::PtrStepSz<uchar>  img2, cv::cuda::PtrStepSz<uchar>  imgres, int wA, int hA, int length) {
	int tx = blockIdx.x * blockDim.x + threadIdx.x;
	int ty = blockIdx.y * blockDim.y + threadIdx.y;
	if (tx >= length || ty >= hA)
	{
		return;
	}
	float alpha = 1.0 * tx /3/ wA;
	int did = ty * imgres.step + tx;
	int sid1 = ty * img1.step + tx;
	int sid2 = ty * img2.step + tx;
	imgres[did] = img1[sid1] * (1 - alpha) + img2[sid2] * (alpha);
	// printf("%d   ", tid);
}

// \brief: ����������uchar3���͵��ںϺ���
// __global__ void MeltHorU3(uchar3* img1, uchar3* img2, uchar3* imgres, int wA, int hA, int length) {
__global__ void MeltHorU3(cv::cuda::PtrStepSz<uchar3> img1, cv::cuda::PtrStepSz<uchar3>  img2, cv::cuda::PtrStep<uchar3> imgres, int wA, int hA, int length, int step) {
	int tid = blockIdx.z * (gridDim.x * gridDim.y) * (blockDim.x * blockDim.y * blockDim.z) \
		+ blockIdx.y * gridDim.x * (blockDim.x * blockDim.y * blockDim.z) \
		+ blockIdx.x * (blockDim.x * blockDim.y * blockDim.z) \
		+ threadIdx.z * (blockDim.x * blockDim.y) \
		+ threadIdx.y * blockDim.x \
		+ threadIdx.x;
	float alpha = 1.0 * tid / hA / wA;
	int steps = img1.step;
	int tx = threadIdx.x + blockIdx.x*blockDim.x;
	int ty = threadIdx.y + blockIdx.y*blockDim.y;
	if (tx < wA && ty<hA) {
		int row = tid / wA;
		// did ��ʾ�������ݵ���������
		// int did = (row*steps)/sizeof(uchar3) + tid % wA;
		// printf("Step is :%d\n", steps);
		int did = ty*steps/3 + tx;
		int rid = ty * imgres.step/3 + tx;
		imgres.data[rid].z = img1.data[did].z * (1 - alpha) + img2.data[did].z * (alpha);
		imgres.data[rid].x = img1.data[did].x * (1 - alpha) + img2.data[did].x * (alpha);
		imgres.data[rid].y = img1.data[did].y * (1 - alpha) + img2.data[did].y * (alpha);
	}
}

// \brief: ʵ������ͼ�������غϲ��ֵ��ںϣ�����������ͼ����ȫ�ںϡ�ͼ��Ϊ8UC3��ʽ
// \para: mat1, mat2������Ҫ�غϵ�ͼ��,ע����GPU�ϵ����ݣ� width��height�ֱ���ͼ��Ŀ�Ⱥ͸߶ȣ� channels��ͨ����
// �����blocksizeĬ��������32��������Ը���ʵ��blockSize�������ã�
int matMeltHor(u_char * mat1, u_char * mat2, u_char * dst, int width, int height, int channels)
{
	dim3 threadsPerBlock(32, 32);
	int gridCols = (width * channels + 32 - 1) / threadsPerBlock.x;
	int gridRows = (height + 32 - 1) / threadsPerBlock.y;
	dim3 blocksPerGrid(gridCols, gridRows);
	int size = width * height * channels;
	matrixMeltHor << <blocksPerGrid, threadsPerBlock >> > (mat1, mat2, dst, width, width, size);
	HANDLE_ERROR(cudaGetLastError());
	return 0;
}

// \brief: ʵ������ͼ�������غϲ��ֵ��ںϣ�����������ͼ����ȫ�ںϡ������ںϵ�ͼ����8UC3��ͼ������
// \para: mat1, mat2������Ҫ�غϵ�ͼ��ע����GPU�ϵ����ݣ� width��height�ֱ���ͼ��Ŀ�Ⱥ͸߶ȣ� channels��ͨ����
// �����blocksizeĬ��������32��������Ը���ʵ��blockSize�������ã�
// NOTE:����δ�ɹ����д��Ż�
int matMeltVer(u_char* mat1, u_char* mat2, u_char* dst, int width, int height, int channels)
{
	dim3 threadsPerBlock(32, 32);
	uchar3* dev_A;
	uchar3* dev_B;
	uchar3* dev_C;
	int mallocSize = width*height*3*sizeof(u_char);
	cudaMalloc((void**)&dev_A, mallocSize);
	cudaMalloc((void**)&dev_B, mallocSize);
	cudaMalloc((void**)&dev_C, mallocSize);
	cudaMemcpy(dev_A, mat1, mallocSize, cudaMemcpyDeviceToDevice);
	cudaMemcpy(dev_B, mat2, mallocSize, cudaMemcpyDeviceToDevice);
	// cudaMemcpy(dev_A, mat1, mallocSize, cudaMemcpyDeviceToDevice);
	int gridCols = (width*channels+32-1)/threadsPerBlock.x;
	int gridRows = (height+32-1)/threadsPerBlock.y;
	dim3 blocksPerGrid(gridCols, gridRows);
	int size = width * height * channels;
	matrixMeltVer<<<blocksPerGrid, threadsPerBlock>>>(dev_A, dev_B, dev_C,  width, height, width*height);
	HANDLE_ERROR(cudaGetLastError());
	cudaMemcpy(dst, dev_C, mallocSize, cudaMemcpyDeviceToDevice);
	return 0;
}

// \brief: ʵ������ͼ�������غϲ��ֵ��ںϣ�����������ͼ����ȫ�ںϡ������ںϵ�ͼ����8UC3��ͼ������
// \para: mat1, mat2������Ҫ�غϵ�ͼ��ע����GPU�ϵ�����,���ݸ�ʽע�⣻ width��height�ֱ���ͼ��Ŀ�Ⱥ͸߶ȣ�
// �����blocksizeĬ��������32��������Ը���ʵ��blockSize�������ã�
int cudaMeltVer(cv::cuda::GpuMat mat1, cv::cuda::GpuMat mat2, cv::cuda::GpuMat dst, int width, int height, int channels)
{
	int length = width * channels;
	dim3 blockrgb(32, 32);
	dim3 gridrgb((length + blockrgb.x - 1) / blockrgb.x, (height + blockrgb.y - 1) / blockrgb.y);
	MeltVer << <gridrgb, blockrgb >> > (mat1, mat2, dst, width, height, length);
	HANDLE_ERROR(cudaGetLastError());
	return 0;
}


// \brief: ʵ������ͼ�������غϲ��ֵ��ںϣ�����������ͼ����ȫ�ںϡ������ںϵ�ͼ����8UC3��ͼ������
// \para: mat1, mat2������Ҫ�غϵ�ͼ��ע����GPU�ϵ�����,���ݸ�ʽע�⣻ width��height�ֱ���ͼ��Ŀ�Ⱥ͸߶ȣ�
// �����blocksizeĬ��������32��������Ը���ʵ��blockSize�������ã�
int cudaMeltHor(cv::cuda::GpuMat mat1, cv::cuda::GpuMat mat2, cv::cuda::GpuMat dst, int width, int height, int channels)
{
	int length = width*channels;
    dim3 blockrgb(32, 32);
	dim3 gridrgb((length + blockrgb.x - 1) / blockrgb.x, (height + blockrgb.y - 1) / blockrgb.y);
    MeltHor<< <gridrgb, blockrgb >> > (mat1, mat2, dst, width, height, length);
	HANDLE_ERROR(cudaGetLastError());
	return 0;
}

int cudaMeltHorUC3(cv::cuda::GpuMat mat1, cv::cuda::GpuMat mat2, cv::cuda::GpuMat dst, int width, int height, int channels)
{
	int length = width*height;
    dim3 blockrgb(32, 32);
	dim3 gridrgb((width+blockrgb.x-1)/blockrgb.x , (height + blockrgb.y-1)/blockrgb.y);
	MeltHorU3<<<gridrgb, blockrgb>>>(mat1, mat2, dst, width, height, length, mat1.step);
	HANDLE_ERROR(cudaGetLastError());
	return 0;
}


int maincu()
{
	u_char A[30][10];
	u_char B[30][10];
	u_char C[30][10] = {0};
	for(int i=0;i<10;i++)
	{
		for(int j=0;j<30;j++)
		{
			A[i][j] = 100;
			B[i][j] = 0;
		}
	}
	u_char* dev_A;
	u_char* dev_B;
	u_char* dev_C;
	int width = 10; int height = 10; int channels = 3;
	float* dev_alphaTableVer = NULL;
	float* dev_alphaTableHor = NULL;
	int mallocSize = sizeof(u_char) * width * height*3;
	HANDLE_ERROR(cudaMalloc((void**)&dev_A, mallocSize));
	HANDLE_ERROR(cudaMalloc((void**)&dev_B, mallocSize));
	HANDLE_ERROR(cudaMalloc((void**)&dev_C, mallocSize));
	int sizeOfTableHor = sizeof(float) * height;
	// HANDLE_ERROR(cudaMemcpy(dev_alphaTableHor, alphaTableHor, sizeOfTableHor, cudaMemcpyHostToDevice));
	HANDLE_ERROR(cudaMemcpy(dev_A, A, mallocSize, cudaMemcpyHostToDevice));
	HANDLE_ERROR(cudaMemcpy(dev_B, B, mallocSize, cudaMemcpyHostToDevice));
	// cudaMeltHor(dev_A, dev_B, dev_C, width, height, channels);
	//dim3 threadsPerBlock(32, 32);
	//int gridCols = (width * channels + 32 - 1) / threadsPerBlock.x;
	//int gridRows = (height + 32 - 1) / threadsPerBlock.y;
	//dim3 blocksPerGrid(gridCols, gridRows);
	//int size = width * height * channels;
	//matrixMelt << <blocksPerGrid, threadsPerBlock >> > (dev_A, dev_B, dev_C, width, width, size);
	//HANDLE_ERROR(cudaGetLastError());
	// cudaMemcpy(C, dev_C, mallocSize, cudaMemcpyDeviceToHost);
	for (size_t i = 0; i < height; i++)
	{
		for (size_t j = 0; j < width*channels; j++)
		{
			printf("%d ", C[i][j]);
		}
		printf("\n");
	}
	cudaFree(dev_A);
	cudaFree(dev_B);
	cudaFree(dev_C);
	return 0;
}