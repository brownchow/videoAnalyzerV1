#ifndef AVSALGORITHM_REQUEST_H
#define AVSALGORITHM_REQUEST_H
/*
curl 静态库下载地址
https://curl.se/download.html

使用vcpkg需要
下载 PowerShell-7.1.0-win-x86.zip

https://github.com/PowerShell/PowerShell/releases?page=4
https://github.com/PowerShell/PowerShell/releases/download/v7.1.0/PowerShell-7.1.0-win-x86.msi


windows 编译库

https://blog.csdn.net/houxian1103/article/details/123343248

*/
#include <string>
namespace AVSAlgorithmLib {
    class Request
    {
    public:
        Request();
        ~Request();

    public:
        bool get(const char* url, std::string& response);
        bool post(const char* url, const char* data, std::string& response);

    };
}
#endif //AVSALGORITHM_REQUEST_H