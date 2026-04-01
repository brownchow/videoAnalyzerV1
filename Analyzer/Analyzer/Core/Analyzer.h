#ifndef ANALYZER_ANALYZER_H
#define ANALYZER_ANALYZER_H

#include <vector>
#include "../../AVSAlgorithm/include/AVSAlgorithm.h"
using namespace AVSAlgorithmLib;
namespace AVSAnalyzer {
#define PLAY 0

#if PLAY
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#endif // PLAY

    struct Control;
    class Scheduler;

    /**
     * Analyzer 类
     * 功能：视频分析器，负责视频帧的分析和事件检测
     */
    class Analyzer
    {
    public:
        /**
         * 构造函数
         * @param scheduler 调度器指针
         * @param control 控制信息指针
         */
        explicit Analyzer(Scheduler* scheduler, Control* control);
        
        /**
         * 析构函数
         */
        ~Analyzer();
        
    public:
        /**
         * 检查视频帧
         * @param check 是否进行检查
         * @param frameCount 帧计数
         * @param data 视频帧数据（BGR格式）
         * @param happenScore 事件发生的分数
         * @return 是否检测到事件
         */
        bool checkVideoFrame(bool check, int64_t frameCount, unsigned char* data, float& happenScore);
        
        /**
         * 检查音频帧
         * @param check 是否进行检查
         * @param frameCount 帧计数
         * @param data 音频帧数据
         * @param size 数据大小
         * @return 是否检测到事件
         */
        bool checkAudioFrame(bool check, int64_t frameCount, unsigned char* data, int size);

        /**
         * 显示BGR格式的图片
         * @param data BGR格式的图片数据
         */
        void SDLShow(unsigned char* data);
        
        /**
         * 显示YUV格式的图片
         * @param linesize 每行的大小
         * @param data YUV格式的图片数据
         */
        void SDLShow(int linesize[8], unsigned char* data[8]);

    private:
        /**
         * 初始化SDL
         * @return 0表示成功，-1表示失败
         */
        int initSDL();
        
        /**
         * 销毁SDL
         * @return 0表示成功
         */
        int destorySDL();

    private:
        Scheduler* mScheduler;   ///< 调度器指针
        Control* mControl;       ///< 控制信息指针

        std::vector<AlgorithmDetectObject> mDetects; ///< 检测结果

#if PLAY
        SDL_Window* mSDLWindow{};       ///< 窗口
        SDL_Renderer* mSDLRenderer{};   ///< 渲染器
        SDL_Texture* mSDLTexture_IYUV{}; ///< YUV纹理
        SDL_Texture* mSDLTexture_BGR24{}; ///< BGR纹理
        SDL_Event mSDLEvent{};          ///< 监听事件
        SDL_Rect  mSDLRect{};           ///< 矩形区域
#endif // PLAY

    };
}
#endif //ANALYZER_ANALYZER_H