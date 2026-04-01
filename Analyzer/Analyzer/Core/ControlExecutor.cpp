#include "ControlExecutor.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Scheduler.h"
#include "Analyzer.h"
#include "Control.h"
#include "AvPullStream.h"
#include "AvPushStream.h"
#include "GenerateAlarm.h"

extern "C" {
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}

namespace AVSAnalyzer {
    /**
     * 构造函数
     * @param scheduler 调度器指针
     * @param control 控制对象指针
     */
    ControlExecutor::ControlExecutor(Scheduler* scheduler, Control* control) :
        mScheduler(scheduler),
        mControl(new Control(*control)),
        mPullStream(nullptr),
        mPushStream(nullptr),
        mGenerateAlarm(nullptr),
        mAnalyzer(nullptr),
        mState(false)
    {
        // 设置执行器启动时间戳
        mControl->executorStartTimestamp = Analyzer_getCurTimestamp();

        LOGI("");
    }

    /**
     * 析构函数
     */
    ControlExecutor::~ControlExecutor()
    {
        LOGI("");

        // 短暂休眠，确保线程有时间结束
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // 将执行状态设置为false
        mState = false;

        // 等待所有线程结束
        for (auto th : mThreads) {
            th->join();
        }
        
        // 释放线程资源
        for (auto th : mThreads) {
            delete th;
            th = nullptr;
        }
        mThreads.clear();

        // 释放拉流对象
        if (mPullStream) {
            delete mPullStream;
            mPullStream = nullptr;
        }
        
        // 释放推流对象
        if (mPushStream) {
            delete mPushStream;
            mPushStream = nullptr;
        }

        // 释放分析器对象
        if (mAnalyzer) {
            delete mAnalyzer;
            mAnalyzer = nullptr;
        }

        // 释放报警生成对象
        if (mGenerateAlarm) {
            delete mGenerateAlarm;
            mGenerateAlarm = nullptr;
        }

        // 释放控制对象
        if (mControl) {
            delete mControl;
            mControl = nullptr;
        }
    }

    /**
     * 启动执行器
     * @param msg 错误信息
     * @return 是否启动成功
     */
    bool ControlExecutor::start(std::string& msg) {
        // 创建拉流对象
        this->mPullStream = new AvPullStream(mScheduler->getConfig(), mControl);
        
        // 连接拉流
        if (this->mPullStream->connect()) {
            // 如果需要推流
            if (mControl->pushStream) {
                // 创建推流对象
                this->mPushStream = new AvPushStream(mScheduler->getConfig(), mControl);
                
                // 连接推流
                if (this->mPushStream->connect()) {
                    // 推流连接成功
                }
                else {
                    msg = "pull stream connect success, push stream connect error";
                    return false;
                }
            }
            else {
                // 不需要推流
            }
        }
        else {
            msg = "pull stream connect error";
            return false;
        }

        // 创建分析器和报警生成器
        this->mAnalyzer = new Analyzer(mScheduler, mControl);
        this->mGenerateAlarm = new GenerateAlarm(mScheduler->getConfig(), mControl);

        // 将执行状态设置为true
        mState = true;

        // 创建并启动拉流读取线程
        std::thread* th = new std::thread(AvPullStream::readThread, this);
        mThreads.push_back(th);

        // 创建并启动视频解码和分析线程
        th = new std::thread(ControlExecutor::decodeAndAnalyzeVideoThread, this);
        mThreads.push_back(th);

        // 如果有音频，创建并启动音频解码和分析线程
        if (mControl->audioIndex > -1) {
            th = new std::thread(ControlExecutor::decodeAndAnalyzeAudioThread, this);
            mThreads.push_back(th);
        }

        // 创建并启动报警生成线程
        th = new std::thread(GenerateAlarm::generateAlarmThread, this);
        mThreads.push_back(th);

        // 如果需要推流
        if (mControl->pushStream) {
            // 如果有视频，创建并启动视频编码和推流线程
            if (mControl->videoIndex > -1) {
                th = new std::thread(AvPushStream::encodeVideoAndWriteStreamThread, this);
                mThreads.push_back(th);
            }

            // 如果有音频，创建并启动音频编码和推流线程
            if (mControl->audioIndex > -1) {
                th = new std::thread(AvPushStream::encodeAudioAndWriteStreamThread, this);
                mThreads.push_back(th);
            }
        }

        // 获取线程句柄
        for (auto th : mThreads) {
            th->native_handle();
        }

        return true;
    }

    /**
     * 获取执行器状态
     * @return 执行器状态
     */
    bool ControlExecutor::getState() {
        return mState;
    }

    /**
     * 设置执行器状态为停止并从调度器中移除
     */
    void ControlExecutor::setState_remove() {
        this->mState = false;
        this->mScheduler->removeExecutor(mControl);
    }

    /**
     * 视频解码和分析线程
     * @param arg 线程参数
     */
    void ControlExecutor::decodeAndAnalyzeVideoThread(void* arg) {
        ControlExecutor* executor = (ControlExecutor*)arg;
        int width = executor->mPullStream->mVideoCodecCtx->width;
        int height = executor->mPullStream->mVideoCodecCtx->height;

        AVPacket pkt; // 未解码的视频帧
        int pktQSize = 0; // 未解码视频帧队列当前长度

        // 分配解码后的数据帧
        AVFrame* frame_yuv420p = av_frame_alloc();// pkt->解码->frame
        AVFrame* frame_bgr = av_frame_alloc();

        // 计算BGR格式帧的缓冲区大小
        int frame_bgr_buff_size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, width, height, 1);
        uint8_t* frame_bgr_buff = (uint8_t*)av_malloc(frame_bgr_buff_size);
        av_image_fill_arrays(frame_bgr->data, frame_bgr->linesize, frame_bgr_buff, AV_PIX_FMT_BGR24, width, height, 1);

        // 创建颜色空间转换上下文（YUV420P转BGR）
        SwsContext* sws_ctx_yuv420p2bgr = sws_getContext(width, height,
            executor->mPullStream->mVideoCodecCtx->pix_fmt,
            executor->mPullStream->mVideoCodecCtx->width,
            executor->mPullStream->mVideoCodecCtx->height,
            AV_PIX_FMT_BGR24,
            SWS_BICUBIC, nullptr, nullptr, nullptr);

        int fps = executor->mControl->videoFps;

        // 算法检测参数
        bool cur_is_check = false; // 当前帧是否进行算法检测
        int continuity_check_count = 0; // 当前连续进行算法检测的帧数
        int continuity_check_max_time = 3000; // 连续进行算法检测，允许最长的时间（毫秒）
        int64_t continuity_check_start = Analyzer_getCurTime(); // 连续检测开始时间（毫秒）
        int64_t continuity_check_end = 0; // 连续检测结束时间

        int ret = -1;
        int64_t frameCount = 0;
        
        // 主循环
        while (executor->getState())
        {
            // 从拉流对象获取视频包
            if (executor->mPullStream->getVideoPkt(pkt, pktQSize)) {
                // 如果有视频流
                if (executor->mControl->videoIndex > -1) {
                    // 发送数据包到解码器
                    ret = avcodec_send_packet(executor->mPullStream->mVideoCodecCtx, &pkt);
                    if (ret == 0) {
                        // 接收解码后的帧
                        ret = avcodec_receive_frame(executor->mPullStream->mVideoCodecCtx, frame_yuv420p);

                        if (ret == 0) {
                            frameCount++;

                            // 将YUV420P格式转换为BGR格式
                            sws_scale(sws_ctx_yuv420p2bgr,
                                frame_yuv420p->data, frame_yuv420p->linesize, 0, height,
                                frame_bgr->data, frame_bgr->linesize);

                            // 判断是否需要进行算法检测
                            if (pktQSize == 0) {
                                cur_is_check = true;
                            }
                            else {
                                cur_is_check = false;
                            }

                            // 累计连续检测帧数
                            if (cur_is_check) {
                                continuity_check_count += 1;
                            }

                            // 计算检测帧率
                            continuity_check_end = Analyzer_getCurTime();
                            if (continuity_check_end - continuity_check_start > continuity_check_max_time) {
                                executor->mControl->checkFps = float(continuity_check_count) / (float(continuity_check_end - continuity_check_start) / 1000);
                                continuity_check_count = 0;
                                continuity_check_start = Analyzer_getCurTime();
                            }

                            // 进行视频帧分析
                            float happenScore;
                            bool happen = executor->mAnalyzer->checkVideoFrame(cur_is_check, frameCount, frame_bgr->data[0], happenScore);

                            // 如果需要推流，将帧推送到推流对象
                            if (executor->mControl->pushStream) {
                                executor->mPushStream->pushVideoFrame(frame_bgr->data[0], frame_bgr_buff_size);
                            }
                            
                            // 将帧推送到报警生成器
                            executor->mGenerateAlarm->pushVideoFrame(frame_bgr->data[0], frame_bgr_buff_size, happen, happenScore);
                        }
                        else {
                            LOGE("avcodec_receive_frame error : ret=%d", ret);
                        }
                    }
                    else {
                        LOGE("avcodec_send_packet error : ret=%d", ret);
                    }
                }

                // 释放数据包
                av_packet_unref(&pkt);
            }
            else {
                // 没有数据，短暂休眠
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        // 释放资源
        av_frame_free(&frame_yuv420p);
        frame_yuv420p = NULL;

        av_frame_free(&frame_bgr);
        frame_bgr = NULL;

        av_free(frame_bgr_buff);
        frame_bgr_buff = NULL;

        sws_freeContext(sws_ctx_yuv420p2bgr);
        sws_ctx_yuv420p2bgr = NULL;
    }

    /**
     * 音频解码和分析线程
     * @param arg 线程参数
     */
    void ControlExecutor::decodeAndAnalyzeAudioThread(void* arg) {
        ControlExecutor* executor = (ControlExecutor*)arg;

        AVPacket pkt; // 未解码的音频帧
        int pktQSize = 0; // 未解码音频帧队列当前长度
        AVFrame* frame = av_frame_alloc(); // pkt->解码->frame

        // 音频输入参数
        int in_channels = executor->mPullStream->mAudioCodecCtx->channels; // 输入声道数
        uint64_t in_channel_layout = av_get_default_channel_layout(in_channels); // 输入声道布局
        AVSampleFormat in_sample_fmt = executor->mPullStream->mAudioCodecCtx->sample_fmt; // 输入采样格式
        int in_sample_rate = executor->mPullStream->mAudioCodecCtx->sample_rate; // 输入采样率
        int in_nb_samples = executor->mPullStream->mAudioCodecCtx->frame_size; // 输入每帧采样数

        // 音频重采样输出参数
        uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO; // 输出声道布局（立体声）
        int out_channels = av_get_channel_layout_nb_channels(out_channel_layout); // 输出声道数
        AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16; // 输出采样格式（16位有符号整数）
        int out_sample_rate = 44100; // 输出采样率
        int out_nb_samples = 1024; // 输出每帧采样数

        // 创建音频重采样上下文
        struct SwrContext* swr_ctx_audioConvert = swr_alloc();
        swr_ctx_audioConvert = swr_alloc_set_opts(swr_ctx_audioConvert,
            out_channel_layout,
            out_sample_fmt,
            out_sample_rate,
            in_channel_layout,
            in_sample_fmt,
            in_sample_rate,
            0, NULL);
        swr_init(swr_ctx_audioConvert);

        // 计算输出缓冲区大小
        int out_buff_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
        uint8_t* out_buff = (uint8_t*)av_malloc(out_buff_size); // 重采样得到的PCM数据

        int ret = -1;
        int64_t frameCount = 0;
        int64_t t3, t4 = 0;
        
        // 主循环
        while (executor->getState())
        {
            // 从拉流对象获取音频包
            if (executor->mPullStream->getAudioPkt(pkt, pktQSize)) {
                // 如果有音频流
                if (executor->mControl->audioIndex > -1) {
                    t3 = Analyzer_getCurTime();
                    
                    // 发送数据包到解码器
                    ret = avcodec_send_packet(executor->mPullStream->mAudioCodecCtx, &pkt);
                    if (ret == 0) {
                        // 接收解码后的帧
                        while (avcodec_receive_frame(executor->mPullStream->mAudioCodecCtx, frame) == 0) {
                            t4 = Analyzer_getCurTime();
                            frameCount++;

                            // 重采样音频数据
                            swr_convert(swr_ctx_audioConvert, &out_buff, out_buff_size, (const uint8_t**)frame->data, frame->nb_samples);

                            // 如果需要推流，将音频数据推送到推流对象
                            if (executor->mControl->pushStream) {
                                executor->mPushStream->pushAudioFrame(out_buff, out_buff_size);
                            }
                        }
                    }
                    else {
                        LOGE("avcodec_send_packet : ret=%d", ret);
                    }
                }
                
                // 释放数据包
                av_packet_unref(&pkt);
            }
            else {
                // 没有数据，短暂休眠
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }

        // 释放资源
        av_frame_free(&frame);
        frame = NULL;

        av_free(out_buff);
        out_buff = NULL;

        swr_free(&swr_ctx_audioConvert);
        swr_ctx_audioConvert = NULL;
    }
}