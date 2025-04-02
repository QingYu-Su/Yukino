# 指定最低需要的 CMake 版本
cmake_minimum_required(VERSION 3.6)

# 定义项目名称 "base"
project(base)

# 将当前二进制目录（build 目录）添加到头文件搜索路径
# 这样可以包含 CMake 生成的配置文件
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# 定义源文件列表
set(SRC
    base64.cc       # 负责 Base64 编解码功能
    ErrorCode.cc    # 处理错误码定义
    Compress.cc     # 处理压缩相关功能
    SysInfo.cc      # 提供系统信息查询
    Timestamp.cc    # 处理时间戳相关操作
    Json.cc         # 处理 JSON 解析功能
)

# 创建一个 OBJECT 类型的库 "base"
# OBJECT 库不会单独生成可执行文件或共享库，而是用于后续的链接
add_library(${PROJECT_NAME} OBJECT ${SRC})
