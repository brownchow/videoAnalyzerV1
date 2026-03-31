#include "Core/Config.h"
#include "Core/Scheduler.h"
#include "Core/Server.h"
#include "../AVSAlgorithm/include/AVSAlgorithm.h"

using namespace AVSAnalyzer;
using namespace AVSAlgorithmLib;

#include <clocale>

int main(int argc, char** argv)
{
	// Set locale for proper character display
	setlocale(LC_ALL, "");
#ifdef WIN32
	srand(time(NULL));//时间初始化
#else
	srand(time(NULL));//时间初始化
#endif // WIN32

	const char* file = NULL;
	const char* ip = "0.0.0.0";
	short port = 9002;

	for (int i = 1; i < argc; i += 2)
	{
		if (argv[i][0] != '-')
		{
			printf("parameter error:%s\n", argv[i]);
			return -1;
		}
		switch (argv[i][1])
		{
			case 'h': {
				// Help message
				//打印help信息
				printf("-h 打印参数配置信息并退出\n");
				printf("-f 配置文件    如：-f conf.json \n");
				printf("-i api服务IP   如：-i 0.0.0.0 \n");
				printf("-p api服务端口 如：-p 9002 \n");
				// Removed Windows-specific pause command
				// system("pause\n"); 
				exit(0); 
				return -1;
			}
			case 'f': {
				file = argv[i + 1];
				break;
			}
			case 'i': {
				ip = argv[i + 1];
				break;
			}
			case 'p': {
				port = atof(argv[i + 1]);
				break;
			}
			default: {
				printf("set parameter error:%s\n", argv[i]);
				return -1;

			}
		}
	}

	// If no config file specified, default to conf.json in current directory
	if (file == NULL) {
		file = "conf.json";
	}


	Config config(file,ip,port);
	if (!config.mState) {
		printf("failed to read config file: %s\n", file);
		return -1;
	}

	config.show();


	AlgorithmConfig algorithmConfig;
	algorithmConfig.algorithmType = config.algorithmType;
	algorithmConfig.algorithmPath = config.algorithmPath;
	algorithmConfig.algorithmDevice = config.algorithmDevice;
	algorithmConfig.algorithmInstanceNum = config.algorithmInstanceNum;
	algorithmConfig.algorithmApiHosts = config.algorithmApiHosts;

	AVSAlgorithm_Init(&algorithmConfig);

	Scheduler scheduler(&config);
	Server server;
	server.start(&scheduler);
	scheduler.loop();

	AVSAlgorithm_Destory();
	return 0;
}