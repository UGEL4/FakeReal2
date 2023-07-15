local vulkan_sdk_dir         = "$(projectdir)/3rdparty/VulkanSDK"
local vulkan_include_dir     = vulkan_sdk_dir .. "/include"
local spdlog_include_dir     = "$(projectdir)/3rdparty/spdlog/include"
local glm_include_dir        = "$(projectdir)/3rdparty/glm"
local rapid_json_include_dir = "$(projectdir)/3rdparty/rapidjson/include"
local glfw_include_dir       = "$(projectdir)/3rdparty/GLFW/include"
local runtime_include        = "$(projectdir)/FakeRealRuntime/Source"
local editor_includes        = {runtime_include, vulkan_include_dir, spdlog_include_dir, glm_include_dir, rapid_json_include_dir, glfw_include_dir}

local targetdir = "$(projectdir)/bin/" .. output_dir .. "/FakeRealEditor"
local objdir    = "$(projectdir)/bin-init/" .. output_dir .. "/FakeRealEditor"

local runtime_lib = "$(projectdir)/bin/" .. output_dir .. "/FakeRealRuntime"
local glfw_lib    = "$(projectdir)/bin/" .. output_dir .. "/GLFW"
local vulkan_lib  = vulkan_sdk_dir .. "/lib/Win32"
target("FakeRealEditor")
    set_kind("binary")
    set_languages("cxx20")
    set_group("editor")
    add_cxxflags(project_cxflags)
    add_defines("UNICODE")

    add_deps("FakeRealRuntime")

    add_linkdirs(runtime_lib, vulkan_lib, glfw_lib)
    add_links("FakeRealRuntime", "GLFW", "vulkan-1")
    
    set_objectdir(objdir)
    set_targetdir(targetdir)
    
    add_includedirs(editor_includes)
    -- add_headerfiles("$(projectdir)/FakeRealEditor/Source/**.h")
    add_files("$(projectdir)/FakeRealEditor/Source/**.cpp")