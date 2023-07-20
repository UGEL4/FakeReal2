local runtime_include     = "$(projectdir)/FakeRealRuntime/Runtime/Include"
local glfw_include        = "$(projectdir)/3rdparty/GLFW/include"
local render_indluces_dir = {"$(projectdir)/FakeRealRuntime/Render/Include", runtime_include, glfw_include}

local targetdir = "$(projectdir)/bin/" .. output_dir .. "/Render"
local objdir    = "$(projectdir)/bin-init/" .. output_dir .. "/Render"
local runtime_lib_dir = "$(projectdir)/bin/" .. output_dir .. "/Runtime"
local glfw_lib        = "$(projectdir)/bin/" .. output_dir .. "/GLFW"
target("Render")
    set_kind("static")
    set_languages("cxx20")
    set_group("render")
    add_cxflags(project_cxflags)
    add_defines("UNICODE")

    add_deps("Runtime")

    add_linkdirs(runtime_lib_dir, glfw_lib)
    add_links("Runtime", "GLFW")

    set_objectdir(objdir)
    set_targetdir(targetdir)

    -- set_pcheader("FRPch.h")

    add_includedirs(render_indluces_dir, {public = true})
    
    add_headerfiles("$(projectdir)/FakeRealRuntime/Render/**.h")
    add_files("$(projectdir)/FakeRealRuntime/Render/**.cpp")