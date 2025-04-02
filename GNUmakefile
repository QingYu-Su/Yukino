# 获取当前 Makefile 所在的目录，作为根目录
ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

# 定义所有支持的目标
ALL_TARGETS := all base check install preinstall package clean example

# 定义了Makefile文件名
MAKE_FILE := Makefile

# 默认构建目录
DEFAULT_BUILD_DIR := build.cmake
# 如果当前目录下存在 Makefile，则使用当前目录作为构建目录，否则使用默认构建目录
BUILD_DIR := $(shell if [ -f $(MAKE_FILE) ]; then echo "."; else echo $(DEFAULT_BUILD_DIR); fi)

# 检测 cmake3 是否可用，否则使用 cmake
CMAKE3 := $(shell if which cmake3>/dev/null ; then echo cmake3; else echo cmake; fi;)

# 检测 workflow 目录下是否存在 workflow-config.cmake.in 文件
WORKFLOW := $(shell if [ -f "workflow/workflow-config.cmake.in" ]; then echo "Found"; else echo "NotFound"; fi)

# 声明所有目标为伪目标，这些目标不是文件，而是任务，不管目录下有没有同名文件，make都会执行它们
.PHONY: $(ALL_TARGETS)

# 默认目标 all，直接运行make会执行该任务，用于编译并生成所需的程序
# 依赖于 base，在执行 all 之前，make 必须先执行 base
# -C指定了在哪个目录执行make，-f指定了要执行的 Makefile 文件名
all: base
	make -C $(BUILD_DIR) -f Makefile

# 基础构建目标
base:

# 如果 workflow目录下存在 workflow-config.cmake.in 文件，则进入 workflow 目录并执行 make
ifeq ("$(WORKFLOW)","Found")
	make -C workflow
endif

# 创建构建目录
	mkdir -p $(BUILD_DIR)

# 根据 DEBUG 变量或 INSTALL_PREFIX 变量决定 CMake 配置参数，然后执行cmake操作，生成最终的makefile。
# 执行make DEBUG=y，则会触发debug模式
# 执行make INSTALL_PREFIX=/path/to/install， 则会触发自定义安装模式
ifeq ($(DEBUG),y)
	cd $(BUILD_DIR) && $(CMAKE3) -D CMAKE_BUILD_TYPE=Debug $(ROOT_DIR)
else ifneq ("${INSTALL_PREFIX}install_prefix", "install_prefix")
	cd $(BUILD_DIR) && $(CMAKE3) -DCMAKE_INSTALL_PREFIX:STRING=${INSTALL_PREFIX} $(ROOT_DIR)
else
	cd $(BUILD_DIR) && $(CMAKE3) $(ROOT_DIR)
endif

# example目标，在example目录下执行make编译并生成所需的程序，依赖于 all 目标
example: all
	make -C example

# test目标，在test目录下执行make check编译并生成所需的程序(只生成check目标)，依赖于 all 目标
check: all
	make -C test check

# 安装、预安装、打包，依赖于 base 目标
# 它会创建BUILD_DIR目录并进入，执行cmake命令，然后执行make命令，最后的$@表示当前的make目标，比如当前使用make install，则在BUILD_DIR目录下，执行make install
install preinstall package: base
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE3) $(ROOT_DIR)
	make -C $(BUILD_DIR) -f Makefile $@

# 清理构建目录和生成文件，删除所有的Makefile文件，所以本文件才需要命名为GUNmakefile，这样就不会误删了
clean:
ifeq ("$(WORKFLOW)","Found")
	-make -C workflow clean
endif
	-make -C test clean
	-make -C example clean
	rm -rf $(DEFAULT_BUILD_DIR)
	rm -rf _include
	rm -rf _lib
	rm -f SRCINFO SRCNUMVER SRCVERSION
	find . -name CMakeCache.txt | xargs rm -f
	find . -name Makefile       | xargs rm -f
	find . -name "*.cmake"      | xargs rm -f
	find . -name CMakeFiles     | xargs rm -rf