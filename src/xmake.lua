target("lgcc")
    add_rules("antlr4", {
        sysincludedir = "/usr/include/antlr4-runtime",
        syslink = "antlr4-runtime"
    })

    add_files("c.g4")
    add_files("main.cpp")