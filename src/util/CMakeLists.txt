# 指定最低 CMake 版本要求  
cmake_minimum_required(VERSION 3.6)

# 设置项目名称为 "util"  
project(util)

# 将当前二进制目录（编译输出目录）添加到头文件搜索路径  
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# 定义源文件列表  
set(SRC
    FileUtil.cc   # 文件操作工具类
    MysqlUtil.cc  # MySQL 相关操作工具类
    PathUtil.cc   # 路径处理工具类
    StrUtil.cc    # 字符串处理工具类
    UriUtil.cc    # URI 解析工具类
    CodeUtil.cc   # 编码转换工具类
)

# 创建一个 OBJECT 类型的库 "util"，包含上述源文件  
# OBJECT 库不会生成最终的静态或动态库，但可以被其他目标链接  
add_library(${PROJECT_NAME} OBJECT ${SRC})
