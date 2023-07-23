local runtime_include      = "$(projectdir)/FakeRealRuntime/Runtime/Include"
local glfw_include         = "$(projectdir)/3rdparty/GLFW/include"
local render_graph_include = "$(projectdir)/FakeRealRuntime/RenderGraph/Include"
local render_indluces_dir  = {"$(projectdir)/FakeRealRuntime/Render/Include", runtime_include, glfw_include, render_graph_include}

local targetdir = "$(projectdir)/bin/" .. output_dir .. "/Render"
local objdir    = "$(projectdir)/bin-init/" .. output_dir .. "/Render"
local runtime_lib_dir  = "$(projectdir)/bin/" .. output_dir .. "/Runtime"
local glfw_lib         = "$(projectdir)/bin/" .. output_dir .. "/GLFW"
local render_graph_lib = "$(projectdir)/bin/" .. output_dir .. "/RenderGraph"
target("Render")
    set_kind("static")
    set_languages("cxx20")
    set_group("render")
    add_cxflags(project_cxflags)
    add_defines("UNICODE")

    --add_deps("Runtime")
    add_deps("RenderGraph")

    add_linkdirs(glfw_lib, render_graph_lib)
    add_links("GLFW", "RenderGraph")

    set_objectdir(objdir)
    set_targetdir(targetdir)

    -- set_pcheader("FRPch.h")

    add_includedirs(render_indluces_dir, {public = true})
    
    add_headerfiles("$(projectdir)/FakeRealRuntime/Render/**.h")
    add_files("$(projectdir)/FakeRealRuntime/Render/**.cpp")