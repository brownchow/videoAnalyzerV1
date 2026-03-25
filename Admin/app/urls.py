
"""
URL路由配置模块
定义Admin应用的所有URL路由，包括页面路由和API接口路由
"""

from django.urls import path
from .views.web import *
from .views.api import *


app_name = 'app'

urlpatterns = [
    # ============ 页面路由 ============
    # 首页
    path('', web_index),

    # 视频流相关页面
    path('stream', web_stream),                          # 视频流列表页面
    path('stream/play', web_stream_play),                 # 视频流播放页面

    # 摄像头相关页面
    path('camera/add', web_camera_add),                  # 添加摄像头页面

    # 告警相关页面
    path('alarm', web_alarm),                            # 告警列表页面

    # 行为识别相关页面
    path('behavior', web_behavior),                       # 行为识别管理页面

    # 布控相关页面
    path('control', web_control),                        # 布控列表页面
    path('control/add', web_control_add),                 # 添加布控页面
    path('control/edit', web_control_edit),               # 编辑布控页面

    # 其他功能页面
    path('warning', web_warning),                        # 警告页面
    path('profile', web_profile),                        # 个人资料页面
    path('notification', web_notification),                # 通知页面

    # 用户认证相关页面
    path('login', web_login),                           # 登录页面
    path('logout', web_logout),                         # 退出登录

    # ============ API接口路由 ============
    # 视频流相关API
    path('allCameraPushStream', api_allCameraPushStream), # 将所有摄像头推流至ZLMediaKit
    path('getStreams', api_getStreams),                 # 获取视频流列表

    # 布控管理相关API
    path('controlAdd', api_controlAdd),                  # 添加或更新布控数据
    path('controlEdit', api_controlEdit),                # 编辑布控数据
    path('analyzerControlAdd', api_analyzerControlAdd),  # 启动布控分析
    path('analyzerControlCancel', api_analyzerControlCancel), # 取消布控分析
    path('analyzerGetControls', api_analyzerGetControls), # 获取布控列表

    # 系统信息相关API
    path('getIndex', api_getIndex),                     # 获取系统首页信息

    # 验证码相关API
    path('getVerifyCode', api_getVerifyCode),           # 获取验证码图片
]