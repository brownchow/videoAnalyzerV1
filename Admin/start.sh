#!/usr/bin/env bash
set -euo pipefail

# 进入脚本所在目录（确保从任意位置启动都可用）
cd "$(dirname "$0")"

# 可选: pip install -r requirements.txt
# pip3 install -r requirements.txt

# 启动 Django 本地服务
python3 manage.py runserver 0.0.0.0:9001
