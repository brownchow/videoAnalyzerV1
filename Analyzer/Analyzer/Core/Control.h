#ifndef ANALYZER_CONTROL_H
#define ANALYZER_CONTROL_H

#include <string>

/**
 * 布控相关结构体
 * @author sysbreak
 * @since 2026-04-01
 */
namespace AVSAnalyzer {

	/**
	 * 布控请求必需参数
	 * 
	 * @author sysbreak
	 * @since 2026-04-01
	 */
	struct Control
	{
	public:

	    /**
		 * 布控编号
		 */
		std::string code;
		
		/**
		 * 布控流 url
		 */
		std::string streamUrl;
		
		/**
		 * 是否推流，默认 false 不推流
		 */
		bool        pushStream = false;
		
		/**
		 * 推流 url
		 */
		std::string pushStreamUrl;
		
		/**
		 * 行为编码
		 */
		std::string behaviorCode;

		/**
		 * 同一布控最小的报警间隔时间（单位毫秒）
		 */
		int64_t alarmMinInterval = 30;

	public:
		// 通过计算获得的参数
		/**
		 * 执行器启动时毫秒级时间戳（13位）
		 */
		int64_t executorStartTimestamp = 0;

		/**
		 * 算法检测的帧率（每秒检测的次数）
		 */
		float   checkFps = 0;

		/**
		 * 布控视频流的像素宽
		 */
		int     videoWidth = 0;

		/**
		 * 布控视频流的像素高
		 */
		int     videoHeight = 0;

		/**
		 * 
		 */
		int     videoChannel = 0;

		/**
		 * 
		 */
		int     videoIndex = -1;

		/**
		 * 
		 */
		int     videoFps = 0;

		/**
		 * 
		 */
		int     audioIndex = -1;

		/**
		 * 
		 */
		int     audioFps = 0;

	public:

		/**
		 * 新增校验
		 * 
		 * @return result_msg
		 */
		bool validateAdd(std::string& result_msg) {
			if (code.empty() || streamUrl.empty() || behaviorCode.empty()) {
				result_msg = "validate parameter error";
				return false;
			}
			if (pushStream) {
				if (pushStreamUrl.empty()) {
					result_msg = "validate parameter pushStreamUrl is error: " + pushStreamUrl;
					return false;
				}
			}
			result_msg = "validate success";
			return true;
		}

		/**
		 * 取消校验
		 * @return result_msg
		 */
		bool validateCancel(std::string& result_msg) {
			if (code.empty()) {
				result_msg = "validate parameter error";
				return false;
			}
			result_msg = "validate success";
			return true;
		}

	};
}
#endif //ANALYZER_CONTROL_H
