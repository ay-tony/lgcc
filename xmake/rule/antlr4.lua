rule("antlr4")
    add_deps("c++")
    set_extensions(".g4")

    on_load(function (target)
        -- 添加 antlr4 c++ 运行时头文件
        local sysincludedir = target:extraconf("rules", "antlr4", "sysincludedir") 
        target:add("sysincludedirs", sysincludedir, { public = true })

        -- 添加 antlr4 c++ 运行时库
        local syslink = target:extraconf("rules", "antlr4", "syslink")
        target:add("syslinks", syslink, { public = true })

        -- 检查并获取 antlr4 命令
        import("lib.detect.find_tool")
        local antlr4 = assert(
            find_tool("antlr4", { check = function (tool) 
                os.run("%s", tool)
            end }),
            "antlr4 not found!")

        -- 生成 .cpp 和 .h 文件的临时存放目录，并加入头文件搜索
        local gendir = path.absolute(path.join(target:autogendir(), "rules", "antlr4"))
        os.mkdir(gendir)
        target:add("includedirs", gendir, { public = true })

        -- 遍历所有 .g4 文件
        for _, sourcefile in ipairs(target:get("files")) do
            if (path.extension(sourcefile) == ".g4") then
                -- 开始生成 .cpp 和 .h 文件
                -- batchcmds:show_progress(opt.progress, "${color.build.object}compiling.antlr4 %s", sourcefile)
                os.cd(path.absolute(path.directory(sourcefile)))
                os.vrunv(antlr4.program, {
                    "-visitor",
                    "-Dlanguage=Cpp",
                    "-o",
                    gendir,
                    path.absolute(path.filename(sourcefile))})
                os.cd("-")

                -- 添加生成的 .cpp 文件到源文件
                for _, genfile in ipairs(os.files(path.join(gendir, "*.cpp"))) do
                    target:add("files", genfile)
                end
            end
        end
    end)

