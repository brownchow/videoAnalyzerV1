#ifndef AVSALARMMANAGE_AVSALARMMANAGE_H
#define AVSALARMMANAGE_AVSALARMMANAGE_H
#include <string>
#include <vector>

// Linux doesn't need __declspec
#define __DECLSPEC_INC


namespace AVSAlarmManageLib {

#ifdef __cplusplus
    extern "C" {
#endif

	struct __DECLSPEC_INC AVSAlarmImage
	{
	
		public:
		static AVSAlarmImage* Create();
		~AVSAlarmImage();
	
		private:
		AVSAlarmImage();
	
		public:
		void initData(unsigned char* data, int size, int width, int height, int channels);
		void freeData();

		/**
		 * 是否发生事件
		 */
		bool  happen = false;

		/**
		 * 发生事件的分数
		 */
		float happenScore = 0;

		unsigned char* getData();
		int getSize();
		int getWidth();
		int getHeight();
		int getChannels();

	private:
		/**
		 * 图片经过jpg压缩后的数据
		 */
		unsigned char* data = nullptr;

		/**
		 * 图片经过jpg压缩后的数据长度
		 */
		int size = 0; 
		
		/**
		 * 原图宽
		 */
		int width = 0;              
		
		/**
		 * 原图高
		 */
		int height = 0;             

		/**
		 * 原图通道长
		 */
		int channels = 0;           
	};
	struct __DECLSPEC_INC AVSAlarm
	{

	public:
		static AVSAlarm* Create(int height,int width,int fps,int64_t happen,const char* controlCode);
		~AVSAlarm();

	private:
		AVSAlarm();
	
	public:

		/**
		 * 宽度
		 */
		int width = 0;

	    /**
		 * 高度
		 */
		int height = 0;

		/**
		 * 帧率
		 * 
		*/
		int fps = 0;

		/**
		 * 发生时间
		 */
		int64_t happen = 0;

		/**
		 * 布控编号
		 */
		std::string controlCode;

		/**
		 * 封面图
		 */
		AVSAlarmImage* headImage = nullptr;

		/**
		 * 组成报警视频的图片帧
		 */
		std::vector<AVSAlarmImage*> images;
	};
	
	/**
	 * 压缩图片
	 */
	bool __DECLSPEC_INC AVSAlarmManage_CompressImage(int height, int width, int channels, unsigned char* bgr, AVSAlarmImage* image);
	
	/**
	 * 处理告警
	 */
	bool __DECLSPEC_INC AVSAlarmManage_HandleAlarm(AVSAlarm* alarm,
		const char* adminHost, 
		const char* rootVideoDir, 
		const char* subVideoDirFormat);

#ifdef __cplusplus
    }
#endif
}
#endif //AVSALARMMANAGE_AVSALARMMANAGE_H
