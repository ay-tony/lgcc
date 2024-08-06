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
            local llc = find_tool("llc").program

            -- 在 $(buildir)/.test 下面编译测试点
            import("core.project.config")
            local filename = path.filename(opt.runargs)
            local testdir = path.join(config.buildir(), ".test")
            os.mkdir(testdir)

            local lgcc_mid = path.absolute(path.join(testdir, filename .. ".ll"))
            local lgcc_out = path.absolute(path.join(testdir, filename .. "-lgcc"))
            local clang_out = path.absolute(path.join(testdir, filename .. "-clang"))

            -- 测试能否通过编译
            local lcc_failed = false
            local clang_failed = false
            try {
                function ()
                    os.execv(lgcc, { opt.runargs, "-o", lgcc_mid })
                    os.execv(llc, { lgcc_mid, "-o", lgcc_out })
                end,
                catch { function () lcc_failed = true end }
            }
            try {
                function ()
                    os.runv(clang, { opt.runargs, "-o", clang_out })
                end,
                catch { function () clang_failed = true end }
            } 

            if (lcc_failed and clang_failed) then
                return true
            elseif (lcc_failed ~= clang_failed) then
                return false
            end

            -- 测试编译后的程序能否正常运行
            local lcc_output
            local clang_output
            try {
                function ()
                    lcc_output, _ = os.iorunv(lcc_out)
                end,
                catch { function () lcc_failed = true end }
            } 
            try {
                function ()
                    clang_output, _ = os.iorunv(clang_out)
                end,
                catch { function () clang_failed = true end }
            } 

            if (lcc_failed and clang_failed) then
                return true
            elseif (lcc_failed ~= clang_failed) then
                return false
            end

            return lcc_output == clang_output
        end)

        on_clean(function (target) 
            import("core.project.config")
            local testdir = path.join(config.buildir(), ".test")
            os.rm(path.join(testdir, "**"))
        end)
end