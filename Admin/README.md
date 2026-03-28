#Admin 视频行为分析系统v1 后台管理

#### 环境
| 程序         | 版本      |
| ---------- | ------- |
| python     | 3.7+    |
| 依赖库      | requirements.txt |

### 安装依赖库
~~~

# 创建虚拟环境
python3 -m venv venv

# 切换到虚拟环境
venv\Scripts\activate

# 更新虚拟环境的pip版本
python3 -m pip install --upgrade pip -i https://pypi.tuna.tsinghua.edu.cn/simple

# 在虚拟环境中安装依赖库
python3 -m pip install -r requirements.txt -i https://pypi.tuna.tsinghua.edu.cn/simple

~~~

### 启动

~~~

// 初始化数据库
python3 manage.py migrate

// 启动后台管理服务
// 启动前确认 9001 端口没有被 rustfs 占用 sudo systemctl stop  rustfs
python3 manage.py runserver 0.0.0.0:9001

// 默认用户
用户名：admin 密码：admin888

~~~

## API 接口与前端页面对应表

| API方法 | URL | 对应前端页面 | 说明 |
|--------|-----|-------------|------|
| `api_analyzerGetControls` | `/analyzerGetControls` | `/control` - 布控列表页面 | 获取布控列表，同步视频流/布控/数据库状态 |
| `api_getStreams` | `/getStreams` | `/stream` - 视频流列表页面 | 获取所有在线视频流 |
| `api_allCameraPushStream` | `/allCameraPushStream` | `/camera/add` - 摄像头添加页面 | 一键推流按钮，将所有摄像头推流至ZLMediaKit |
| `api_getIndex` | `/getIndex` | `/` - 系统首页/仪表盘 | 获取系统资源使用情况 |
| `api_controlAdd` | `/controlAdd` | `/control/add` - 添加布控页面 | 添加布控数据 |
| `api_controlEdit` | `/controlEdit` | `/control/edit` - 编辑布控页面 | 编辑布控数据 |
| `api_analyzerControlAdd` | `/analyzerControlAdd` | `/control` - 布控列表页面 | 启动布控按钮，启动行为识别分析 |
| `api_analyzerControlCancel` | `/analyzerControlCancel` | `/control` - 布控列表页面 | 停止布控按钮，停止行为识别分析 |
| `api_getVerifyCode` | `/getVerifyCode` | `/login` - 登录页面 | 获取验证码图片 |

