# 项目说明

一个简单易用的Linux高性能异步Web框架，完全基于C++11实现

# 项目特性
1. 基于[Sogou C++ Workflow](https://github.com/sogou/workflow)实现，可以搭建出高性能且完全异步的Web框架，方便用户使用。
2. 使用spdlog库记录运行日志，包括调试信息，警告信息和错误信息，便于系统监控与故障排查。

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