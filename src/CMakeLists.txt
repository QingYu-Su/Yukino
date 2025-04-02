# 指定 CMake 最低版本要求
cmake_minimum_required(VERSION 3.6)

# ==========================
#  处理环境变量中的路径
# ==========================

# 处理 `LIBRARY_PATH` 环境变量，转换为 CMake 可识别的格式
if (NOT "$ENV{LIBRARY_PATH}" STREQUAL "")
    string(REPLACE ":" ";" LIBRARY_PATH $ENV{LIBRARY_PATH})  # 将 `:` 替换为 `;`，符合 CMake 的列表格式
    set(CMAKE_SYSTEM_LIBRARY_PATH ${LIBRARY_PATH};${CMAKE_SYSTEM_LIBRARY_PATH})  # 添加到 CMake 的系统库路径
endif ()

# 处理 `CPLUS_INCLUDE_PATH` 环境变量，转换为 CMake 可识别的格式
if (NOT "$ENV{CPLUS_INCLUDE_PATH}" STREQUAL "")
    string(REPLACE ":" ";" INCLUDE_PATH $ENV{CPLUS_INCLUDE_PATH})  # 同样进行路径分隔符转换
    set(CMAKE_SYSTEM_INCLUDE_PATH ${INCLUDE_PATH};${CMAKE_SYSTEM_INCLUDE_PATH})  # 添加到 CMake 的系统头文件路径
endif ()

# ==========================
#  查找并链接外部库
# ==========================

# 查找 OpenSSL 库，`REQUIRED` 选项表示该库是必须的，找不到就报错
find_package(OpenSSL REQUIRED)

# 定义一个布尔变量，表示是否支持 `sys/uio.h`
set (HAVE_SYS_UIO_H_01 true)

# ==========================
#  处理 Workflow 依赖项
# ==========================

if (WITH_VCPKG_TOOLCHAIN)
    # 如果使用 vcpkg 作为工具链，则查找 `Workflow` 库（CMake 配置文件模式）
    find_package(Workflow REQUIRED CONFIG)
else ()
    # 如果 `Workflow` 没有被标记为已安装，则尝试手动指定路径查找
    if (NOT WORKFLOW_INSTALLED)
        find_package(Workflow REQUIRED CONFIG HINTS ../workflow)
    endif ()
endif()

# ==========================
#  设置头文件搜索路径
# ==========================

include_directories(
    ${OPENSSL_INCLUDE_DIR}          # OpenSSL 头文件目录
    ${CMAKE_CURRENT_SOURCE_DIR}     # 当前源代码目录
    ${WORKFLOW_INCLUDE_DIR}         # Workflow 头文件目录
    ${INC_DIR}/Yukino               # 项目自身的 `Yukino` 头文件
)

# ==========================
#  编译选项
# ==========================

# 设置 C 语言编译标志
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -fPIC -pipe -std=gnu90")

# 设置 C++ 语言编译标志
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -pipe -std=c++11 -fno-exceptions")

# ==========================
#  添加子目录
# ==========================

add_subdirectory(base)  # 编译 `base` 目录
add_subdirectory(util)  # 编译 `util` 目录
add_subdirectory(core)  # 编译 `core` 目录

# ==========================
#  处理目标依赖
# ==========================

# 让 `base`, `util`, `core` 这三个模块在构建时依赖 `LINK_HEADERS`
add_dependencies(base LINK_HEADERS)
add_dependencies(util LINK_HEADERS)
add_dependencies(core LINK_HEADERS)

# ==========================
#  生成静态库
# ==========================

add_library(
    ${PROJECT_NAME} STATIC   # 定义一个静态库（`.a` 文件）
    $<TARGET_OBJECTS:base>   # 使用 `base` 目标的对象文件
    $<TARGET_OBJECTS:util>   # 使用 `util` 目标的对象文件
    $<TARGET_OBJECTS:core>   # 使用 `core` 目标的对象文件
)
add_dependencies(${PROJECT_NAME} LINK_HEADERS)  # 让静态库在构建时依赖 `LINK_HEADERS`

# ==========================
#  安装库文件
# ==========================

install(
    TARGETS ${PROJECT_NAME}  # 安装静态库
    ARCHIVE                  # 归类到 `ARCHIVE`（存档库）
    DESTINATION ${CMAKE_INSTALL_LIBDIR}  # 目标安装路径
    COMPONENT devel          # 归类到开发组件 `devel`
)

# ==========================
#  生成动态库，并且处理 macOS 和 Linux 的差异
# ==========================

if (APPLE)
    set(LIBSO ${LIB_DIR}/libYukino.a)  # macOS 使用 `.a` 作为静态库文件
else ()
    set(LIBSO ${LIB_DIR}/libYukino.so)  # Linux 使用 `.so` 作为共享库文件

    # 创建一个自定义目标 `SCRIPT_SHARED_LIB`，用于生成 `libYukino.so` 的依赖信息
    add_custom_target(
        SCRIPT_SHARED_LIB ALL
        COMMAND ${CMAKE_COMMAND} -E echo 'GROUP ( libYukino.a AS_NEEDED ( libz.so libworkflow.so) ) ' > ${LIBSO}
    )
    add_dependencies(SCRIPT_SHARED_LIB ${PROJECT_NAME})  # 让 `SCRIPT_SHARED_LIB` 依赖 `Yukino` 库
endif ()

# 安装 `libYukino.so`
install(
    FILES ${LIBSO}  # 共享库文件
    DESTINATION ${CMAKE_INSTALL_LIBDIR}  # 安装到 `lib` 目录
    COMPONENT devel  # 归类到开发组件 `devel`
)

# ==========================
#  安装 CMake 导出文件
# ==========================

# targets.cmake文件记录了项目的具体信息，如这个库的目标名称是什么，这个库的依赖项有哪些。
install (
    TARGETS ${CMAKE_PROJECT_NAME}  # 需要安装的目标
    EXPORT ${CMAKE_PROJECT_NAME}-targets  # 生成 CMake 导出文件
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}  # `.a` 静态库的安装目录
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}  # `.so` 共享库的安装目录
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}  # 可执行文件的安装目录
)

# 安装 `-targets.cmake` 目标描述文件
install (
    EXPORT ${CMAKE_PROJECT_NAME}-targets
    FILE ${CMAKE_PROJECT_NAME}-targets.cmake
    DESTINATION "${CMAKE_CONFIG_INSTALL_DIR}"  # 安装到 CMake 配置目录
)
