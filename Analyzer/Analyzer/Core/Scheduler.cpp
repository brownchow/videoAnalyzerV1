#include "Scheduler.h"
#include "Config.h"
#include "Control.h"
#include "ControlExecutor.h"
#include "Utils/Log.h"

namespace AVSAnalyzer {
    /**
     * 构造函数
     * @param config 配置对象指针
     */
    Scheduler::Scheduler(Config* config) : mConfig(config), mState(false),
        mLoopAlarmThread(nullptr)
    {
        LOGI("");
    }

    /**
     * 析构函数
     */
    Scheduler::~Scheduler()
    {
        LOGI("");
        // 清空报警队列
        clearAlarmQueue();
        // 等待报警线程结束
        mLoopAlarmThread->join();
        delete mLoopAlarmThread;
        mLoopAlarmThread = nullptr;
    }

    /**
     * 获取配置对象
     * @return 配置对象指针
     */
    Config* Scheduler::getConfig() {
        return mConfig;
    }

    /**
     * 启动调度器主循环
     */
    void Scheduler::loop() {
        LOGI("Loop Start");
        // 创建并启动报警循环线程
        mLoopAlarmThread = new std::thread(Scheduler::loopAlarmThread, this);
        mLoopAlarmThread->native_handle();
        int64_t l = 0;
        while (mState)
        {
            ++l;
            // 处理待删除的执行器
            handleDeleteExecutor();
        }
        LOGI("Loop End");
    }

    /**
     * 获取所有控制列表
     * @param controls 控制对象向量引用
     * @return 控制数量
     */
    int Scheduler::apiControls(std::vector<Control*>& controls) {
        int len = 0;
        mExecutorMapMtx.lock();
        for (auto f = mExecutorMap.begin(); f != mExecutorMap.end(); ++f)
        {
            ++len;
            controls.push_back(f->second->mControl);
        }
        mExecutorMapMtx.unlock();
        return len;
    }

    /**
     * 根据控制码获取控制对象
     * @param code 控制码
     * @return 控制对象指针
     */
    Control* Scheduler::apiControl(std::string& code) {
        Control* control = nullptr;
        mExecutorMapMtx.lock();
        for (auto f = mExecutorMap.begin(); f != mExecutorMap.end(); ++f)
        {
            if (f->first == code) {
                control = f->second->mControl;
            }
        }
        mExecutorMapMtx.unlock();
        return control;
    }

    /**
     * 添加控制
     * @param control 控制对象指针
     * @param result_code 结果代码
     * @param result_msg 结果消息
     */
    void Scheduler::apiControlAdd(Control* control, int& result_code, std::string& result_msg) {
        // 检查控制是否已存在
        if (isAdd(control)) {
            result_msg = "部控器正在运行.....";
            result_code = 1000;
            return;
        }
        // 检查执行器数量是否超过限制
        if (getExecutorMapSize() >= mConfig->controlExecutorMaxNum) {
            result_msg = "the number of control exceeds the limit";
            result_code = 0;
        }
        else {
            // 创建控制执行器
            ControlExecutor* executor = new ControlExecutor(this, control);
            // 启动执行器
            if (executor->start(result_msg)) {
                // 添加到执行器映射表
                if (addExecutor(control, executor)) {
                    result_msg = "add success";
                    result_code = 1000;
                }
                else {
                    // 添加失败，释放执行器
                    delete executor;
                    executor = nullptr;
                    result_msg = "add error";
                    result_code = 0;
                }
            }
            else {
                // 启动失败，释放执行器
                delete executor;
                executor = nullptr;
                result_code = 0;
            }
        }
    }

    /**
     * 取消控制
     * @param control 控制对象指针
     * @param result_code 结果代码
     * @param result_msg 结果消息
     */
    void Scheduler::apiControlCancel(Control* control, int& result_code, std::string& result_msg) {
        // 获取控制执行器
        ControlExecutor* controlExecutor = getExecutor(control);
        if (controlExecutor) {
            // 检查执行器状态
            if (controlExecutor->getState()) {
                result_msg = "control is running, ";
            }
            else {
                result_msg = "control is not running, ";
            }
            // 移除执行器
            removeExecutor(control);
            result_msg += "remove success";
            result_code = 1000;
            return;
        }
        else {
            result_msg = "there is no such control";
            result_code = 0;
            return;
        }
    }

    /**
     * 设置调度器状态
     * @param state 状态值
     */
    void Scheduler::setState(bool state) {
        mState = state;
    }

    /**
     * 获取调度器状态
     * @return 状态值
     */
    bool Scheduler::getState() {
        return mState;
    }

    /**
     * 获取执行器映射表大小
     * @return 执行器数量
     */
    int Scheduler::getExecutorMapSize() {
        mExecutorMapMtx.lock();
        int size = mExecutorMap.size();
        mExecutorMapMtx.unlock();

        return size;
    }

    /**
     * 检查控制是否已添加
     * @param control 控制对象指针
     * @return 是否已添加
     */
    bool Scheduler::isAdd(Control* control) {
        mExecutorMapMtx.lock();
        bool isAdd = mExecutorMap.end() != mExecutorMap.find(control->code);
        mExecutorMapMtx.unlock();

        return isAdd;
    }

    /**
     * 添加执行器
     * @param control 控制对象指针
     * @param controlExecutor 控制执行器指针
     * @return 是否添加成功
     */
    bool Scheduler::addExecutor(Control* control, ControlExecutor* controlExecutor) {
        bool add = false;
        mExecutorMapMtx.lock();
        if (mExecutorMap.size() < mConfig->controlExecutorMaxNum) {
            // 检查是否已存在
            bool isAdd = mExecutorMap.end() != mExecutorMap.find(control->code);
            if (!isAdd) {
                // 添加到映射表
                mExecutorMap.insert(std::pair<std::string, ControlExecutor* >(control->code, controlExecutor));
                add = true;
            }
        }
        mExecutorMapMtx.unlock();
        return add;
    }

    /**
     * 移除执行器（加入到待实际删除队列）
     * @param control 控制对象指针
     * @return 是否移除成功
     */
    bool Scheduler::removeExecutor(Control* control) {
        bool result = false;

        mExecutorMapMtx.lock();
        auto f = mExecutorMap.find(control->code);
        if (mExecutorMap.end() != f) {
            ControlExecutor* executor = f->second;   
            // 添加到待删除队列
            std::unique_lock <std::mutex> lck(mTobeDeletedExecutorQ_mtx);
            mTobeDeletedExecutorQ.push(executor);
            mTobeDeletedExecutorQ_cv.notify_one();
            // 从映射表中删除
            result = mExecutorMap.erase(control->code) != 0;
        }
        mExecutorMapMtx.unlock();
        return result;
    }

    /**
     * 获取执行器
     * @param control 控制对象指针
     * @return 控制执行器指针
     */
    ControlExecutor* Scheduler::getExecutor(Control* control) {
        ControlExecutor* executor = nullptr;
        mExecutorMapMtx.lock();
        auto f = mExecutorMap.find(control->code);
        if (mExecutorMap.end() != f) {
            executor = f->second;
        }
        mExecutorMapMtx.unlock();
        return executor;
    }

    /**
     * 处理删除执行器
     */
    void Scheduler::handleDeleteExecutor() {
        std::unique_lock <std::mutex> lck(mTobeDeletedExecutorQ_mtx);
        mTobeDeletedExecutorQ_cv.wait(lck);
        // 处理所有待删除的执行器
        while (!mTobeDeletedExecutorQ.empty()) {
            ControlExecutor* executor = mTobeDeletedExecutorQ.front();
            mTobeDeletedExecutorQ.pop();
            LOGI("code=%s,streamUrl=%s", executor->mControl->code.data(), executor->mControl->streamUrl.data());
            // 释放执行器
            delete executor;
            executor = nullptr;
        }
    }

    /**
     * 报警循环线程函数
     * @param arg 线程参数
     */
    void Scheduler::loopAlarmThread(void* arg) {
        Scheduler* scheduler = (Scheduler*)arg;
        AVSAlarm* alarm = nullptr;
        int alarmQSize;

        bool ret = false;
        while (true) {
            // 每1秒检查一次报警队列
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            // 从队列中获取告警图片
            ret = scheduler->getAlarm(alarm, alarmQSize);
            if (ret) {
                LOGI("发送（1）条报警，剩余待报警=%d,mAlarmImageInstanceCount=%d", alarmQSize, scheduler->mAlarmImageInstanceCount);
                // 处理报警
                AVSAlarmManage_HandleAlarm(alarm, "", scheduler->mConfig->rootVideoDir.c_str(), "%Y/%m/%d-%H-%M");
                // 将组成告警视频的图片，一张张释放，释放Alarm的图片资源
                for (int i = 0; i < alarm->images.size(); i++)
                {
                    AVSAlarmImage* image = alarm->images[i];
                    if (image) {
                        scheduler->giveBackAlarmImage(image);
                    }
                }
                alarm->images.clear();
                // 释放报警对象
                delete alarm;
                alarm = nullptr;
            }
        }
    }

    /**
     * 添加报警，将告警对象添加到队列
     * @param alarm 报警对象指针
     */
    void Scheduler::addAlarm(AVSAlarm* alarm) {
        mAlarmQ_mtx.lock();
        mAlarmQ.push(alarm);
        mAlarmQ_mtx.unlock();
    }

    /**
     * 从队列池获取一个压缩图片的实例
     * @return 报警图片对象指针
     */
    AVSAlarmImage* Scheduler::gainAlarmImage() {
        AVSAlarmImage* image = nullptr;
        mAlarmImageQ_mtx.lock();
        if (mAlarmImageQ.empty()) {
            mAlarmImageQ_mtx.unlock();
            // 队列为空，创建新实例
            image = AVSAlarmImage::Create();
            mAlarmImageInstanceCount++;
        }
        else {
            // 从队列中获取
            image = mAlarmImageQ.front();
            mAlarmImageQ.pop();
            mAlarmImageQ_mtx.unlock();
        }
        return image;
    }

    /**
     * 将一个压缩图片的实例归还到队列池
     * @param image 报警图片对象指针
     */
    void Scheduler::giveBackAlarmImage(AVSAlarmImage* image) {
        // 释放图片数据
        image->freeData();
        // 归还到队列
        mAlarmImageQ_mtx.lock();
        mAlarmImageQ.push(image);
        mAlarmImageQ_mtx.unlock();
    }

    /**
     * 获取报警，从队列中获取对应告警对象
     * @param alarm 报警对象指针引用
     * @param alarmQSize 报警队列大小
     * @return 是否获取成功
     */
    bool Scheduler::getAlarm(AVSAlarm*& alarm, int& alarmQSize) {
        mAlarmQ_mtx.lock();
        if (!mAlarmQ.empty()) {
            // 从队列中获取报警
            alarm = mAlarmQ.front();
            mAlarmQ.pop();
            alarmQSize = mAlarmQ.size();
            mAlarmQ_mtx.unlock();
            return true;
        }
        else {
            alarmQSize = 0;
            mAlarmQ_mtx.unlock();
            return false;
        }
    }

    /**
     * 清空报警队列
     */
    void Scheduler::clearAlarmQueue() {}

}
