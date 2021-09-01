#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>
#include "device_types.h"
#include <assert.h>
#include <device_launch_parameters.h>

typedef unsigned char u_char;

static void HandleError(cudaError_t err,
	const char* file,
	int line) {
	if (err != cudaSuccess) {
		printf("%s in %s at line %d\n", cudaGetErrorString(err),
			file, line);
		exit(EXIT_FAILURE);
	}
}
#define HANDLE_ERROR( err ) (HandleError( err, __FILE__, __LINE__ ))


__global__ void matrixMelt(unsigned char* A, unsigned char* B, unsigned char* C, int start,
	int wA, int wB, int size, const float* alphaTable)
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
	int aIdx = (idx /3) % 10;
	if (idx == 88)
	{
		for (size_t i = 0; i < 10; i++)
		{
			printf("%f ", alphaTable[i]);
		}
		printf("\n");
	}
	float alpha = aIdx*1.0 / wA;
	printf("alpha[%d] is %f, index in mat is %d\n", aIdx, alpha, idx);
	C[idx] = A[idx] * alpha + B[idx] * (1 - alpha);

}

void meltCall(int width, int height, int channels)
{
	u_char* dev_A = NULL;
	u_char* dev_B = NULL;
	u_char* dev_C = NULL;


	u_char A[10][30];
	u_char B[10][30];
	u_char C[10][30] = { 0 };
	for (int i = 0; i < 30; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			A[j][i] = 100;
			B[j][i] = 0;
		}
	}
	float* alphaTableVer = (float*)malloc(width * sizeof(float));
	float* alphaTableHor = (float*)malloc(height * sizeof(float));
	for (size_t i = 0; i < height; i++)
	{
		alphaTableVer[i] = 1 - i / (height * 1.0);
		printf("%f  ", alphaTableVer[i]);
	}
	printf("\n");
	for (size_t i = 0; i < width; i++)
	{
		alphaTableHor[i] = 1 - i / (width * 1.0);
		printf("%f  ", alphaTableHor[i]);
	}
	printf("\n");
	float* dev_alphaTableVer = NULL;
	float* dev_alphaTableHor = NULL;
	const int mallocSize = sizeof(u_char) * width * height * channels;
	HANDLE_ERROR(cudaMalloc((void**)&dev_A, mallocSize));
	HANDLE_ERROR(cudaMalloc((void**)&dev_B, mallocSize));
	HANDLE_ERROR(cudaMalloc((void**)&dev_C, mallocSize));
	HANDLE_ERROR(cudaMalloc((void**)&dev_alphaTableVer, height * sizeof(float)));
	HANDLE_ERROR(cudaMalloc((void**)&dev_alphaTableHor, height * sizeof(float)));
	int sizeOfTableHor = sizeof(float) * height;
	HANDLE_ERROR(cudaMemcpy(dev_alphaTableHor, alphaTableHor, sizeOfTableHor, cudaMemcpyHostToDevice));
	HANDLE_ERROR(cudaMemcpy(dev_A, A, mallocSize, cudaMemcpyHostToDevice));
	HANDLE_ERROR(cudaMemcpy(dev_B, B, mallocSize, cudaMemcpyHostToDevice));

	dim3 threadsPerBlock(32, 32);
	int gridCols = (30 + 32 - 1) / threadsPerBlock.x;
	int gridRows = (10 + 32 - 1) / threadsPerBlock.y;
	dim3 blocksPerGrid(gridCols, gridRows);

	matrixMelt <<<blocksPerGrid, threadsPerBlock >>> (dev_A, dev_B, dev_C, 0, 10, 10, mallocSize,
		dev_alphaTableHor);
	HANDLE_ERROR(cudaGetLastError());
	cudaMemcpy(C, dev_C, mallocSize, cudaMemcpyDeviceToHost);
	for (size_t i = 0; i < 10; i++)
	{
		for (size_t j = 0; j < 30; j++)
		{
			printf("%d ", C[i][j]);
		}
		printf("\n");
	}

	free(alphaTableHor);
	free(alphaTableVer);

}

int main()
{
	meltCall(10, 10, 3);
}
