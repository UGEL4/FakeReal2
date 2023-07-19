-- local vulkan_sdk_dir         = "$(projectdir)/3rdparty/VulkanSDK"
-- local vulkan_libs_dir        = vulkan_sdk_dir .. "/lib/Win32"
-- local vulkan_include_dir     = vulkan_sdk_dir .. "/include"
-- local spdlog_include_dir     = "$(projectdir)/3rdparty/spdlog/include"
-- local glm_include_dir        = "$(projectdir)/3rdparty/glm"
-- local rapid_json_include_dir = "$(projectdir)/3rdparty/rapidjson/include"
-- local glfw_include_dir       = "$(projectdir)/3rdparty/GLFW/include"
-- local runtime_indluces_dir   = {"$(projectdir)/FakeRealRuntime/Source", spdlog_include_dir, vulkan_include_dir, glm_include_dir, rapid_json_include_dir, glfw_include_dir, 
-- "$(projectdir)/FakeRealRuntime/Runtime/Include"}

-- local targetdir = "$(projectdir)/bin/" .. output_dir .. "/FakeRealRuntime"
-- local objdir    = "$(projectdir)/bin-init/" .. output_dir .. "/FakeRealRuntime"
-- local glfw_lib = "$(projectdir)/bin/" .. output_dir .. "/GLFW"
-- target("FakeRealRuntime")
--     set_kind("static")
--     set_languages("cxx20")
--     set_group("runtime")
--     add_cxflags(project_cxflags)
--     add_defines("UNICODE")

--     add_deps("GLFW")

--     add_linkdirs(vulkan_libs_dir, glfw_lib)
--     add_links("vulkan-1", "GLFW")

--     set_objectdir(objdir)
--     set_targetdir(targetdir)

--     set_pcheader("FRPch.h")

--     add_includedirs(runtime_indluces_dir, {public = true})
    
--     add_headerfiles("$(projectdir)/FakeRealRuntime/Source/**.h", "$(projectdir)/FakeRealRuntime/Runtime/**.h")
--     add_files("$(projectdir)/FakeRealRuntime/Source/**.cpp", "$(projectdir)/FakeRealRuntime/**/build.**.cpp")

includes("Source/xmake.lua")
includes("Runtime/xmake.lua")
includes("Render/xmake.lua")