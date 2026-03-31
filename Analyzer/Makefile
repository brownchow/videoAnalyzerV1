# Makefile for Analyzer module - Linux version
# Video Behavior Analyzer System v1

CC = g++
CFLAGS = -std=c++11 -Wall -Wextra -pthread -I/usr/include/python3.10
LDFLAGS = -pthread

# Include directories
INCLUDES = -I./Analyzer/Core \
           -I./Analyzer/Core/Utils \
           -I./AVSAlgorithm/include \
           -I./AVSAlarmManage/include \
           -I/usr/include/jsoncpp \
           -I/usr/include/opencv4 \
           -I/usr/local/include \
           -I/usr/include/libavcodec \
           -I/usr/include/libavformat \
           -I/usr/include/libavutil \
           -I/usr/include/libswscale \
           -I/usr/include/libavfilter \
           -I/usr/include/libavdevice \
           -I/usr/include/libpostproc \
           -I/usr/include/libswresample

# Library directories
LIBDIRS = -L/usr/lib/x86_64-linux-gnu \
          -L/usr/local/lib \
          -L/usr/lib

# Libraries to link
LIBS = -lavcodec -lavformat -lavutil -lswscale -lswresample -lz -lbz2 -l pthread \
       -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect \
       -lopencv_video -lopencv_videoio -lopencv_imgcodecs \
       -ljsoncpp -levent -lpthread -ldl -lcurl -lpython3.10 -lturbojpeg

# Source files
SRCS = ./Analyzer/main.cpp \
       ./Analyzer/Core/Analyzer.cpp \
       ./Analyzer/Core/AvPullStream.cpp \
       ./Analyzer/Core/AvPushStream.cpp \
       ./Analyzer/Core/Config.cpp \
       ./Analyzer/Core/ControlExecutor.cpp \
       ./Analyzer/Core/GenerateAlarm.cpp \
       ./Analyzer/Core/Scheduler.cpp \
       ./Analyzer/Core/Server.cpp \
       ./AVSAlgorithm/Core/AVSAlgorithm.cpp \
       ./AVSAlgorithm/Core/Algorithm.cpp \
       ./AVSAlgorithm/Core/AlgorithmWithApi.cpp \
       ./AVSAlgorithm/Core/AlgorithmWithPy.cpp \
       ./AVSAlgorithm/Core/AlgorithmPool.cpp \
       ./AVSAlgorithm/Core/Utils/Request.cpp \
       ./AVSAlarmManage/Core/AVSAlarmManage.cpp \
       ./AVSAlarmManage/Core/GenerateVideo.cpp \
       ./AVSAlarmManage/Core/Utils/Request.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = analyzer

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LIBDIRS) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean