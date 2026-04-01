#include "../include/AVSAlarmManage.h"
#include "Utils/Log.h"
#include "Utils/Common.h"
#include "GenerateVideo.h"

#define TESTMESSAGE "build AVSAlarmManageLib"
#pragma message(TESTMESSAGE)

namespace AVSAlarmManageLib {

    /**
     * 创建 AVSAlarmImage 对象
     * @return AVSAlarmImage 指针
     */
    AVSAlarmImage* AVSAlarmImage::Create() {
        return new AVSAlarmImage;
    }

    /**
     * 构造函数
     */
    AVSAlarmImage::AVSAlarmImage() {}

    /**
     * 析构函数
     */
    AVSAlarmImage::~AVSAlarmImage() {
        this->freeData();
    }

    /**
     * 初始化图像数据
     * @param data 图像数据
     * @param size 数据大小
     * @param width 宽度
     * @param height 高度
     * @param channels 通道数
     * 注意：这不是构造函数，需要手动调用
     */
    void AVSAlarmImage::initData(unsigned char* data, int size, int width, int height, int channels) {
        // 先释放原有数据
        this->freeData();
        
        // 赋值并复制数据
        this->size = size;
        this->data = (unsigned char*)malloc(this->size);
        memcpy(this->data, data, this->size);
        this->width = width;
        this->height = height;
        this->channels = channels;
    }

    /**
     * 释放图像数据
     */
    void AVSAlarmImage::freeData() {
        if (this->data) {
            free(this->data);
            this->data = nullptr;
        }
        this->size = 0;
        this->width = 0;
        this->height = 0;
        this->channels = 0;
    }

    /**
     * 获取图像数据
     * @return 图像数据指针
     */
    unsigned char* AVSAlarmImage::getData() {
        return this->data;
    }

    /**
     * 获取数据大小
     * @return 数据大小
     */
    int AVSAlarmImage::getSize() {
        return this->size;
    }

    /**
     * 获取图像宽度
     * @return 宽度
     */
    int AVSAlarmImage::getWidth() {
        return this->width;
    }

    /**
     * 获取图像高度
     * @return 高度
     */
    int AVSAlarmImage::getHeight() {
        return this->height;
    }

    /**
     * 获取通道数
     * @return 通道数
     */
    int AVSAlarmImage::getChannels() {
        return this->channels;
    }

    /**
     * 创建 AVSAlarm 对象
     * @param height 高度
     * @param width 宽度
     * @param fps 帧率
     * @param happen 发生时间戳
     * @param controlCode 控制代码
     * @return AVSAlarm 指针
     */
    AVSAlarm* AVSAlarm::Create(int height, int width, int fps, int64_t happen, const char* controlCode) {
        AVSAlarm* alarm = new AVSAlarm;

        alarm->height = height;
        alarm->width = width;
        alarm->fps = fps;
        alarm->happen = happen;
        alarm->controlCode = controlCode;

        return alarm;
    }

    /**
     * 构造函数
     */
    AVSAlarm::AVSAlarm() {
        LOGI("");
    }

    /**
     * 析构函数
     */
    AVSAlarm::~AVSAlarm() {
        LOGI("");
    }

    /**
     * 压缩图像
     * @param height 高度
     * @param width 宽度
     * @param channels 通道数
     * @param bgr BGR格式图像数据
     * @param image 输出图像对象
     * @return 是否压缩成功
     */
    bool AVSAlarmManage_CompressImage(int height, int width, int channels, unsigned char* bgr, AVSAlarmImage* image) {
        return Common_CompressImage(height, width, channels, bgr, image);
    }

    /**
     * 处理报警
     * @param alarm 报警对象
     * @param adminHost 管理主机
     * @param rootVideoDir 视频根目录
     * @param subVideoDirFormat 子视频目录格式
     * @return 是否处理成功
     */
    bool AVSAlarmManage_HandleAlarm(AVSAlarm* alarm,
        const char* adminHost,
        const char* rootVideoDir, const char* subVideoDirFormat) {
        // 创建视频生成对象并运行
        GenerateVideo gen(alarm);
        gen.run(rootVideoDir, subVideoDirFormat);
        return true;
    }

}