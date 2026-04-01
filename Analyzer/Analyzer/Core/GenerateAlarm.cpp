#include "GenerateAlarm.h"
#include "Config.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "Control.h"
#include "ControlExecutor.h"
#include "Scheduler.h"
#include "../../AVSAlarmManage/include/AVSAlarmManage.h"
#include <cstring>

using namespace AVSAlarmManageLib;

namespace AVSAnalyzer {
    /**
     * 构造函数
     * @param config 配置对象指针
     * @param control 控制对象指针
     */
    GenerateAlarm::GenerateAlarm(Config* config, Control* control) :
        mConfig(config),
        mControl(control)
    {

    }

    /**
     * 析构函数
     */
    GenerateAlarm::~GenerateAlarm()
    {
        clearVideoFrameQueue();
    }

    /**
     * 推送视频帧
     * @param data 视频数据
     * @param size 数据大小
     * @param happen 是否发生事件
     * @param happenScore 事件发生的分数
     */
    void GenerateAlarm::pushVideoFrame(unsigned char* data, int size, bool happen, float happenScore) {
        // 创建新的视频帧
        VideoFrame* frame = new VideoFrame(VideoFrame::BGR, size, mControl->videoWidth, mControl->videoHeight);
        frame->size = size;
        memcpy(frame->data, data, size);
        frame->happen = happen;
        frame->happenScore = happenScore;

        // 将视频帧加入队列
        mVideoFrameQ_mtx.lock();
        mVideoFrameQ.push(frame);
        mVideoFrameQ_mtx.unlock();
    }

    /**
     * 获取视频帧
     * @param frame 视频帧指针引用
     * @param frameQSize 帧队列大小
     * @return 是否获取成功
     */
    bool GenerateAlarm::getVideoFrame(VideoFrame*& frame, int& frameQSize) {
        mVideoFrameQ_mtx.lock();

        if (!mVideoFrameQ.empty()) {
            frame = mVideoFrameQ.front();
            mVideoFrameQ.pop();
            frameQSize = mVideoFrameQ.size();
            mVideoFrameQ_mtx.unlock();
            return true;

        } else {
            mVideoFrameQ_mtx.unlock();
            return false;
        }

    }

    /**
     * 清空视频帧队列
     */
    void GenerateAlarm::clearVideoFrameQueue() {
        mVideoFrameQ_mtx.lock();
        while (!mVideoFrameQ.empty()) {
            VideoFrame* frame = mVideoFrameQ.front();
            mVideoFrameQ.pop();
            delete frame;
            frame = nullptr;
        }
        mVideoFrameQ_mtx.unlock();

    }

    /**
     * 生成报警线程，重点！
     * @param arg 线程参数
     */
    void GenerateAlarm::generateAlarmThread(void* arg) {
        ControlExecutor* executor = (ControlExecutor*)arg;
        int width = executor->mControl->videoWidth;
        int height = executor->mControl->videoHeight;
        int channels = 3;

        VideoFrame* videoFrame = nullptr; // 未编码的视频帧（bgr格式）
        int         videoFrameQSize = 0; // 未编码视频帧队列当前长度

        std::vector<AVSAlarmImage* > cacheV;
        int cacheV_max_size = 250; // 150 = 25 * 6，最多缓存事件发生前6秒的数据，1张压缩图片100kb
        int cacheV_min_size = 50;  // 50 = 25 * 2, 最少缓存事件发生前2秒的数据

        bool happening = false; // 当前是否正在发生报警行为
        std::vector<AVSAlarmImage* > happenV;
        int     happenV_alarm_max_size = 500;
        int64_t last_alarm_timestamp = 0; // 上一次报警的时间戳

        int64_t t1, t2 = 0;

        while (executor->getState()) {
            if (executor->mGenerateAlarm->getVideoFrame(videoFrame, videoFrameQSize)) {
                t1 = Analyzer_getCurTime();
                // 获取报警图片对象，从队列里拿一个图片
                AVSAlarmImage* image = executor->mScheduler->gainAlarmImage();
                // 压缩图片
                bool comp = AVSAlarmManage_CompressImage(height, width, channels, videoFrame->data, image);
                if (comp) {
                    image->happen = videoFrame->happen;
                    image->happenScore = videoFrame->happenScore;
                }

                t2 = Analyzer_getCurTime();

                if (happening) { // 报警事件已经发生，正在进行中
                    if (comp) {
                        happenV.push_back(image);
                        // LOGI("h=%d,w=%d,compressSize=%lu,compress spend: %lld(ms),happenV.size=%lld",
                        //    height, width, compressImage->size, (t2 - t1), happenV.size());
                    } else {
                        executor->mScheduler->giveBackAlarmImage(image);
                    }
                    if (happenV.size() >= happenV_alarm_max_size) {
                        last_alarm_timestamp = Analyzer_getCurTimestamp();
                        // 创建报警对象
                        AVSAlarm* alarm = AVSAlarm::Create(
                            height,
                            width,
                            executor->mControl->videoFps,
                            last_alarm_timestamp,
                            executor->mControl->code.data()
                        );
                        // 添加图片到报警对象
                        for (size_t i = 0; i < happenV.size(); i++) {
                            alarm->images.push_back(happenV[i]);
                        }
                        happenV.clear();

                        // 添加报警到调度器
                        executor->mScheduler->addAlarm(alarm);

                        happening = false;
                    }
                    delete videoFrame;
                    videoFrame = nullptr;

                } else { // 暂未发生报警事件
                    if (comp) {
                        cacheV.push_back(image);
                        // LOGI("cache h=%d,w=%d,compressSize=%d,compress spend: %lld(ms),cacheQ.size=%lld",height, width, compressImage.getSize(), (t2 - t1), cacheV.size());
                        if (!cacheV.empty() && cacheV.size() > cacheV_max_size) {
                            // 满足缓存过期帧
                            auto b = cacheV.begin();
                            cacheV.erase(b);
                            AVSAlarmImage* headImage = *b;
                            executor->mScheduler->giveBackAlarmImage(headImage);
                        }

                        if (videoFrame->happen && cacheV.size() > cacheV_min_size &&
                            (Analyzer_getCurTimestamp() - last_alarm_timestamp) > executor->mControl->alarmMinInterval) {
                            // 满足报警触发帧
                            happening = true;
                            happenV = cacheV;
                            cacheV.clear();
                        }
                    } else {
                        executor->mScheduler->giveBackAlarmImage(image);
                    }
                    delete videoFrame;
                    videoFrame = nullptr;
                }
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }

        }

    }
}
