target("core")  -- 定义一个构建目标，名字为 "core"
    set_kind("object")  -- 指定目标类型为 "object"（对象文件库）
    add_files("*.cc")   -- 添加所有 `.cc` 源文件
    add_files("*.c")    -- 添加所有 `.c` 源文件
    add_packages("workflow")  -- 添加对 `workflow` 这个外部库的依赖
