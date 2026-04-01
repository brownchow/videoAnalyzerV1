#ifndef AVSALARMMANAGE_GENERATEVIDEO_H
#define AVSALARMMANAGE_GENERATEVIDEO_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}
namespace AVSAlarmManageLib {
	struct AVSAlarmImage;
	struct AVSAlarm;

	/**
	 * GenerateVideo
	 * 负责将报警帧图像集合编码成一段视频（FLV）文件。
	 * 构造时传入 AVSAlarm 结构（包含分辨率、帧率、时间戳等信息），
	 * 执行 run() 输出视频文件。
	 */
	class GenerateVideo
	{
	public:
		GenerateVideo() = delete;
		GenerateVideo(AVSAlarm* alarm);
		~GenerateVideo();

		/**
		 * 执行视频生成流程
		 * @param rootVideoDir 输出根目录，文件名（包含随机后缀）会在内部生成
		 * @param subVideoDirFormat 子目录格式，目前参数会传递但暂不拆分
		 * @return true 表示成功，false 表示失败
		 */
		bool run(const char* rootVideoDir, const char* subVideoDirFormat);

	private:
		/**
		 * 初始化编码上下文（输出文件、编码器、流等）
		 */
		bool initCodecCtx(const char * url);

		/**
		 * 销毁并释放编码相关资源
		 */
		void destoryCodecCtx();

		AVSAlarm* mAlarm;
		AVFormatContext* mFmtCtx = nullptr;
		AVCodecContext* mVideoCodecCtx = nullptr;
		AVStream* mVideoStream = nullptr;
		int mVideoIndex = -1;
	};

}
#endif //AVSALARMMANAGE_GENERATEVIDEO_H
