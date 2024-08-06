target("lgcc")
    add_deps("antlr")
    add_files("main.cpp")

target("antlr")
    set_kind("object")
    add_rules("antlr4", {
        sysincludedir = "/usr/include/antlr4-runtime",
        syslink = "antlr4-runtime"
    })

    add_files("c.g4")
    set_policy("build.fence", true)
    