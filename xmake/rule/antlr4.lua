rule("antlr4")
    add_deps("c++")
    set_extensions(".g4")

    before_buildcmd_file(function (target, batchcmds, sourcefile, opt)
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
        batchcmds:mkdir(gendir)
        target:add("includedirs", gendir, { public = true })

        -- 开始生成 .cpp 和 .h 文件
        batchcmds:show_progress(opt.progress, "${color.build.object}compiling.antlr4 %s", sourcefile)
        os.cd(path.absolute(path.directory(sourcefile)))
        batchcmds:vrunv(antlr4.program, {
            "-Dlanguage=Cpp",
            "-o",
            gendir,
            path.absolute(path.filename(sourcefile))})
        os.cd("-")

        -- 因为异步编译，提前准备生成的 .cpp 文件列表
        local genfiles = {
            path.join(gendir, path.basename(sourcefile) .. "Lexer.cpp"),
            path.join(gendir, path.basename(sourcefile) .. "Parser.cpp"),
            path.join(gendir, path.basename(sourcefile) .. "Listener.cpp"),
            path.join(gendir, path.basename(sourcefile) .. "BaseListener.cpp")
        }

        -- 编译生成的 .cpp 文件
        for _, genfile in ipairs(genfiles) do
            local objectfile = target:objectfile(genfile)   -- 对应的 .o 文件
            batchcmds:show_progress(opt.progress, "${color.build.object}compiling.antlr4 %s", genfile)
            batchcmds:compile(genfile, objectfile)          -- 编译
            table.insert(target:objectfiles(), objectfile)  -- 把 .o 文件加入链接表
            batchcmds:set_depmtime(os.mtime(objectfile))    -- 设置文件修改时间，判定依赖要用
        end

        batchcmds:add_depfiles(sourcefile)  -- 添加对 .g4 源文件 的依赖
        batchcmds:set_depcache(target:dependfile(objectfile))
    end)