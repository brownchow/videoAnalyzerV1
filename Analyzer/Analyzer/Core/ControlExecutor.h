#ifndef ANALYZER_CONTROLEXECUTOR_H
#define ANALYZER_CONTROLEXECUTOR_H

#include <thread>
#include <queue>
#include <mutex>

namespace AVSAnalyzer {
    class Scheduler;
    class AvPullStream;
    class AvPushStream;
    class GenerateAlarm;
    class Analyzer;
    struct Control;

    /**
     * 视频帧结构体
     */
    struct VideoFrame
    {
    public:
        /**
         * 视频帧类型枚举
         */
        enum VideoFrameType
        {
            BGR = 0,    // BGR格式
            YUV420P,    // YUV420P格式
        };

        /**
         * 构造函数
         * @param type 视频帧类型
         * @param size 数据大小
         * @param width 宽度
         * @param height 高度
         */
        VideoFrame(VideoFrameType type, int size, int width, int height) {
            this->type = type;
            this->size = size;
            this->width = width;
            this->height = height;
            this->data = new uint8_t[this->size];
        }

        /**
         * 析构函数
         */
        ~VideoFrame() {
            delete[] this->data;
            this->data = nullptr;
        }

        VideoFrameType type;  // 视频帧类型
        int size;             // 数据大小
        int width;            // 宽度
        int height;           // 高度
        uint8_t* data;        // 数据指针
        bool happen = false;  // 是否发生事件
        float happenScore = 0; // 发生事件的分数
    };

    /**
     * 音频帧结构体
     */
    struct AudioFrame
    {
    public:
        /**
         * 构造函数
         * @param size 数据大小
         */
        AudioFrame(int size) {
            this->size = size;
            this->data = new uint8_t[this->size];
        }

        /**
         * 析构函数
         */
        ~AudioFrame() {
            delete[] this->data;
            this->data = NULL;
        }

        int size;        // 数据大小
        uint8_t* data;   // 数据指针
    };

    /**
     * 控制执行器类
     * 负责管理视频流的拉取、解码、分析和推流
     */
    class ControlExecutor
    {
    public:
        /**
         * 构造函数
         * @param scheduler 调度器指针
         * @param control 控制对象指针
         */
        explicit ControlExecutor(Scheduler* scheduler, Control* control);

        /**
         * 析构函数
         */
        ~ControlExecutor();

    public:
        /**
         * 解码视频帧和实时分析视频帧
         * @param arg 线程参数
         */
        static void decodeAndAnalyzeVideoThread(void* arg);

        /**
         * 解码音频帧和实时分析音频帧
         * @param arg 线程参数
         */
        static void decodeAndAnalyzeAudioThread(void* arg);

    public:
        /**
         * 启动执行器
         * @param msg 错误信息
         * @return 是否启动成功
         */
        bool start(std::string& msg);

        /**
         * 获取执行器状态
         * @return 执行器状态
         */
        bool getState();

        /**
         * 设置执行器状态为停止并从调度器中移除
         */
        void setState_remove();

    public:
        /**
         * 控制对象
         */
        Control* mControl;    
        
        /**
         * 调度器
         */
        Scheduler* mScheduler;

        /**
         * 拉流对象
         */
        AvPullStream* mPullStream;

        /**
         * 推流对象
         */
        AvPushStream* mPushStream;

        /**
         * 报警生成器
         */
        GenerateAlarm* mGenerateAlarm;

        /**
         * 分析器
         */
        Analyzer* mAnalyzer;        

    private:
        /**
         * 执行器状态
         */
        bool mState = false;

        /**
         * 线程列表
         */
        std::vector<std::thread*> mThreads;
    };
}

#endif //ANALYZER_CONTROLEXECUTOR_H