/*!
 * \file hardDecode.cpp
 * \author ASUS
 * \date  2021
 * \comment 代码中实现ffmpeg rtsp 拉流并进行解码的操作。代码中实现了软解码与硬解码
 *          当宏定义为USE_HARD_DEVICE时为硬解码，为 SOFTDECODE时为软解码
 */
#include<../include/sourceDecode.h>
// #define USE_HARD_DEVICE
#define SOFTDECODE
enum AVPixelFormat hwDevicePixel, frameShowFormat;
enum AVPixelFormat hw_get_format(AVCodecContext *ctx,const enum AVPixelFormat *fmts)
{
    const enum AVPixelFormat *p;
    for (p = fmts; *p != AV_PIX_FMT_NONE; p++) {
        if (*p == hwDevicePixel) {
            return *p;
        }
    }

    return AV_PIX_FMT_NONE;
}
void hardDecode(const char* rtspString)
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
    AVFrame *pAVHostFrame = 0;
    AVFrame *pAVFrameRGB24 = 0;    // 单帧颜色转换后的缓存
    struct SwsContext *pSwsContext = 0;             // ffmpeg编码数据格式转换
    AVDictionary *pAVDictionary = 0;                // ffmpeg数据字典，用于配置一些编码器属性等

    enum AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
    enum AVHWDeviceType print_type = AV_HWDEVICE_TYPE_NONE;
    AVBufferRef* hwDeviceCtx = NULL;
    type = av_hwdevice_find_type_by_name("cuda");
    // 遍历设备支持的硬件类型
    while ((print_type = av_hwdevice_iterate_types(print_type))!=AV_HWDEVICE_TYPE_NONE)
    {
        LOGD<<"support devices "<<av_hwdevice_get_type_name(print_type);
    }
    // 判断type
    if(type == AV_HWDEVICE_TYPE_NONE)
    {
        LOG(plog::error)<<type<<" is not supported.";
        return;
    }

    int ret = 0;    // 函数执行结果
    int videoIndex = -1;   //音频所在的序号
    int numBytes = 0;  // 解码后的数据长度
    unsigned char *outBuffer = 0;    // 解码后的数据存放缓存区

    // 分配
    pAVFormatContext = avformat_alloc_context();
    pAVFrame = av_frame_alloc();
    pAVPacket = av_packet_alloc();
    pAVFrameRGB24 = av_frame_alloc();
    pAVHostFrame = av_frame_alloc();
    hwDeviceCtx = av_hwdevice_ctx_alloc(AV_HWDEVICE_TYPE_CUDA );
    if(!pAVFormatContext||!pAVFrame||!pAVPacket||!pAVFrameRGB24)
    {
        LOG(plog::error)<<"Failed to alloc\n";
        return;
    }

    // 第一步： 注册容器和编码器等
    av_register_all();
    avformat_network_init();

    // 第二部： 打开文件，成功返回0;TODO: 注意这里会有阻塞，所以可以在这里写一个判断，查询rtsp是否可用
    LOG(plog::info)<<"opnecv:"<<rtspString;
    ret = avformat_open_input(&pAVFormatContext, const_cast<char*>(rtspString)
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
#ifdef SOFTDECODE
    pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);
    if(!pAVCodec)
    {
        LOG(plog::error)<<"Failed to find decoder,avcodec_find_decoder(pAVCodecContext->codec_id):"<<pAVCodecContext->codec_id;
        return;
    }
#endif

#ifdef USE_HARD_DEVICE
    // 根据解码器获取支持此解码方式的硬件加速器。所有支持的将建解码器保存在AVCodec的hw_configs中
    // 对于硬件解码器来说又是单独的AVCodec
    if((ret = av_find_best_stream(pAVFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &pAVCodec, 0) )<0)
    {
        LOG(plog::info)<<"av_find_best_stream failed "<<ret<<std::endl;
        return;
    }
    for(int i=0;;i++)
    {
        // hwcodec : AVCodecHWConfig, 解码配置文件
        const AVCodecHWConfig *hwcodec = avcodec_get_hw_config(pAVCodec,i);
        if(hwcodec==NULL)
            break;
        // 可能一个解码器对应多个硬件加速方式，这里挑选出来
        if(hwcodec->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && hwcodec->device_type== type)
        {
            hwDevicePixel = hwcodec->pix_fmt;
        }
        

    }
	if ((pAVCodecContext = avcodec_alloc_context3(pAVCodec)) == NULL)
	{
		LOG(plog::error) << "avcodec_alloc_context3 failed";
		return;
	}
    pAVStream = pAVFormatContext->streams[videoIndex];
    // 给解码器复制相关参数
    if (avcodec_parameters_to_context(pAVCodecContext, pAVStream->codecpar)<0)
    {
        LOG(plog::error) << "avcodec_parameters_to_context failed";
    }
    // 配置获取硬件加速器像素格式的函数，
    pAVCodecContext->get_format = hw_get_format;
    // 创建硬件加速器的缓冲区
    if(av_hwdevice_ctx_create(&hwDeviceCtx, type, 0, 0, 0)<0)
    {
        LOG(plog::error)<<"av_hardevice_ctx_create failed,the type is"<<type;
        return;
    }
    // 如果hwdeviceCtx有值，使用硬件解码
    pAVCodecContext->hw_device_ctx = av_buffer_ref(hwDeviceCtx);

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

    LOG(plog::info) << "比特率:" << pAVCodecContext->bit_rate;
    PLOGD << "宽高:" << pAVCodecContext->width << "x" << pAVCodecContext->height;
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
    // Assertion desc failed at libswscale/swscale_internal.h:677该错误发生时，
    // 是获取原数据的高度有问题
    // 如果是硬解码，这里需要注意，解码器的上下文对应的这里是cuda中的format，而不是转出的format
    frameShowFormat = pAVCodecContext->pix_fmt;
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
    int fillResult = avpicture_fill((AVPicture *)pAVFrameRGB24,
                       outBuffer,
                       AV_PIX_FMT_BGR24,
                       pAVCodecContext->width,
                       pAVCodecContext->height);
    LOG(plog::info) << "avpicture_fill() result: " << fillResult;
    // 步骤八： 读取一帧数据的数据包
    clock_t startT, endT;
    int countText = 0;
    frameShow.create(pAVCodecContext->height, pAVCodecContext->width, CV_8UC3);
    if (!av_read_frame(pAVFormatContext, pAVPacket))
    {
        if (pAVPacket->stream_index == videoIndex)
        {
            ret = avcodec_send_packet(pAVCodecContext, pAVPacket);
            if (!avcodec_receive_frame(pAVCodecContext, pAVFrame))
            {
                LOG(plog::error) << "avcodec_receive_frame() failed";
            }
#ifdef USE_HARD_DEVICE



            if (pAVFrame->format == hwDevicePixel)
            {
                // 如果采用硬件加速，则调用avcodec_receive_frame()函数后，解码后的数据在GPU，
                // 需要通过此函数将数据转移到CPU中来
                
                if ((ret = av_hwframe_transfer_data(pAVHostFrame, pAVFrame, 0)) < 0)
                {
                    LOGD << "av_hwframe_transfer_data failed";
                    return;
                }

                pSwsContext = sws_getContext(pAVHostFrame->width,
                    pAVHostFrame->height,
                    (AVPixelFormat)pAVHostFrame->format,
                    pAVCodecContext->width,
                    pAVCodecContext->height,
                    AV_PIX_FMT_BGR24,
                    SWS_FAST_BILINEAR,
                    0,
                    0,
                    0);
            }
#endif // USE_HARD_DEVICE
        }
    }
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
            if(ret)
            {
                LOG(plog::error)<<"Failed to avcodec_send_packet(()";
                break;
            }
            while(!avcodec_receive_frame(pAVCodecContext, pAVFrame))  // 使用send_packet时，需要使用receive函数获取解码后的帧；
            {
#ifdef USE_HARD_DEVICE
                if(pAVFrame->format == hwDevicePixel)
                {
                    // 如果采用硬件加速，则调用avcodec_receive_frame()函数后，解码后的数据在GPU，
                    // 需要通过此函数将数据转移到CPU中来
                    if((ret = av_hwframe_transfer_data(pAVHostFrame, pAVFrame, 0))<0)
                    {
                        LOGD<<"av_hwframe_transfer_data failed";
                        return;
                    }
                    else
                    {
                        // pAVFrame = pAVHostFrame;
                    }

                }
#else
                pAVHostFrame = pAVFrame;
#endif
                int scaleFlag = sws_scale(pSwsContext,
                          (const uint8_t* const*)pAVHostFrame->data,
                          pAVHostFrame->linesize,
                          0,
                          pAVCodecContext->height,
                          pAVFrameRGB24->data,
                          pAVFrameRGB24->linesize);
                std::cout << "sws_scale return: " << scaleFlag << std::endl;
                //cv::Mat frameGet = avframeToCvmat(pAVHostFrame);
                // 格式为RGBA=8:8:8:8”
                // rmask 应为 0xFF000000  但是颜色不对 改为 0x000000FF 对了
                // gmask     0x00FF0000                  0x0000FF00
                // bmask     0x0000FF00                  0x00FF0000
                // amask     0x000000FF                  0xFF000000
                // 测试了ARGB，也是相反的，而QImage是可以正确加载的
                // 暂时只能说这个地方标记下，可能有什么设置不对什么的
                //-- 显示 --//
                
                memcpy(frameShow.data, outBuffer, numBytes);
                std::cout << frameShow.step1() << std::endl;
                std::cout << frameShow.step1(1) << std::endl;
                cv::imshow("decode", frameShow);
                cv::waitKey(1);
                // -- 推流-- //

            }

            endT = clock();
            std::cout<<"解码时间："<<endT - startT<<std::endl;
            // 下一帧
            currentFrame ++;
        }
        av_packet_unref(pAVPacket);
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
    if(pAVFrameRGB24)
    {
        av_frame_free(&pAVFrameRGB24);
        pAVFrameRGB24 = 0;
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
#ifdef USE_HARD_DEVICE
    if(hwDeviceCtx)
    {
        av_buffer_unref(&hwDeviceCtx);
    }
#endif
    if(pAVHostFrame)
    {
        av_frame_free(&pAVHostFrame);
        pAVHostFrame = 0;
        LOG(plog::info)<<"avframe_free(pAVHostFrame)";
    }

}
cv::Mat avframeToCvmat(const AVFrame* frame)
{
	int width = frame->width;
	int height = frame->height;
	cv::Mat image(height, width, CV_8UC3);
	int cvLinesizes[1];
	cvLinesizes[0] = image.step1();
	SwsContext* conversion = sws_getContext(width, height, (AVPixelFormat)frame->format, width, height, AVPixelFormat::AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	sws_scale(conversion, frame->data, frame->linesize, 0, height, &image.data, cvLinesizes);
	sws_freeContext(conversion);
	return image;
}
