rule("antlr4")
    add_deps("c++")
    set_extensions(".g4")

    on_load(function (target)
        -- 添加相关选项，并获取到对应值
        import("core.base.option")
        target:add("options", "antlr4-include-directory")
        target:add("options", "antlr4-lib-name")
        local sysincludedir = option.get("antlr4-include-directory")
        local syslink = option.get("antlr4-lib-name")

        -- 添加 antlr4 c++ 运行时头文件
        target:add("sysincludedirs", sysincludedir, { public = true })

        -- 添加 antlr4 c++ 运行时库
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

option("antlr4-include-directory")
    set_default("/usr/include/antlr4-runtime/") 

option("antlr4-lib-name")
    set_default("antlr4-runtime") 
