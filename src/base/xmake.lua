target("base")                  -- 定义目标名称 "base"
    set_kind("object")           -- 设置目标类型为 "object"（对象库）
    add_files("*.cc")            -- 添加所有 `.cc` 源文件
    add_packages("workflow")     -- 链接 `workflow` 库（通过 `xmake.lua` 的包管理）