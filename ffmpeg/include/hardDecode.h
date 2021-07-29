#ifndef HARDDECODE_H
#define HARDDECODE_H
#include <string>
#include <stdio.h>
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/hwcontext.h>
#include <libavutil/pixfmt.h>
#include <libavutil/error.h>
#include <libavutil/avutil.h>
#ifdef __cplusplus
}
#endif
using namespace std;

class HardEnDecoder
{
public:
    HardEnDecoder();
    ~HardEnDecoder();

    void doEncoder();
    void doDecoder();
};


#endif // HARDDECODE_H
