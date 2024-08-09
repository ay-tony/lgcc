target("lgcc")
    add_deps("antlr")
    add_files("main.cpp")

target("antlr")
    set_kind("object")
    add_deps("antlr-generate")

target("antlr-generate")
    set_kind("object")

    add_rules("antlr4")

    add_files("lgcc.g4")
    set_policy("build.fence", true)

