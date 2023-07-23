local render_include = "$(projectdir)/FakeRealRuntime/Render/Include"
local game_includes  = {render_include}

local targetdir = "$(projectdir)/bin/" .. output_dir .. "/sample_render"
local objdir    = "$(projectdir)/bin-init/" .. output_dir .. "/sample_render"

local render_lib = "$(projectdir)/bin/" .. output_dir .. "/Render"
target("sample_render")
    set_kind("binary")
    set_languages("cxx20")
    set_group("sample")
    add_cxxflags(project_cxflags)
    add_defines("UNICODE")

    add_deps("Render")

    add_linkdirs(render_lib)
    add_links("Render")
    
    set_objectdir(objdir)
    set_targetdir(targetdir)
    
    add_includedirs(game_includes)
    add_headerfiles("$(projectdir)/samples/app/sample_render/**.h")
    add_files("$(projectdir)/samples/app/sample_render/**.cpp")