# 指定 CMake 的最低版本要求  
cmake_minimum_required(VERSION 3.6)

# 定义项目名称为 "core"
project(core)

# 将当前 CMake 的二进制目录作为头文件搜索路径  
# 这可以让 `#include` 语句在编译时能正确找到生成的头文件  
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# 定义源码文件列表  
set(SRC
    BluePrint.cc      # 可能是一个蓝图或路由系统的实现
    HttpContent.cc    # 处理 HTTP 内容
    HttpFile.cc       # 处理 HTTP 文件请求
    HttpServerTask.cc # 处理 HTTP 服务器任务
    RouteTable.cc     # 维护路由表
    Aspect.cc         # 可能用于 AOP（面向切面编程）
    HttpDef.cc        # 定义 HTTP 相关的常量或结构
    HttpServer.cc     # HTTP 服务器的主要实现
    Router.cc         # 负责路由匹配
    HttpCookie.cc     # 处理 HTTP Cookie
    HttpMsg.cc        # 处理 HTTP 消息（请求/响应）
    MultiPartParser.c # 解析 multipart/form-data（用于文件上传）
)

# 创建一个 OBJECT 类型的库 core  
# OBJECT 库不会单独生成可执行文件或共享库，而是用于后续链接到其他目标中
add_library(${PROJECT_NAME} OBJECT ${SRC})
