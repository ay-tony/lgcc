set_project("lgcc")
set_version("0.0.1", {build = "%Y%m%d%H%M"})
set_xmakever("2.8.5")

add_rules("mode.debug", "mode.release")

set_languages("cxxlatest")
set_warnings("everything")

includes("src", "test", "xmake")
