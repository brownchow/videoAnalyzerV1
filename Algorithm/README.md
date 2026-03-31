# Algorithm 视频行为分析系统v1 算法服务

### 环境
| 程序         | 版本      |
| ---------- | ------- |
| python     | 3.7+    |
| 依赖库      | requirements.txt |

### 安装依赖库
~~~

# 删除旧虚拟环境
rm -rf myenv

# 创建虚拟环境
python3 -m venv myenv

# 切换到虚拟环境
source myenv/bin/activate

# 更新虚拟环境的pip版本
pip install -U pip

# 在虚拟环境中安装依赖库
pip install -r requirements.txt

~~~

### 启动

~~~
python3 AlgorithmApiServer.py --port 9003

~~~

