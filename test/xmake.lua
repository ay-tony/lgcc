for _, file in ipairs(os.files("*.c")) do 
    local basename = path.basename(file)
    target(basename)
        set_kind("phony")
        set_default(false)
        add_deps("lgcc")
        add_tests("test", { runargs = path.absolute(file) })

        on_test(function (target, opt)
            -- 获取系统和项目中的编译工具
            import("lib.detect.find_tool")
            local lgcc = path.absolute(path.join(target:targetdir(), "lgcc"))
            local clang = find_tool("clang").program

            -- 在 $(buildir)/.test 下面编译测试点
            import("core.project.config")
            local filename = path.filename(opt.runargs)
            local testdir = path.join(config.buildir(), ".test")
            os.mkdir(testdir)

            local lgcc_mid = path.absolute(path.join(testdir, filename .. ".ll"))
            local lgcc_out = path.absolute(path.join(testdir, filename .. "-lgcc"))
            local clang_out = path.absolute(path.join(testdir, filename .. "-clang"))

            -- 测试能否通过编译
            local lgcc_failed = false
            local clang_failed = false
            try {
                function ()
                    os.runv(lgcc, { opt.runargs, "-o", lgcc_mid })
                    os.runv(clang, { lgcc_mid, "-o", lgcc_out })
                end,
                catch { function () lgcc_failed = true end }
            }
            try {
                function ()
                    os.runv(clang, { opt.runargs, "-o", clang_out })
                end,
                catch { function () clang_failed = true end }
            } 

            if (lgcc_failed and clang_failed) then
                return true
            elseif (lgcc_failed ~= clang_failed) then
                return false
            end

            -- 测试编译后的程序能否正常运行
            local outf = os.tmpfile()
            local errf = os.tmpfile()

            local lgcc_return
            try {
                function()
                    lgcc_return, _ = os.execv(lgcc_out, {}, { stdout = outf, stderr = errf })
                end
            }
            local lgcc_stdout = io.readfile(outf)
            local lgcc_stderr = io.readfile(errf)

            local clang_return
            try {
                function()
                    clang_return, _ = os.execv(clang_out, {}, { stdout = outf, stderr = errf })
                end
            }
            local clang_stdout = io.readfile(outf)
            local clang_stderr = io.readfile(errf)

            os.rm(outf)
            os.rm(errf)

            return (lgcc_stdout == clang_stdout) and
                   (lgcc_stderr == clang_stderr) and
                   (lgcc_return == clang_return)
        end)

        on_clean(function (target) 
            import("core.project.config")
            local testdir = path.join(config.buildir(), ".test")
            os.rm(path.join(testdir, "**"))
        end)
end
