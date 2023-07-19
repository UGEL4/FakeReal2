local vulkan_sdk_dir         = "$(projectdir)/3rdparty/VulkanSDK"
-- local vulkan_libs_dir        = vulkan_sdk_dir .. "/lib/Win32"
local vulkan_include_dir     = vulkan_sdk_dir .. "/include"
local spdlog_include_dir     = "$(projectdir)/3rdparty/spdlog/include"
local glm_include_dir        = "$(projectdir)/3rdparty/glm"
local rapid_json_include_dir = "$(projectdir)/3rdparty/rapidjson/include"
-- local glfw_include_dir       = "$(projectdir)/3rdparty/GLFW/include"
local runtime_include        = "$(projectdir)/FakeRealRuntime/Runtime/Include"
local old_runtime_include    = "$(projectdir)/FakeRealRuntime/Source"
local runtime_indluces_dir   = {spdlog_include_dir, vulkan_include_dir, glm_include_dir, rapid_json_include_dir, runtime_include, old_runtime_include}

local targetdir = "$(projectdir)/bin/" .. output_dir .. "/Runtime"
local objdir    = "$(projectdir)/bin-init/" .. output_dir .. "/Runtime"
-- local glfw_lib = "$(projectdir)/bin/" .. output_dir .. "/GLFW"
target("Runtime")
    set_kind("static")
    set_languages("cxx20")
    set_group("runtime")
    add_cxflags(project_cxflags)
    add_defines("UNICODE")

    set_objectdir(objdir)
    set_targetdir(targetdir)

    -- set_pcheader("FRPch.h")

    add_includedirs(runtime_indluces_dir, {public = true})
    
    add_headerfiles("$(projectdir)/FakeRealRuntime/Runtime/**.h")
    add_files("$(projectdir)/FakeRealRuntime/**/build.**.cpp")