#include "../include/sourceDecode.h"
/*
 *代码实现读取本地视频文件，解码后保存到本地
*/
static FILE *pInput_File  = NULL;
static FILE *pOutput_File = NULL;

static char *Input_FileName  = NULL;
static char *Output_FileName = NULL;
#define USE_HARD_DEVICE

//
static int decode_write_frame(const char *outfilename, AVCodecContext *avctx,
                              AVFrame *frame, int *frame_count, AVPacket *pkt, int last)
{
    int i;
    int idx;
    int color_idx;
    int len, got_frame;
    char buf[1024];

    len = avcodec_decode_video2(avctx, frame, &got_frame, pkt);
    if(len < 0){
        fprintf(stderr, "Error while decoding frame %d\n", *frame_count);
        return len;
    }

    printf("len %d got_frame %d\n",len, got_frame);

    if(got_frame){
        printf("Saving %s frame %3d\n", last?"last":"", *frame_count);
        fflush(stdout);

        //the picture is allocated by the decoder, no need to free it
        (*frame_count)++;

        fwrite(frame->data[0], 1, frame->width*frame->height, pOutput_File);
        fwrite(frame->data[1], 1, (frame->width/2)*(frame->height/2), pOutput_File);
        fwrite(frame->data[2], 1, (frame->width/2)*(frame->height/2), pOutput_File);

        if(pkt->data){
            pkt->size -= len;
            pkt->data += len;
        }
    }

    return 0;
}

int decodeVideo()
{
    int ret = 0;
    int len = 0;
    int frame_count = 0;
    AVCodec *codec = NULL;
    AVCodecContext *codecCtx = NULL;
    AVCodecParserContext *pCodecParserCtx = NULL;
    AVFrame *frame;
    AVPacket pkt;
    uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t *pDataPtr;
    size_t  uDataSize;
    // Input_FileName = const_cast<char*>("E:/Code/myFFmpeg/data/input/T-3-01.mp4");
    Input_FileName = const_cast<char*>("F:/Datasets/Init ship Video/20210428Video/192.168.10.100_01_20210428120927665.mp4");

    Output_FileName = const_cast<char*>("E:/Code/myFFmpeg/data/output/T-3-01.mp4");

    //Input_FileName  = argv[1];
    // Output_FileName = argv[2];

    pInput_File = fopen(Input_FileName, "rb+");
    if(!pInput_File){
        fprintf(stderr, "Open input file fail\n");
        exit(1);
    }

    pOutput_File = fopen(Output_FileName, "wb+");
    if(!pOutput_File){
        fprintf(stderr, "Open output file fail\n");
        exit(1);
    }

    //set end of buffer to 0
    memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

    printf("Decode video file %s to %s\n", Input_FileName, Output_FileName);

    av_register_all();

    av_init_packet(&pkt);

    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if(!codec){
        fprintf(stderr, "cannot find the decoder\n");
        exit(1);
    }

    codecCtx = avcodec_alloc_context3(codec);
    if(!codecCtx){
        fprintf(stderr, "could not allocate video codec context\n");
        exit(1);
    }

    if(codec->capabilities & AV_CODEC_CAP_TRUNCATED){
        codecCtx->flags |= AV_CODEC_FLAG_TRUNCATED;
    }

    pCodecParserCtx = av_parser_init(AV_CODEC_ID_H264);
    if(!pCodecParserCtx){
        fprintf(stderr,"Error:alloc parser fail\n");
        exit(1);
    }

    //open the decoder
    if(avcodec_open2(codecCtx, codec, NULL) < 0){
        fprintf(stderr, "Could not open the decoder\n");
        exit(1);
    }

    //open frame structure
    frame = av_frame_alloc();
    if(!frame){
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    frame_count = 0;
    for(;;){
        uDataSize = fread(inbuf, 1, INBUF_SIZE, pInput_File);
        if(uDataSize == 0)
            break;

        pDataPtr = inbuf;
        while(uDataSize > 0){
            //decode the data in the buffer to AVPacket, include a NAL unit data
            len = av_parser_parse2(pCodecParserCtx, codecCtx, &(pkt.data), &(pkt.size),
                                   pDataPtr, uDataSize,
                                   AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
            uDataSize -= len;
            pDataPtr  += len;

            if(pkt.size == 0){
                continue;
            }
            printf("Decode frame pts %d pkt.size %d\n", (int)pkt.pts, (int)pkt.size);

            if(decode_write_frame(Output_FileName, codecCtx, frame, &frame_count, &pkt, 0) < 0){
                exit(1);
            }

        }
    }

    //decode the data in the decoder itself
    pkt.size = 0;
    pkt.data = NULL;
    decode_write_frame(Output_FileName, codecCtx, frame, &frame_count, &pkt, 0);

    fclose(pInput_File);
    fclose(pOutput_File);
    av_frame_free(&frame);

    return 0;
}

// 硬件解码
#ifdef USE_HARD_DEVICE
enum AVPixelFormat hw_device_pixel;

#endif

void decodeCamera(const char* cameraStress)
{
    // 准备工作，初始化所用参数
    int64_t startTime = 0;    // 记录播放开始
    int currentFrame = 0;    // 当前帧序号
    double fps = 0;           // 帧率
    double interval = 0;      // 帧间隔
    int gotpicture=0;

    cv::Mat frameShow;

    // # ffmpeg相关变量预先定义与分配#
    // ffmpeg的全局上下文，所有ffmpeg操作都需要
    AVFormatContext* pAVFormatContext = 0;
    // ffmpeg流信息
    AVStream *pAVStream = 0;
    // ffmpeg 编码上下文
    AVCodecContext *pAVCodecContext = 0;

    AVCodec *pAVCodec = 0; // ffmpeg 编码器
    AVPacket *pAVPacket = 0;  //ffmpeg 单帧数据包
    AVFrame *pAVFrame = 0;   // ffmpeg 单帧缓存
    AVFrame *pAVFrameRGB32 = 0;    // 单帧颜色转换后的缓存
    AVFrame* tmpAVFrame = 0;
    struct SwsContext *pSwsContext = 0;             // ffmpeg编码数据格式转换
    AVDictionary *pAVDictionary = 0;                // ffmpeg数据字典，用于配置一些编码器属性等


    int ret = 0;    // 函数执行结果
    int videoIndex = -1;   //音频所在的序号
    int numBytes = 0;  // 解码后的数据长度
    unsigned char *outBuffer = 0;    // 解码后的数据存放缓存区

    // 分配
    pAVFormatContext = avformat_alloc_context();
    pAVFrame = av_frame_alloc();
    pAVPacket = av_packet_alloc();
    pAVFrameRGB32 = av_frame_alloc();

    if(!pAVFormatContext||!pAVFrame||!pAVPacket||!pAVFrameRGB32)
    {
        LOG(plog::error)<<"Failed to alloc\n";
        return;
    }

    // 第一步： 注册容器和编码器等
    av_register_all();
    avformat_network_init();

    // 第二部： 打开文件，成功返回0;TODO: 注意这里会有阻塞，所以可以在这里写一个判断，查询rtsp是否可用
    LOG(plog::info)<<"opnecv:"<<cameraStress;
    ret = avformat_open_input(&pAVFormatContext, const_cast<char*>(cameraStress)
                              ,0,0);
    if(ret <0)
    {
        LOG(plog::info)<<"Failed to open input\n";
        return;
    }
    // 第三步： 探测流媒体信息
    ret = avformat_find_stream_info(pAVFormatContext, 0);
    if(ret<0)
    {
        LOG(plog::error)<<"Failed to get stream info";
        return;
    }

    // 第四步： 提取流信息，提取视频信息
    for(int index = 0; index < pAVFormatContext->nb_streams;index++)
    {
        pAVCodecContext = pAVFormatContext->streams[index]->codec;
        pAVStream = pAVFormatContext->streams[index];
        // 寻找视频解码器

        switch (pAVCodecContext->codec_type) {
        case AVMEDIA_TYPE_UNKNOWN:
                LOG(plog::info)<<"流序号"<<index<<"类型为："<<"AVMEDIA_TYPE_UNKNOWN";
            break;
        case AVMEDIA_TYPE_VIDEO:
            LOG(plog::info)<<"流序号"<<index<<"类型为："<<"AVMEDIA_TYPE_VIDEO";
            videoIndex = index;
            break;
        case AVMEDIA_TYPE_AUDIO:
            LOG(plog::info)<<"流序号"<<index<<"类型为："<<"AVMEDIA_TYPE_AUDIO";
            break;
        case AVMEDIA_TYPE_DATA:
            LOG(plog::info) << "流序号:" << index << "类型为:" << "AVMEDIA_TYPE_DATA";
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            LOG(plog::info) << "流序号:" << index << "类型为:" << "AVMEDIA_TYPE_SUBTITLE";
            break;
        case AVMEDIA_TYPE_ATTACHMENT:
            LOG(plog::info) << "流序号:" << index << "类型为:" << "AVMEDIA_TYPE_ATTACHMENT";
            break;
        case AVMEDIA_TYPE_NB:
            LOG(plog::info) << "流序号:" << index << "类型为:" << "AVMEDIA_TYPE_NB";
            break;
        default:
            break;
        }
        if(videoIndex != -1)
        {
            break;
        }
    }
    if(videoIndex==-1||!pAVCodecContext)
    {
        LOG(plog::error)<<"Failed to find video stream";
        return;
    }
    // 第五步， 对找到的视频流查找解码器
    pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);
    if(!pAVCodec)
    {
        LOG(plog::error)<<"Failed to find decoder,avcodec_find_decoder(pAVCodecContext->codec_id):"<<pAVCodecContext->codec_id;
        return;
    }

#ifdef USE_HARD_DEVICE
    const AVCodecHWConfig *hwcodec = NULL;

#endif

    // 步骤六： 打开解码器
    // 设置缓存大小： 1024000
    av_dict_set(&pAVDictionary, "buffer_size", "1024000", 0);
    // 设置超时时间 20s
    av_dict_set(&pAVDictionary, "stimeout", "20000000", 0);
    // 设置最大延迟 3s
    av_dict_set(&pAVDictionary, "max_delay", "3000000", 0);
    // 打开方式 tcp/udp
    av_dict_set(&pAVDictionary, "rtsp_transport", "tcp", 0);
    ret = avcodec_open2(pAVCodecContext, pAVCodec, &pAVDictionary);
    if(ret)
    {
        LOG(plog::error)<<"Failed to avcodec_open2()";
        return;
    }
    // 显示视频相关的参数信息（编码上下文）

    PLOGD << "比特率:" << pAVCodecContext->bit_rate;
    LOG(plog::info) << "宽高:" << pAVCodecContext->width << "x" << pAVCodecContext->height;
    PLOGD << "格式:" << pAVCodecContext->pix_fmt;  // AV_PIX_FMT_YUV420P 0
    PLOGD << "帧率分母:" << pAVCodecContext->time_base.den;
    PLOGD << "帧率分子:" << pAVCodecContext->time_base.num;
    PLOGD << "帧率分母:" << pAVStream->avg_frame_rate.den;
    PLOGD << "帧率分子:" << pAVStream->avg_frame_rate.num;
    PLOGD << "总时长:" << pAVStream->duration / 10000.0 << "s";
    PLOGD << "总帧数:" << pAVStream->nb_frames;
    PLOGD << "编码格式：" << pAVCodec->long_name;    // 这里获取编码格式
    // 有总时长的时候计算帧率
    fps = pAVStream->nb_frames / (pAVStream->duration / 10000.0);
    interval = pAVStream->duration / 10.0 / pAVStream->nb_frames;
    // 没有总时长的时候，使用分子分母计算
    fps = pAVStream->avg_frame_rate.num*1.0f/pAVStream->avg_frame_rate.den;
    interval = 1*1000/fps;
    PLOGD<<"平均帧率："<<fps;
    PLOGD<<"帧间隔"<<interval<<"ms";
    // 步骤七： 对拿到的原始数据格式进行缩放转换为指定的格式高宽大小
    pSwsContext = sws_getContext(pAVCodecContext->width,
                                 pAVCodecContext->height,
                                 pAVCodecContext->pix_fmt,
                                 pAVCodecContext->width,
                                 pAVCodecContext->height,
                                 AV_PIX_FMT_BGR24,
                                 SWS_FAST_BILINEAR,
                                 0,
                                 0,
                                 0);
    numBytes = avpicture_get_size(AV_PIX_FMT_BGR24,
                                  pAVCodecContext->width,
                                  pAVCodecContext->height);
    outBuffer = (unsigned char*)av_malloc(numBytes);
    // pAVFrame32的data指针指向了outBUffer
    int fillFlag = avpicture_fill((AVPicture *)pAVFrameRGB32,
                       outBuffer,
                       AV_PIX_FMT_BGR24,
                       pAVCodecContext->width,
                       pAVCodecContext->height);
    LOG(plog::info) << "avpicture_fill return " << fillFlag;
    // 步骤八： 读取一帧数据的数据包
    clock_t startT, endT;
    int countText = 0;
    while(av_read_frame(pAVFormatContext, pAVPacket)>=0)
    {
        countText++;
        std::cout<<"CountText: "<<countText<<std::endl;
        startT = clock();
        if(pAVPacket->stream_index == videoIndex)
        {
            // 对读取的数据包进行解码
            ret = avcodec_send_packet(pAVCodecContext, pAVPacket); // 将数据包发送给解码器，解码
            // 也可以使用decode_video2函数进行解码，能直接获取解码结果
            // ret = avcodec_decode_video2(pAVCodecContext, pAVFrame, &gotpicture,pAVPacket);
            if(ret)
            {
                LOG(plog::error)<<"Failed to avcodec_send_packet(()";
                break;
            }
            while(!avcodec_receive_frame(pAVCodecContext, pAVFrame))  // 使用send_packet时，需要使用receive函数获取解码后的帧；
            {
                int scaleFlat = sws_scale(pSwsContext,
                          (const uint8_t *const *)pAVFrame->data,
                          pAVFrame->linesize,
                          0,
                          pAVCodecContext->height,
                          pAVFrameRGB32->data,
                          pAVFrameRGB32->linesize);
                std::cout << "scaleFlat:  " << scaleFlat << std::endl;
                // 格式为RGBA=8:8:8:8”
                // rmask 应为 0xFF000000  但是颜色不对 改为 0x000000FF 对了
                // gmask     0x00FF0000                  0x0000FF00
                // bmask     0x0000FF00                  0x00FF0000
                // amask     0x000000FF                  0xFF000000
                // 测试了ARGB，也是相反的，而QImage是可以正确加载的
                // 暂时只能说这个地方标记下，可能有什么设置不对什么的
                //-- 显示 --//
                frameShow.create(pAVCodecContext->height, pAVCodecContext->width, CV_8UC3);
                memcpy(frameShow.data, outBuffer, numBytes);
                //cv::imshow("decode",  frameShow);
                //cv::waitKey(1);
                // -- 推流-- //


            }

            endT = clock();
            std::cout<<"解码时间："<<endT - startT<<std::endl;
            // 下一帧
            currentFrame ++;
        }
    }
    LOG(plog::info)<<"释放回收资源";
    if(outBuffer)
    {
        av_free(outBuffer);
        outBuffer = 0;
    }
    if(pSwsContext)
    {
        sws_freeContext(pSwsContext);
        pSwsContext = 0;
    }
    if(pAVFrame)
    {
        av_frame_free(&pAVFrame);
        pAVFrame = 0;
        LOG(plog::info)<<"av_frame_free()";
    }
    if(pAVFrameRGB32)
    {
        av_frame_free(&pAVFrameRGB32);
        pAVFrameRGB32 = 0;
        LOG(plog::info)<<"av_frame_free(pAVFrame32)";
    }
    if(pAVPacket)
    {
        av_packet_free(&pAVPacket);
        pAVPacket = 0;
        LOG(plog::info)<<"av_packet_free()";
    }
    if(pAVCodecContext)
    {
        avcodec_close(pAVCodecContext);
        pAVCodecContext = 0;
        LOG(plog::info)<<"avcodec_close(avcodecContext)";
    }
    if(pAVFormatContext)
    {
        avformat_close_input(&pAVFormatContext);
        avformat_free_context(pAVFormatContext);
        pAVFormatContext = 0;
        LOG(plog::info)<<"avformat_free_context(pavformatcontext)";
    }

}

// 实现硬解码



