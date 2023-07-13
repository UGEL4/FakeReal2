add_rules("mode.debug", "mode.release")

output_dir = "$(mode)/$(plat)/$(arch)"

includes("xmake/3rdparty/glfw.lua")