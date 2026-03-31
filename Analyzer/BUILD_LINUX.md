# Video Behavior Analyzer System v1 - Linux Ubuntu 22.04 编译指南

## 概述

本文档详细介绍了在 Ubuntu 22.04 LTS 系统上编译和运行 Video Behavior Analyzer System v1 Analyzer 模块的步骤。

## 系统要求

- Ubuntu 22.04 LTS (或其他基于 Debian 的 Linux 发行版)
- 推荐最小硬件配置：
  - CPU: 4核心
  - 内存: 8GB RAM
  - 存储: 10GB 可用空间

## 依赖库安装

在开始编译之前，需要安装以下依赖库：

```bash
# 更新包列表
sudo apt update

# 安装编译工具和依赖库
sudo apt install -y build-essential \
                    cmake \
                    git \
                    pkg-config \
                    libavcodec-dev \
                    libavformat-dev \
                    libavutil-dev \
                    libswscale-dev \
                    libavdevice-dev \
                    libavfilter-dev \
                    libpostproc-dev \
                    libswresample-dev \
                    libjsoncpp-dev \
                    libopencv-dev \
                    libevent-dev \
                    libcurl4-openssl-dev \
                    libbz2-dev \
                    libpython3.10-dev \
                    libturbojpeg0-dev \
                    python3-numpy \
                    python3-pip

# 安装OpenCV的额外模块（如果需要）
sudo apt install -y libopencv-dev python3-opencv

# 验证Python和NumPy安装
python3 -c "import numpy; print('NumPy version:', numpy.__version__)"
```

> **注意**：如果您使用的不是Python 3.10，请将 `libpython3.10-dev` 替换为对应版本的开发包，例如 `libpython3.8-dev` 或 `libpython3.9-dev`。

## 源码获取

假设您已经克隆了仓库或已有源码：

```bash
# 克隆仓库（如果尚未获取源码）
git clone <repository-url>
cd BXC_VideoAnalyzer_v1/Analyzer

# 如果已经有源码，确保在Analyzer目录中
cd /path/to/BXC_VideoAnalyzer_v1/Analyzer
```

## 编译步骤

### 1. 清理以前的编译产物（可选但推荐）

```bash
make clean
```

### 2. 编译项目

```bash
make
```

编译过程将生成名为 `analyzer` 的可执行文件。

### 3. 验证编译成功

编译成功后，您应该能看到类似以下的输出：

```
g++ ./Analyzer/main.o ./Analyzer/Core/Analyzer.o ... -o analyzer [library flags]
```

并且在当前目录中会生成 `analyzer` 可执行文件。

## 运行方式

编译成功后，您可以通过以下方式运行Analyzer：

### 基本运行

```bash
# 查看帮助信息
./analyzer -h

# 使用默认配置运行（需要conf.json文件存在）
./analyzer -f conf.json

# 指定IP和端口运行
./analyzer -f conf.json -i 0.0.0.0 -p 9002
```

### 参数说明

- `-h`：显示帮助信息
- `-f <file>`：指定配置文件路径（默认：conf.json）
- `-i <ip>`：指定API服务绑定的IP地址（默认：0.0.0.0）
- `-p <port>`：指定API服务绑定的端口（默认：9002）

## 配置文件

Analyzer需要一个配置文件（默认为`conf.json`）才能正常运行。配置文件应包含以下字段：

```json
{
  "serverIp": "0.0.0.0",
  "serverPort": 9002,
  "controlExecutorMaxNum": 10,
  "algorithmType": "opencv",  // 或 "openvino", "api", "py"
  "algorithmPath": "/path/to/algorithm",
  "algorithmDevice": "CPU",
  "algorithmInstanceNum": 2,
  "algorithmApiHosts": ["http://localhost:8000"]
}
```

请根据实际情况修改配置文件中的参数。

## 常见问题排查

### 1. 找不到库文件错误

如果在编译过程中遇到类似 `cannot find -lxxx` 的错误，请确保对应的 `-dev` 包已正确安装。

### 2. Python相关错误

如果遇到Python相关的链接错误，请检查：
- 是否安装了正确版本的 `libpython3.x-dev`
- Python开发包是否与系统中Python版本匹配

### 3. OpenCV相关错误

如果遇到OpenCV相关错误，请尝试：
- 重新安装 `libopencv-dev` 和 `python3-opencv`
- 确保OpenCV版本兼容（本项目测试使用OpenCV 4.x）

### 4. 运行时动态链接库找不到

如果运行时提示找不到某个共享库，可以使用以下命令检查：

```bash
ldd analyzer | grep "not found"
```

然后安装对应的库文件包。

## 性能优化建议

对于生产环境部署，建议：

1. 使用Release模式编译（当前Makefile已优化为Release模式）
2. 根据实际硬件调整 `algorithmInstanceNum` 和 `controlExecutorMaxNum` 参数
3. 确保充足的内存和CPU资源，特别是处理高分辨率视频流时
4. 考虑使用SSD存储以提高视频缓存读取速度

## 许可证

本项目遵循MIT许可证。详见项目根目录的LICENSE文件。

## 技术支持

如有问题，请参考项目文档或联系开发团队。