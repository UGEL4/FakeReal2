
local targetdir = "$(projectdir)/bin/" .. output_dir .. "/RenderGraph"
local objdir    = "$(projectdir)/bin-init/" .. output_dir .. "/RenderGraph"

local dependency_graph_include = "$(projectdir)/FakeRealRuntime/Runtime/DependencyGraph/Include"
local dependency_graph_lib = "$(projectdir)/bin/" .. output_dir .. "/DependencyGraph"
local runtime_include = "$(projectdir)/FakeRealRuntime/Runtime/Include"
local runtime_lib     = "$(projectdir)/bin/" .. output_dir .. "/Runtime"
target("RenderGraph")
    set_kind("static")
    set_group("RenderGraph")

    add_deps("DependencyGraph", "Runtime")

    add_cxflags(project_cxflags)
    add_defines("UNICODE")

    set_objectdir(objdir)
    set_targetdir(targetdir)

    add_linkdirs(dependency_graph_lib, runtime_lib)
    add_links("DependencyGraph", "Runtime")

    add_headerfiles("$(projectdir)/FakeRealRuntime/RenderGraph/Include/**/*.h")
    add_includedirs("$(projectdir)/FakeRealRuntime/RenderGraph/Include", dependency_graph_include, runtime_include, {public = true})
    add_files("$(projectdir)/FakeRealRuntime/RenderGraph/Src/**/*.cpp")