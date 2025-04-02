# 说明

1. 本文件将对make所需的变量进行说明，避免混乱
2. 本文件假设项目源码在`~/Yukino`中

# GUNmakefile

1. 在大多数情况下，make 默认会查找以下三个文件之一，这里使用GNUmakefile是因为后续会使用CMake生成Makefile，在清除时为了避免误删，所以文件名设置成优先级最高的那个格式。
   1. GNUmakefile（优先级最高）
   2. makefile
   3. Makefile（一般情况下的标准名称）
2. `ROOT_DIR`：当前Makefile文件所在的目录，即`~/Yukino`
3. `ALL_TARGETS`：**make变量**，所有支持的目标，即可以通过make后面跟上参数执行的命令，比如`make install`，如果只执行`make`，则目标默认为`all`。
4. `DEFAULT_BUILD_DIR`：默认构建目录，用于存放构建过程生成的中间文件，默认为`~/Yukino/build.cmake`
5. `BUILD_DIR`：当前构建目录。如果当前目录下，即`~/Yukino`下存在Makefile，则使用当前目录作为构建目录，否则使用默认构建目录。
6. `CMAKE3`：检测`cmake3`命令是否可用，否则使用`cmake`命令。
7. `WORKFLOW`：检测`~/Yukino/workflow`目录下是否存在 `workflow-config.cmake.in`文件，有的话则赋值为`Found`，没有则为`NotFound`。
8. `all`：默认目标all，依赖于base，all会在`BUILD_DIR`下执行make命令。
9. `base`：
   1.  如果`WORKFLOW`为`Found`，则在`~/Yukino/workflow`目录下执行make命令。
   2.  创建`BUILD_DIR`目录。
   3.  在`BUILD_DIR`目录下执行`CMAKE3`命令，指定CMake配置文件路径为`ROOT_DIR`。
   4.  如果执行`make DEBUG=y`，则会执行`CMAKE3 -D CMAKE_BUILD_TYPE=Debug`，即修改CMake中的一个变量，使其进入debug模式。
   5.  如果执行`make INSTALL_PREFIX=/path/to/install`，则会执行`CMAKE3 -DCMAKE_INSTALL_PREFIX:STRING=${INSTALL_PREFIX}`，即修改本项目的安装路径。
10. `example`：依赖于 all 目标，在`~/Yukino/example`下执行make命令
11. `check`：依赖于 all 目标，在`~/Yukino/test`下执行`make check`命令
12. `install preinstall package`：创建`BUILD_DIR`目录，在`BUILD_DIR`目录下执行`CMAKE3`命令，指定CMake配置文件路径为`ROOT_DIR`。并且此时执行命令会传递，即如果在根目录下执行`make install`，那么在`BUILD_DIR`目录下也会执行`make install`。
13. `clean`：清理构建目录和生成的头文件和库文件目录，删除所有的Makefile文件。

# ~/Yukino/CMakeLists_Headers.txt
1. `SRC_HEADERS`：列出`src`目录下的所有头文件
2. `INCLUDE_HEADERS`：赋值为`SRC_HEADERS`

# ~/Yukino/Yukino-config.cmake.in
1. CMake项目导出配置文件，对于同样使用CMake配置的项目，可以在其`CMakeLists.txt`文件中使用`find_package(Yukino)`来加载变量，比如Yukino库的头文件路径和库文件路径这些都作为变量加载为`YUKINO_INCLUDE_DIR`和`YUKINO_LIB_DIR`
2. 注意，该文件并不是最终的配置文件，而是一个模板，里面有一些占位符的不定变量，通过在项目的CMakeLists.txt文件中去赋值，然后生成最终的`.cmake`配置文件

# ~/Yukino/CMakeLists.txt
1. `CMAKE_BUILD_TYPE`：**CMake变量**，构建类型，默认为`RelWithDebInfo`（带调试信息的发布模式），并存入缓存（如果通过命令行修改了该变量的值，CMake会把这个设置保存在缓存中。下次运行 CMake 时，它会使用这个值）。
2. `INC_DIR`：**自定义变量**，头文件路径，值为`~/Yukino/_include`
3. `LIB_DIR`：**自定义变量**，库文件路径，值为`~/Yukino/_lib`
4. `GNUInstallDirs`：**CMake模块**，项目安装到系统目录相关的CMake 模块，它自动根据系统的默认路径来设置这些目录，帮助跨平台安装。
   1. `CMAKE_INSTALL_PREFIX（安装路径的根目录）`：`/usr/local或C:/Program Files/<project_name>`
   2. `CMAKE_INSTALL_BINDIR（二进制文件安装目录）`：`/usr/local/bin或C:/Program Files/<project_name>/bin`
   3. `CMAKE_INSTALL_LIBDIR（库文件安装目录）`：`/usr/local/lib或C:/Program Files/<project_name>/lib`
   4. `CMAKE_INSTALL_INCLUDEDIR（头文件安装目录）`：`/usr/local/include或C:/Program Files/<project_name>/include`
   5. `CMAKE_INSTALL_DOCDIR（文档安装目录）`：`/usr/local/share/doc/<project_name>或C:/Program Files/<project_name>/doc`
5. `CMAKE_CONFIG_INSTALL_FILE`：**自定义变量**，CMake项目导出配置文件在当前源码目录下的路径，值为`~/Yukino/build.cmake/config.toinstall.cmake`。后续该文件会被安装到系统目录。
6. `CMAKE_CONFIG_INSTALL_DIR`：**自定义变量**，CMake项目导出配置文件要安装到系统目录下的位置，值为`/usr/local/lib/cmake/Yukino`
7. `CMAKE_LIBRARY_OUTPUT_DIRECTORY`：**CMake变量**，动态库文件的输出路径，与`LIB_DIR`一样。
8. `CMAKE_ARCHIVE_OUTPUT_DIRECTORY`：**CMake变量**，静态库文件的输出路径，与`LIB_DIR`一样。
9. `LINK_HEADERS`：**自定义make目标**，在执行`make all`时会自动执行，用于管理头文件。
10. `INCLUDE_HEADERS`：**自定义变量**，是所有头文件的路径，由`INCLUDE(CMakeLists_Headers.txt)`命令导入。通过这个变量会可以将其所有头文件拷贝到`~/Yukino/_include/Yukino`中，并且这个过程会绑定在`LINK_HEADERS`目标执行之前。
11. 通过`add_subdirectory(src)`导入`~/Yukino/src`目录下的`CMakeLists.txt`文件，相当于是将代码拷贝进来，但有些变量是根据其位置动态变化的，在当前文件和子文件内是不一样的，这一点要注意。
12. 通过`Yukino-config.cmake.in`去生成配置文件为`Yukino-config.cmake`，但该配置文件导出的头文件路径变量和库文件路径变量指向了当前源码目录下的`_include`和`_lib`，并且生成的配置文件是保存在`~/Yukino`，实际上是给开发环境用的，并不会安装到系统目录下。
13. 通过`Yukino-config.cmake.in`去生成配置文件为`~/Yukino/build.cmake/config.toinstall.cmake`，并且其头文件路径和库文件路径指向了系统目录下的真正保存路径。
14. 后续有3个安装命令，在通过`Cmake`生成`makefile`文件后，需要执行`make install`才会触发，主要是拷贝并重命名`Yukino/build.cmake/config.toinstall.cmake`文件到系统目录下，拷贝头文件到系统目录下，拷贝文档文件到系统目录下。

# ~/Yukino/src/CMakeLists.txt

1. `CMAKE_SYSTEM_LIBRARY_PATH`：**CMake变量**，系统库文件目录，读取并将当前环境变量`LIBRARY_PATH`添加到该变量中。
2. `CMAKE_SYSTEM_INCLUDE_PATH`：**CMake变量**，系统头文件目录，读取并将当前环境变量`INCLUDE_PATH`添加到该变量中。
3. 加载`OpenSSL`和`Workflow`项目的变量，通过`find_package()`加载。
4. 设置当前项目的头文件路径为`OPENSSL_INCLUDE_DIR、WORKFLOW_INCLUDE_DIR、CMAKE_CURRENT_SOURCE_DIR和~/Yukino/_include/Yukino`目录中。其中`CMAKE_CURRENT_SOURCE_DIR`表示当前`CMakeLists.txt`所在目录，即`~/Yukino/src`。
5. 添加`base、util和core`子目录，按顺序读取其`CMakeLists.txt`文件，这几个文件生成的目标为`base、util和core`，并且使其依赖于`LINK_HEADERS`目标，即必须在`LINK_HEADERS`完成后在构建。
6. 将`base、util和core`联合生成静态库，名为Yukino，并且使其依赖于`LINK_HEADERS`目标，即必须在`LINK_HEADERS`完成后在构建。
7. 安装静态库到系统目录下。
8. 生成动态库并安装到系统目录下。
9. 生成`Yukino-targets.cmake`文件，它记录了整个项目的具体信息，供`Yukino-config.cmake`文件调用。他们两者的区别如下：
   1.  `config.cmake`主要用于提供库的基本信息，如这个库在哪个目录下，这个库有哪些头文件。
   2.  `targets.cmake`主要用于提供具体信息，如这个库的目标名称是什么，这个库的依赖项有哪些。