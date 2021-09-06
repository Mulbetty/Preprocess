#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>
#include "device_types.h"
#include <assert.h>
#include "helper_cuda.h"
#include "helper_functions.h"

#ifdef __cplusplus
extern "C"
{
#endif
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
#define DLL_EXPORT_API __declspec(dllexport)

DLL_EXPORT_API 
int matMeltHor(u_char * mat1, u_char * mat2, u_char * dst, int width, int height, int channels);
DLL_EXPORT_API
int matMeltVer(u_char * mat1, u_char * mat2, u_char * dst, int width, int height, int channels);

// \brief: cuda上在竖直方向上实现图像渐变融合。从上到下融合
DLL_EXPORT_API
int cudaMeltVer(u_char* mat1, u_char* mat2, u_char* dst, int width, int height, int channels);

DLL_EXPORT_API
int cudaMeltHor(u_char* mat1, u_char* mat2, u_char* dst, int width, int height, int channels);

#ifdef __cplusplus
}
#endif
