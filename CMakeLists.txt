cmake_minimum_required(VERSION 3.6)  # 指定 CMake 最低版本要求

# 设置构建类型为 RelWithDebInfo（带调试信息的发布模式），并存入缓存
# CMake 会记住你选择的构建类型。在以后的构建中，即使你不再明确指定该变量的值，CMake 会使用缓存中的值
# STRING指明了变量类型，"build type"指明了变量说明
# 如果通过命令行修改了该变量的值，CMake会把这个设置保存在缓存中。下次运行 CMake 时，它会使用这个值。
set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "build type")

# 定义项目名称 Yukino，版本 0.0.1，支持 C 和 C++ 语言
project(Yukino
    VERSION 0.0.1
    LANGUAGES C CXX)

#### CHECK - 检查包含文件

include(CheckIncludeFile)     # 包含 CMake 预定义模块，可以检查 C 语言头文件是否存在
include(CheckIncludeFileCXX)  # 包含 CMake 预定义模块，可以检查 C++ 头文件是否存在

#### PREPARE - 预备阶段，定义目录路径

# 设置头文件和库文件的存放路径，存入缓存
set(INC_DIR ${PROJECT_SOURCE_DIR}/_include CACHE PATH "Yukino inc")
set(LIB_DIR ${PROJECT_SOURCE_DIR}/_lib CACHE PATH "Yukino lib")

# 包含 GNU 安装目录相关的 CMake 模块，它自动根据系统的默认路径来设置这些目录，帮助跨平台安装
# CMAKE_INSTALL_PREFIX（安装路径的根目录）：/usr/local或C:/Program Files/<project_name>
# CMAKE_INSTALL_BINDIR（二进制文件安装目录）：/usr/local/bin或C:/Program Files/<project_name>/bin
# CMAKE_INSTALL_LIBDIR（库文件安装目录）：/usr/local/lib或C:/Program Files/<project_name>/lib
# CMAKE_INSTALL_INCLUDEDIR（头文件安装目录）：/usr/local/include或C:/Program Files/<project_name>/include
# CMAKE_INSTALL_DOCDIR（文档安装目录）：/usr/local/share/doc/<project_name>或C:/Program Files/<project_name>/doc
include(GNUInstallDirs)  

# 设置 CMake 配置文件的安装路径，供其他项目使用find_package用
# PROJECT_BINARY_DIR为构建目录（即 CMake 执行时的当前目录），在执行cmake命令时自动设置，也可以在cmake文件中设置。
set(CMAKE_CONFIG_INSTALL_FILE ${PROJECT_BINARY_DIR}/config.toinstall.cmake)
set(CMAKE_CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# 设置库文件的输出路径，包括静态库和动态库
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})

# 创建一个自定义目标 LINK_HEADERS，可以手动 make <target> 来运行
# 这个目标在执行make all自动执行，用于管理头文件
add_custom_target(
    LINK_HEADERS ALL
    COMMENT "link headers..."  # 显示注释信息
)

# 包含外部 CMake 文件，处理头文件相关操作，定义INCLUDE_HEADERS变量
INCLUDE(CMakeLists_Headers.txt)

# 定义宏
# 定义一个自定义命令，附加到已有的 target 上，在 target 构建时执行
macro(makeLink src dest target)
    add_custom_command(
        TARGET ${target} PRE_BUILD  # 在目标构建前执行命令
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${dest}  # 仅在文件不同情况下进行拷贝
        DEPENDS ${dest}  # 设置依赖，确保目标文件更新
    )
endmacro()

# 在头文件链接前创建目标头文件目录
add_custom_command(
    TARGET LINK_HEADERS PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${INC_DIR}/${PROJECT_NAME}
)

# 遍历 INCLUDE_HEADERS 变量中的所有头文件，拷贝到目标路径
foreach(header_file ${INCLUDE_HEADERS})
    string(REPLACE "/" ";" arr ${header_file})  # 替换路径分隔符，比如arr = "src;base;Copyable.h"
    list(GET arr -1 file_name)  # 获取获取列表中的最后一个原始，即文件名，比如file_name = "Copyable.h"
    # 自动将头文件复制到 INC_DIR/${PROJECT_NAME}/ 目录
    makeLink(${PROJECT_SOURCE_DIR}/${header_file} ${INC_DIR}/${PROJECT_NAME}/${file_name} LINK_HEADERS)
endforeach()

# 添加 src 子目录，编译源代码
add_subdirectory(src)

#### CONFIG - 配置阶段，生成 CMake 配置文件

include(CMakePackageConfigHelpers)  # 包含 CMake 预定义的包配置辅助模块

# 设置CMake配置文件的头文件和库文件目录
# 生成其他项目所需的 CMake 配置文件，供其他项目使用find_package用
# 比如同样使用CMake配置的项目，可以使用find_package(Yukino)来加载变量，比如Yukino库的头文件路径和库文件路径
# cmake.in为模板输入文件，其中的变量用@为占位符包围起来，然后使用当前文件的变量去替代，最后生成.cmake文件
# 这里配置文件的头文件路径和库文件路径为源码目录下_include和_lib，其他项目在源码目录下即可find_package(Yukino)
# 生成的配置文件只会在源码目录下，并不会在安装在系统目录下，方便排错。
set(CONFIG_INC_DIR ${INC_DIR})
set(CONFIG_LIB_DIR ${LIB_DIR})
configure_package_config_file(
    ${PROJECT_NAME}-config.cmake.in  # 输入模板文件
    ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}-config.cmake  # 生成的配置文件
    INSTALL_DESTINATION ${CMAKE_CONFIG_INSTALL_DIR}  # 指定安装目录
    PATH_VARS CONFIG_INC_DIR CONFIG_LIB_DIR  # 替换变量
)

# 同上
# 此时头文件路径和库文件路径为对应的系统安装目录，其他项目可以通过find_package(Yukino)去使用
set(CONFIG_INC_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set(CONFIG_LIB_DIR ${CMAKE_INSTALL_LIBDIR})
configure_package_config_file(
    ${PROJECT_NAME}-config.cmake.in
    ${CMAKE_CONFIG_INSTALL_FILE}
    INSTALL_DESTINATION ${CMAKE_CONFIG_INSTALL_DIR}
    PATH_VARS CONFIG_INC_DIR CONFIG_LIB_DIR
)

# 安装 CMake 配置文件，以便 find_package(wfrest) 可以找到
install(
    FILES ${CMAKE_CONFIG_INSTALL_FILE}  # 要安装的 CMake 配置文件（config.toinstall.cmake）
    DESTINATION ${CMAKE_CONFIG_INSTALL_DIR}  # 安装目标目录，例如 /usr/local/lib/cmake/Yukino/
    COMPONENT devel  # 归类到 "devel" 组件，确保它属于开发相关文件，比如执行make install --component devel
    RENAME ${PROJECT_NAME}-config.cmake  # 重命名为 Yukino-config.cmake，便于 find_package(Yukino) 识别
)


# 安装头文件，以便其他项目可以使用 wfrest 的 API
install(
    FILES ${INCLUDE_HEADERS}  # 要安装的头文件列表
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}  # 目标安装目录
    COMPONENT devel  # 归类到 "devel" 组件，只有开发环境需要安装
)

# 安装 README.md 文档，方便用户查看项目说明
install(
    FILES README.md  # 需要安装的文档文件
    DESTINATION "${CMAKE_INSTALL_DOCDIR}-${PROJECT_VERSION}"  # 目标安装目录，带版本号
    COMPONENT devel  # 归类到 "devel" 组件，只有开发环境需要
)

