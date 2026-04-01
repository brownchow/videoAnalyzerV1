#include "Core/Config.h"         // 配置管理模块
#include "Core/Scheduler.h"      // 任务调度器
#include "Core/Server.h"         // API服务器
#include "../AVSAlgorithm/include/AVSAlgorithm.h"  // 算法库

using namespace AVSAnalyzer;      // 使用分析器命名空间
using namespace AVSAlgorithmLib;  // 使用算法库命名空间

#include <clocale>  // 用于设置本地化，支持中文显示

/**
 * 程序主入口函数
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 程序退出状态码
 */
int main(int argc, char** argv)
{
	// 设置本地化，支持中文显示
	setlocale(LC_ALL, "");

	// 初始化随机数种子
#ifdef WIN32
	srand(time(NULL));
#else
	srand(time(NULL));
#endif // WIN32

	// 命令行参数默认值
	const char* file = NULL;     // 配置文件路径
	const char* ip = "0.0.0.0";  // API服务IP地址
	short port = 9002;           // API服务端口

	// 解析命令行参数
	for (int i = 1; i < argc; i += 2)
	{
		if (argv[i][0] != '-')
		{
			printf("parameter error:%s\n", argv[i]);
			return -1;
		}
		switch (argv[i][1])
		{
			case 'h': {  // 显示帮助信息
				// 打印帮助信息
				printf("-h 打印参数配置信息并退出\n");
				printf("-f 配置文件    如：-f conf.json \n");
				printf("-i api服务IP   如：-i 0.0.0.0 \n");
				printf("-p api服务端口 如：-p 9002 \n");
				exit(0); 
				return -1;
			}
			case 'f': {  // 指定配置文件路径
				file = argv[i + 1];
				break;
			}
			case 'i': {  // 指定API服务IP
				ip = argv[i + 1];
				break;
			}
			case 'p': {  // 指定API服务端口
				port = atof(argv[i + 1]);
				break;
			}
			default: {  // 参数错误
				printf("set parameter error:%s\n", argv[i]);
				return -1;
			}
		}
	}

	// 如果未指定配置文件，默认加载当前目录下的 config.json
	if (file == NULL) {
		file = "config.json";
		printf("未指定配置文件，使用默认配置文件: %s\n", file);
	}

	// 加载配置文件
	Config config(file, ip, port);
	if (!config.mState) {
		printf("[错误] 读取配置文件失败: %s\n", file);
		printf("请确保当前目录下存在 config.json，或使用 -f 参数指定有效的配置文件\n");
		return -1;
	}

	// 显示配置信息
	printf("\n===== 配置信息 =====\n");
	config.show();
	printf("==================\n\n");

	// 初始化算法配置
	AlgorithmConfig algorithmConfig;
	algorithmConfig.algorithmType = config.algorithmType;           // 算法类型
	algorithmConfig.algorithmPath = config.algorithmPath;           // 算法路径
	algorithmConfig.algorithmDevice = config.algorithmDevice;       // 算法运行设备
	algorithmConfig.algorithmInstanceNum = config.algorithmInstanceNum; // 算法实例数量
	algorithmConfig.algorithmApiHosts = config.algorithmApiHosts;   // 算法API主机列表

	// 初始化算法库
	printf("初始化算法库...\n");
	AVSAlgorithm_Init(&algorithmConfig);
	printf("算法库初始化完成\n\n");

	// 创建调度器
	printf("创建任务调度器...\n");
	Scheduler scheduler(&config);

	// 创建并启动服务器
	printf("创建并启动API服务器...\n");
	Server server;
	server.start(&scheduler);
	printf("API服务器启动成功，监听地址: %s:%d\n\n", ip, port);

	// 启动调度器主循环，告警循环线程，1秒钟检查一次
	printf("启动调度器主循环...\n");
	scheduler.loop();

	// 销毁算法库
	printf("销毁算法库...\n");
	AVSAlgorithm_Destory();
	printf("程序退出\n");
	return 0;
}