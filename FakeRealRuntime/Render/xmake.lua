local runtime_include     = "$(projectdir)/FakeRealRuntime/Runtime/Include"
local render_indluces_dir = {"$(projectdir)/FakeRealRuntime/Render/Include", runtime_include}

local targetdir = "$(projectdir)/bin/" .. output_dir .. "/Render"
local objdir    = "$(projectdir)/bin-init/" .. output_dir .. "/Render"
local runtime_lib_dir = "$(projectdir)/bin/" .. output_dir .. "/Runtime"
target("Render")
    set_kind("static")
    set_languages("cxx20")
    set_group("render")
    add_cxflags(project_cxflags)
    add_defines("UNICODE")

    add_deps("Runtime")

    add_linkdirs(runtime_lib_dir)
    add_links("Runtime")

    set_objectdir(objdir)
    set_targetdir(targetdir)

    -- set_pcheader("FRPch.h")

    add_includedirs(render_indluces_dir, {public = true})
    
    add_headerfiles("$(projectdir)/FakeRealRuntime/Render/**.h")
    add_files("$(projectdir)/FakeRealRuntime/Render/**.cpp")