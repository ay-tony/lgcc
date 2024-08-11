target("lgcc")
    add_deps("antlr")
    add_packages("argparse")
    add_packages("spdlog")
    add_files("main.cpp")

target("antlr")
    set_kind("object")
    add_deps("antlr-generate")

target("antlr-generate")
    set_kind("object")

    add_rules("antlr4", {
        sysincludedir = "$(antlr4_include_directory)",
        syslink = "$(antlr4_lib_name)"
    })
    add_options("antlr4_include_directory")
    add_options("antlr4_lib_name")

    add_files("lgcc.g4")
    set_policy("build.fence", true)


option("antlr4_include_directory")
    set_default("/usr/include/antlr4-runtime/") 

option("antlr4_lib_name")
    set_default("antlr4-runtime") 
