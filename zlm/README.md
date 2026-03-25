# ZLMediaKit 媒体服务器说明

## 简介

ZLMediaKit是一个高性能的流媒体服务器，支持RTSP、RTMP、HTTP-FLV、WebSocket-FLV、HTTP-TS、WebSocket-TS、HTTP-fMP4、WebSocket-fMP4、HLS、RTP等多种流媒体协议。

## 编译说明

### Ubuntu 22.04 编译步骤

1. **安装依赖**
   ```bash
   sudo apt-get update
   sudo apt-get install -y build-essential cmake git libssl-dev
   ```

2. **克隆代码**
   ```bash
   git clone https://github.com/ZLMediaKit/ZLMediaKit.git
   cd ZLMediaKit
   git submodule update --init
   ```

3. **编译**
   ```bash
   mkdir build
   cd build
   cmake ..
   make -j4
   ```

4. **获取编译产物**
   编译完成后，可执行文件位于 `release/linux/Debug/MediaServer`

## 使用说明

### 配置文件

- `config.ini` 是ZLMediaKit的配置文件，可根据需要修改

### 启动服务

**注意**：在Linux平台上，如果使用默认端口（如80、443、554等），需要以root用户或使用sudo模式启动。如果使用非特权端口（如8080、4430、5540等），则可以使用普通用户启动。

```bash
# 直接运行（使用非特权端口时）
./MediaServer

# 使用sudo启动（使用默认特权端口时）
sudo ./MediaServer

# 后台运行
./MediaServer -d

# 使用sudo后台运行
sudo ./MediaServer -d
```

### 停止服务

```bash
# 查找进程
ps aux | grep MediaServer

# 终止进程
kill -9 <pid>
```

## 功能特性

- 支持多种流媒体协议的推拉流
- 支持录像、截图、转码等功能
- 支持集群部署
- 支持WebRTC
- 高性能、低延迟

## 相关链接

- [ZLMediaKit GitHub](https://github.com/ZLMediaKit/ZLMediaKit)
- [ZLMediaKit Wiki](https://github.com/ZLMediaKit/ZLMediaKit/wiki/)
