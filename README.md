# 项目说明

一个简单易用的Linux高性能异步Web框架，完全基于C++11实现，支持HTTP/1.0或HTTP1.1

# 项目特性
1. 基于[Sogou C++ Workflow](https://github.com/sogou/workflow)实现，可以搭建出高性能且完全异步的Web框架，方便用户使用。
2. 使用[spdlog](https://github.com/gabime/spdlog)记录运行日志，包括调试信息，警告信息和错误信息，便于系统监控与故障排查。
3. 实现动态路由支持，包括路由参数（{name}）和通配符（*），支持路由分组。
4. 支持各种HTTP服务的常见需求：HTTP头部字段处理、URL参数解析、表单数据处理（`application/x-www-form-urlencoded`和`multipart/form-data`）、Cookie处理。
5. 支持文件处理，包括发送文件，接收文件和静态文件服务。
6. 提供路由处理函数后附加异步任务的功能，并且框架会确保在异步任务完成后再发送HTTP响应。
7. 基于面向切面编程（AOP）范式，本框架支持添加中间件的功能，既可以注册全局中间件，也可以针对某个路由的处理函数添加中间件。
8. 支持HTTPS服务器，支持代理服务器，可以主动发起HTTP请求、MySQL请求和Redis请求，然后转发给客户端。

# 项目依赖

- Linux，内核版本 >= 6.0
- workflow, 版本 >= 0.9.9
- spdlog，版本 >= 1.15.2
- Cmake，版本 >= 3.6
- zlib1g-dev
- libssl-dev
- libgtest-dev
- gcc和g++

# 编译安装

## ArchLinux

1. 安装系统依赖
    ```
    pacman -S base-devel cmake zlib openssl gtest spdlog
    ```
2. 安装workflow
    ```
    git clone --recursive https://github.com/sogou/workflow.git
    cd workflow
    mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr
    make -j$(nproc)
    sudo make install
    ```
3. 安装本项目
    ```
    git clone https://github.com/QingYu-Su/Yukino.git
    cd Yukino
    make
    sudo make install
    ```

# 其他文档

1. [Make文件说明](/docs/Make.md)

# 参考实现

1. [wfrest](https://github.com/wfrest/wfrest)