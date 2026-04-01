#ifndef ANALYZER_GENERATEALARM_H
#define ANALYZER_GENERATEALARM_H

#include <queue>
#include <mutex>

namespace AVSAnalyzer {
    class Config;
    struct Control;
    struct VideoFrame;
    struct AudioFrame;

    /**
     * 报警生成器类
     * 负责处理视频帧并生成报警
     */
    class GenerateAlarm
    {
    public:
        /**
         * 构造函数
         * @param config 配置对象指针
         * @param control 控制对象指针
         */
        GenerateAlarm(Config* config, Control* control);
        
        /**
         * 析构函数
         */
        ~GenerateAlarm();
        
    public:
        /**
         * 推送视频帧
         * @param data 视频数据
         * @param size 数据大小
         * @param happen 是否发生事件
         * @param happenScore 事件发生的分数
         */
        void pushVideoFrame(unsigned char* data, int size, bool happen, float happenScore);
        
        /**
         * 生成报警线程
         * @param arg 线程参数
         */
        static void generateAlarmThread(void* arg);
        
    private:
        Config* mConfig;   // 配置对象
        Control* mControl; // 控制对象

        // 视频帧队列
        std::queue <VideoFrame*> mVideoFrameQ;
        std::mutex               mVideoFrameQ_mtx;
        
        /**
         * 获取视频帧
         * @param frame 视频帧指针引用
         * @param frameQSize 帧队列大小
         * @return 是否获取成功
         */
        bool getVideoFrame(VideoFrame*& frame, int& frameQSize);
        
        /**
         * 清空视频帧队列
         */
        void clearVideoFrameQueue();

    };
}

#endif //ANALYZER_GENERATEALARM_H