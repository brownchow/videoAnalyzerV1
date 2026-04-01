#ifndef ANALYZER_SCHEDULER_H
#define ANALYZER_SCHEDULER_H

#include <map>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <thread>

#include "../../AVSAlarmManage/include/AVSAlarmManage.h"
using namespace AVSAlarmManageLib;

namespace AVSAnalyzer {
    class Config;
    class ControlExecutor;
    struct Control;

    /**
     * 调度器类
     * 负责管理控制执行器、处理报警等功能
     */
    class Scheduler
    {
    public:
        friend class ControlExecutor;

        /**
         * 构造函数
         * @param config 配置对象指针
         */
        Scheduler(Config* config);
        
        /**
         * 析构函数
         */
        ~Scheduler();
        
    public:
        /**
         * 获取配置对象
         * @return 配置对象指针
         */
        Config* getConfig();
        
        /**
         * 启动调度器主循环
         */
        void loop();

        /**
         * 设置调度器状态
         * @param state 状态值
         */
        void setState(bool state);
        
        /**
         * 获取调度器状态
         * @return 状态值
         */
        bool getState();

        /**
         * 添加报警
         * @param alarm 报警对象指针
         */
        void addAlarm(AVSAlarm* alarm);

        /**
         * 从队列池获取一个压缩图片的实例
         * @return 报警图片对象指针
         */
        AVSAlarmImage* gainAlarmImage();
        
        /**
         * 将一个压缩图片的实例归还到队列池
         * @param image 报警图片对象指针
         */
        void giveBackAlarmImage(AVSAlarmImage* image);
        
        // 报警图片实例计数
        int mAlarmImageInstanceCount = 0;

        // API 接口函数 start
        /**
         * 获取所有控制列表
         * @param controls 控制对象向量引用
         * @return 控制数量
         */
        int apiControls(std::vector<Control*>& controls);
        
        /**
         * 根据控制码获取控制对象
         * @param code 控制码
         * @return 控制对象指针
         */
        Control* apiControl(std::string& code);
        
        /**
         * 添加控制
         * @param control 控制对象指针
         * @param result_code 结果代码
         * @param result_msg 结果消息
         */
        void apiControlAdd(Control* control, int& result_code, std::string& result_msg);
        
        /**
         * 取消控制
         * @param control 控制对象指针
         * @param result_code 结果代码
         * @param result_msg 结果消息
         */
        void apiControlCancel(Control* control, int& result_code, std::string& result_msg);
        // API 接口函数 end

    private:
        // 配置对象指针
        Config* mConfig;

        // 调度器状态
        bool mState;

        // 控制执行器映射表 <控制码, 控制执行器指针>
        std::map<std::string, ControlExecutor*> mExecutorMap;
        std::mutex                              mExecutorMapMtx;
        
        /**
         * 获取执行器映射表大小
         * @return 执行器数量
         */
        int getExecutorMapSize();
        
        /**
         * 检查控制是否已添加
         * @param control 控制对象指针
         * @return 是否已添加
         */
        bool isAdd(Control* control);
        
        /**
         * 添加执行器
         * @param control 控制对象指针
         * @param controlExecutor 控制执行器指针
         * @return 是否添加成功
         */
        bool addExecutor(Control* control, ControlExecutor* controlExecutor);
        
        /**
         * 移除执行器（加入到待实际删除队列）
         * @param control 控制对象指针
         * @return 是否移除成功
         */
        bool removeExecutor(Control* control);
        
        /**
         * 获取执行器
         * @param control 控制对象指针
         * @return 控制执行器指针
         */
        ControlExecutor* getExecutor(Control* control);

        // 待删除执行器队列
        std::queue<ControlExecutor*> mTobeDeletedExecutorQ;
        std::mutex                   mTobeDeletedExecutorQ_mtx;
        std::condition_variable      mTobeDeletedExecutorQ_cv;
        
        /**
         * 处理删除执行器
         */
        void handleDeleteExecutor();

        // 报警处理 start
        // 报警循环线程
        std::thread* mLoopAlarmThread;
        
        /**
         * 报警循环线程函数
         * @param arg 线程参数
         */
        static void loopAlarmThread(void* arg);
        
        // 报警队列
        std::queue<AVSAlarm*> mAlarmQ;
        std::mutex            mAlarmQ_mtx;
        
        /**
         * 获取报警
         * @param alarm 报警对象指针引用
         * @param alarmQSize 报警队列大小
         * @return 是否获取成功
         */
        bool getAlarm(AVSAlarm*& alarm, int& alarmQSize);
        
        /**
         * 清空报警队列
         */
        void clearAlarmQueue();

        // 报警图片队列
        std::queue<AVSAlarmImage* > mAlarmImageQ;
        std::mutex                  mAlarmImageQ_mtx;

        // 报警处理 end

    };
}

#endif //ANALYZER_SCHEDULER_H