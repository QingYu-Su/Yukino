target("util")  -- 定义一个目标（target），名称为 "util"
    set_kind("object")  -- 设置目标类型为 "object"，即编译为对象文件（.o），不会生成可执行文件或库
    add_files("*.cc")  -- 添加当前目录下所有 `.cc` 源文件
    add_packages("workflow")  -- 添加 `workflow` 依赖包
