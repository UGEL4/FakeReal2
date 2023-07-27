local targetdir = "$(projectdir)/bin/" .. output_dir .. "/MateParser"
local objdir    = "$(projectdir)/bin-init/" .. output_dir .. "/MateParser"
target("MateParser")
    set_kind("binary")
    set_languages("cxx20")

    set_objectdir(objdir)
    set_targetdir(targetdir)

    add_files("$(projectdir)/MateParser/Source/main.cpp")