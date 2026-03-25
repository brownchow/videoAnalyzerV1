"""
API接口模块
提供系统后端API接口，包括布控管理、视频流管理、系统信息等功能
"""

from app.models import *
from app.views.ViewsBase import *
from app.utils.OSInfo import OSInfo
from app.utils.Captcha import Captcha
import threading

# 初始化验证码工具
font_path = os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + "/font/hei.ttf"
captcha = Captcha(font_path=font_path)

# URL: /analyzerGetControls
# 前端页面: /control (web_control.html) - 布控列表页面
# 前端调用: $.ajax({ url: '/analyzerGetControls', ... })
def api_analyzerGetControls(request):
    """
    获取布控列表接口
    同步获取在线视频流、在线布控数据和数据库中的布控数据，并更新布控状态
    
    Args:
        request: HTTP请求对象
    
    Returns:
        HttpResponseJson: 包含布控列表的JSON响应
        - code: 状态码，1000表示成功
        - msg: 消息描述
        - data: 布控数据列表，包含布控状态、视频流状态、检测FPS等信息
    """

    code = 0
    msg = "error"
    data = []
    data_runaway = []
    try:
        __online_streams_dict = {}  # 在线的视频流字典
        __online_controls_dict = {} # 在线的布控数据字典
        data = [] # 数据库中存储的布控数据列表

        # 定义三个并发任务
        def task1(__online_streams_dict):
            """获取在线视频流数据"""
            __streams = base_media.getMediaList()
            for d in __streams:
                if d.get("active"):
                    __online_streams_dict[d.get("code")] = d
        
        def task2(__online_controls_dict):
            """获取在线布控数据"""
            __state, __msg, __controls = base_analyzer.controls()
            for d in __controls:
                __online_controls_dict[d.get("code")] = d
        
        def task3(data):
            """获取数据库中的布控数据"""
            __data = base_djangoSql.select("select * from av_control")
            data.extend(__data)

        # 创建并启动三个线程并发执行
        t1 = threading.Thread(target=task1, args=(__online_streams_dict,))
        t2 = threading.Thread(target=task2, args=(__online_controls_dict,))
        t3 = threading.Thread(target=task3, args=(data,))

        t1.start()
        t2.start()
        t3.start()

        # 等待所有线程执行完成
        t1.join()
        t2.join()
        t3.join()

        codeSet = set() # 数据库中所有布控code的集合

        # 遍历数据库中的布控数据，更新状态
        for d in data:
            codeSet.add(d.get("code"))
            d_stream_code = "%s_%s"%(d["stream_app"],d["stream_name"])
            d["create_time"] = d["create_time"].strftime("%Y-%m-%d %H:%M")

            # 检查视频流是否在线
            if __online_streams_dict.get(d_stream_code):
                d["stream_active"] = True # 当前视频流在线
            else:
                d["stream_active"] = False # 当前视频流不在线

            # 检查布控是否在线
            __online_control = __online_controls_dict.get(d["code"])
            d["checkFps"] = "0"

            if __online_control:
                d["cur_state"] = 1 # 布控中
                d["checkFps"] = "%.2f"%float(__online_control.get("checkFps"))
            else:
                if 0 == int(d.get("state")):
                    d["cur_state"] = 0 # 未布控
                else:
                    d["cur_state"] = 5 # 布控中断

            # 如果数据库中的布控状态和最新状态不一致，需要更新至最新状态
            if d.get("state") != d.get("cur_state"):
                print("-------数据库布控状态和最新状态不一致",d.get("code"),d.get("state"),d.get("cur_state"))

                update_state_sql = "update av_control set state=%d where id=%d " % (d.get("cur_state"), d.get("id"))
                r = base_djangoSql.execute(update_state_sql)
                print(update_state_sql, r)

                # update_control = Control.objects.get(id=d.get("id"))
                # update_control.state = d.get("sync_state_latest")
                # update_control.save()

        # 检查失控的布控数据
        for code,d in __online_controls_dict.items():
            if code not in codeSet:
                # 布控数据在运行中，但却不存在本地数据表中，该数据为失控数据，需要关闭其运行状态
                print("失控的布控数据，需关闭",code,d)

        code = 1000
        msg = "success"

    except Exception as e:
        msg = str(e)


    res = {
        "code":code,
        "msg":msg,
        "data":data
    }
    return HttpResponseJson(res)

# URL: /getStreams
# 前端页面: /stream (web_stream.html) - 视频流列表页面
# 前端调用: $.ajax({ url: '/getStreams', ... })
def api_getStreams(request):
    """
    获取视频流列表接口
    获取所有在线的视频流，包括用户推流和摄像头推流，并返回视频流的详细信息
    
    Args:
        request: HTTP请求对象
    
    Returns:
        HttpResponseJson: 包含视频流列表的JSON响应
        - code: 状态码，1000表示成功
        - msg: 消息描述
        - data: 视频流数据列表，包含视频流状态、编码信息、播放地址等
    """
    code = 0
    msg = "error"
    data = []

    try:
        # 获取所有在线视频流
        streams = base_media.getMediaList()
        streams_in_camera_dict ={} # 摄像头推流的视频流字典

        # 获取数据库中的摄像头信息
        cameras = base_djangoSql.select("select * from av_camera")
        cameras_dict = {}
        # 摄像头按照code生成字典
        for camera in cameras:
            push_stream_app = camera.get("push_stream_app")
            push_stream_name = camera.get("push_stream_name")
            code = "%s_%s" % (push_stream_app, push_stream_name)
            cameras_dict[code] = camera

        # 将所有在线的视频流分为用户推流的和摄像头推流的
        for stream in streams:
            stream_code = stream.get("code")
            if cameras_dict.get(stream_code):
                # 摄像头推流
                streams_in_camera_dict[stream_code] = stream
            else:
                # 用户推流
                stream["ori"] = "用户推流"
                data.append(stream)

        # 处理所有的摄像头，如果摄像头出现在在线视频流字典中，则更新到对应视频流的状态中
        for camera in cameras:
            push_stream_app = camera.get("push_stream_app")
            push_stream_name = camera.get("push_stream_name")
            code = "%s_%s" % (push_stream_app, push_stream_name)

            camera_stream = streams_in_camera_dict.get(code,None)

            # 构建摄像头视频流信息
            stream = {
                "active": True if camera_stream else False,
                "code":code,
                "app": push_stream_app,
                "name": push_stream_name,
                "produce_speed": camera_stream.get("produce_speed") if camera_stream else "",
                "video": camera_stream.get("video") if camera_stream else "",
                "audio": camera_stream.get("audio") if camera_stream else "",
                "originUrl": camera_stream.get("originUrl") if camera_stream else "",  # 推流地址
                "originType": camera_stream.get("originType") if camera_stream else "",  # 推流地址采用的推流协议类型
                "originTypeStr": camera_stream.get("originTypeStr") if camera_stream else "",  # 推流地址采用的推流协议类型（字符串）
                "clients": camera_stream.get("clients") if camera_stream else 0,  # 客户端总数量
                "schemas_clients": camera_stream.get("schemas_clients") if camera_stream else [],
                "flvUrl": base_media.get_flvUrl(push_stream_app, push_stream_name),
                "hlsUrl": base_media.get_hlsUrl(push_stream_app, push_stream_name),
                "ori": camera.get("name") # 摄像头名称
            }
            data.append(stream)

        code = 1000
        msg = "success"

    except Exception as e:
        msg = str(e)


    res = {
        "code":code,
        "msg":msg,
        "data":data
    }
    return HttpResponseJson(res)


# URL: /allCameraPushStream
# 前端页面: /camera/add (web_camera_handle.html) - 摄像头添加页面
# 前端调用: $.ajax({ url: '/allCameraPushStream', ... }) - 一键推流按钮
def api_allCameraPushStream(request):
    """
    将数据库中保存的所有摄像头进行推流至ZLMediaKit
    
    Args:
        request: HTTP请求对象
    
    Returns:
        HttpResponseJson: 包含操作结果的JSON响应
        - code: 状态码
        - msg: 消息描述
    """
    code = 0
    msg = "error"

    try:
        # 获取数据库中的所有摄像头
        cameras = base_djangoSql.select("select * from av_camera")

        # 遍历所有摄像头，为每个摄像头创建推流代理
        for camera in cameras:
            push_stream_vhost = camera.get("push_stream_vhost")
            push_stream_app = camera.get("push_stream_app")
            push_stream_name = camera.get("push_stream_name")
            origin_url = camera.get("origin_url")
            print(camera)
            # 调用ZLMediaKit添加流代理
            key = base_media.addStreamProxy(app=push_stream_app,name=push_stream_name,origin_url=origin_url,vhost=push_stream_vhost)
            # flag = base_media.delStreamProxy(key)
            # print("key:",key,"flag:",flag)
        # code = 1000
        # msg = "success"

    except Exception as e:
        msg = str(e)

    res = {
        "code": code,
        "msg": msg,
    }
    return HttpResponseJson(res)


# URL: /getIndex
# 前端页面: / (web_index.html) - 系统首页/仪表盘
# 前端调用: $.ajax({ url: '/getIndex', ... }) - 获取系统资源使用情况
def api_getIndex(request):
    """
    获取系统首页信息接口
    获取系统资源使用情况，用于前端展示系统状态
    
    Args:
        request: HTTP请求对象
    
    Returns:
        HttpResponseJson: 包含系统信息的JSON响应
        - code: 状态码，1000表示成功
        - msg: 消息描述
        - os_info: 系统信息，包含CPU、内存、磁盘等使用情况
    """
    code = 0
    msg = "error"
    os_info = {}

    try:
        # 获取系统信息
        osSystem = OSInfo()
        os_info = osSystem.info()
        code = 1000
        msg = "success"

    except Exception as e:
        msg = str(e)

    res = {
        "code":code,
        "msg":msg,
        "os_info":os_info
    }
    return HttpResponseJson(res)


# URL: /controlAdd
# 前端页面: /control/add (web_control_handle.html) - 添加布控页面
# 前端调用: $.ajax({ url: '/controlAdd', type: 'POST', ... }) - 保存布控数据
def api_controlAdd(request):
    """
    添加或更新布控数据接口
    支持新增布控和更新已有布控数据
    
    Args:
        request: HTTP请求对象，POST方法
        - controlCode: 布控代码
        - behaviorCode: 行为识别代码
        - pushStream: 是否推流（"1"表示是）
        - remark: 备注
        - streamApp: 视频流应用名
        - streamName: 视频流名称
        - streamVideo: 视频编码信息
        - streamAudio: 音频编码信息
    
    Returns:
        HttpResponseJson: 包含操作结果的JSON响应
        - code: 状态码，1000表示成功
        - msg: 消息描述
    """
    code = 0
    msg = "error"

    if request.method == 'POST':
        # 解析POST参数
        params = parse_post_params(request)

        controlCode = params.get("controlCode")
        behaviorCode = params.get("behaviorCode")
        pushStream = True if '1' == params.get("pushStream") else False
        remark = params.get("remark")

        streamApp = params.get("streamApp")
        streamName = params.get("streamName")
        streamVideo = params.get("streamVideo")
        streamAudio = params.get("streamAudio")

        # 验证必要参数
        if controlCode and behaviorCode and streamApp and streamName and streamVideo:
            __save_state = False
            __save_msg = "error"
            try:
                # 尝试查找已有布控数据
                control = None
                try:
                    control = Control.objects.get(code=controlCode)
                except:
                    pass

                if control:
                    # 更新已有布控数据
                    control.stream_app = streamApp
                    control.stream_name = streamName
                    control.stream_video = streamVideo
                    control.stream_audio = streamAudio

                    control.behavior_code = behaviorCode
                    control.interval = 1
                    control.sensitivity = 1
                    control.overlap_thresh = 1
                    control.remark = remark
                    control.push_stream = pushStream
                    control.last_update_time = datetime.now()
                    control.save()

                    if control.id:
                        __save_state = True
                        __save_msg = "更新布控数据成功"
                    else:
                        __save_msg = "更新布控数据失败"

                else:
                    # 新增布控数据
                    control = Control()
                    control.user_id = getUser(request).get("id")
                    control.sort = 0
                    control.code = controlCode

                    control.stream_app = streamApp
                    control.stream_name = streamName
                    control.stream_video = streamVideo
                    control.stream_audio = streamAudio

                    control.behavior_code = behaviorCode
                    control.interval = 1
                    control.sensitivity = 1
                    control.overlap_thresh = 1
                    control.remark = remark

                    control.push_stream = pushStream
                    control.push_stream_app = base_media.default_push_stream_app
                    control.push_stream_name = controlCode

                    control.create_time = datetime.now()
                    control.last_update_time = datetime.now()

                    control.save()

                    if control.id:
                        __save_state = True
                        __save_msg = "添加布控数据成功"
                    else:
                        __save_msg = "添加布控数据失败"
            except Exception as e:
                __save_msg = "同步布控数据失败：" + str(e)

            if __save_state:
                code = 1000
            msg = __save_msg

        else:
            msg = "the request params is error"
    else:
        msg = "the request method is not supported"


    res = {
        "code":code,
        "msg":msg
    }
    return HttpResponseJson(res)


# URL: /controlEdit
# 前端页面: /control/edit (web_control_handle.html) - 编辑布控页面
# 前端调用: $.ajax({ url: '/controlEdit', type: 'POST', ... }) - 更新布控数据
def api_controlEdit(request):
    """
    编辑布控数据接口
    更新已有布控的行为识别代码、推流设置和备注信息
    
    Args:
        request: HTTP请求对象，POST方法
        - controlCode: 布控代码
        - behaviorCode: 行为识别代码
        - pushStream: 是否推流（"1"表示是）
        - remark: 备注
    
    Returns:
        HttpResponseJson: 包含操作结果的JSON响应
        - code: 状态码，1000表示成功
        - msg: 消息描述
    """
    code = 0
    msg = "error"

    if request.method == 'POST':
        # 解析POST参数
        params = parse_post_params(request)

        controlCode = params.get("controlCode")
        behaviorCode = params.get("behaviorCode")
        pushStream = True if '1' == params.get("pushStream") else False
        remark = params.get("remark")

        # 验证必要参数
        if controlCode and behaviorCode:
            try:
                # 查找布控数据
                control = Control.objects.get(code=controlCode)

                # 更新布控信息
                control.behavior_code = behaviorCode
                control.interval = 1
                control.sensitivity = 1
                control.overlap_thresh = 1
                control.remark = remark
                control.push_stream = pushStream

                control.last_update_time = datetime.now()
                control.save()

                if control.id:
                    code = 1000
                    msg = "更新布控数据成功"
                else:
                    msg = "更新布控数据失败"

            except Exception as e:
                msg = "更新布控数据失败：" + str(e)
        else:
            msg = "the request params is error"
    else:
        msg = "the request method is not supported"


    res = {
        "code":code,
        "msg":msg
    }
    return HttpResponseJson(res)


# URL: /analyzerControlAdd
# 前端页面: /control (web_control.html) - 布控列表页面
# 前端调用: $.ajax({ url: '/analyzerControlAdd', type: 'POST', ... }) - 启动布控按钮
def api_analyzerControlAdd(request):
    """
    启动布控分析接口
    将布控数据发送给分析器，启动行为识别分析
    
    Args:
        request: HTTP请求对象，POST方法
        - controlCode: 布控代码
    
    Returns:
        HttpResponseJson: 包含操作结果的JSON响应
        - code: 状态码，1000表示成功
        - msg: 消息描述
    """
    code = 0
    msg = "error"

    if request.method == 'POST':
        # 解析POST参数
        params = parse_post_params(request)
        print("params=",params)

        controlCode = params.get("controlCode")

        # 验证必要参数
        if controlCode:
            # 查找布控数据
            control = None
            try:
                control = Control.objects.get(code=controlCode)
            except:
                pass
            
            if control:
                # 调用分析器添加布控
                __state,__msg = base_analyzer.control_add(
                    code=controlCode,
                    behaviorCode=control.behavior_code,
                    streamUrl=base_media.get_flvUrl(control.stream_app, control.stream_name),
                    pushStream=control.push_stream,
                    pushStreamUrl=base_media.get_rtmpUrl(control.push_stream_app,control.push_stream_name),

                )
                msg = __msg
                if __state:
                    # 更新布控状态为运行中
                    control = Control.objects.get(code=controlCode)
                    control.state = 1
                    control.save()
                    code = 1000
            else:
                msg = "请先保存数据！"

        else:
            msg = "the request params is error"
    else:
        msg = "the request method is not supported"


    res = {
        "code":code,
        "msg":msg
    }
    return HttpResponseJson(res)


# URL: /analyzerControlCancel
# 前端页面: /control (web_control.html) - 布控列表页面
# 前端调用: $.ajax({ url: '/analyzerControlCancel', type: 'POST', ... }) - 停止布控按钮
def api_analyzerControlCancel(request):
    """
    取消布控分析接口
    停止指定布控的行为识别分析
    
    Args:
        request: HTTP请求对象，POST方法
        - controlCode: 布控代码
    
    Returns:
        HttpResponseJson: 包含操作结果的JSON响应
        - code: 状态码，1000表示成功
        - msg: 消息描述
    """
    code = 0
    msg = "error"

    if request.method == 'POST':
        # 解析POST参数
        params = parse_post_params(request)
        print("params=",params)

        controlCode = params.get("controlCode")
        
        # 验证必要参数
        if controlCode:
            # 查找布控数据
            control = None
            try:
                control = Control.objects.get(code=controlCode)
            except:
                pass

            if control:
                # 调用分析器取消布控
                __state, __msg = base_analyzer.control_cancel(
                    code=controlCode
                )

                msg = __msg
                if __state:
                    # 更新布控状态为未布控
                    control = Control.objects.get(code=controlCode)
                    control.state = 0
                    control.save()

                    code = 1000
            else:
                msg = "不存在该布控数据！"

        else:
            msg = "the request params is error"
    else:
        msg = "the request method is not supported"


    res = {
        "code":code,
        "msg":msg
    }
    return HttpResponseJson(res)


# URL: /getVerifyCode
# 前端页面: /login (web_login.html) - 登录页面
# 前端调用: <img src="/getVerifyCode?action=login"> 或 <img src="/getVerifyCode?action=reg">
def api_getVerifyCode(request):
    """
    基于PIL模块动态生成响应状态码图片
    :param request:
    :return:
    """
    params = parse_get_params(request)

    action = params.get("action")

    if action in ["login","reg"]:
        state, verify_code, verify_img_byte = captcha.getVerifyCode()

        key = action+"_verify_code"
        request.session[key]=verify_code

        return HttpResponse(verify_img_byte)
    else:
        return HttpResponse("error")





