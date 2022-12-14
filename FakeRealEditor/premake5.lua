project "FakeRealEditor"
    kind "ConsoleApp" --项目类型
    language "c++"
    cppdialect "c++17"
    staticruntime "On"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}") --输出目录
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}") --中间临时文件

    files --该项目的文件
    {
        "Source/**.h",
        "Source/**.cpp"
    }
    includedirs --附加包含目录
    {
        "%{wks.location}/FakeRealRuntime/Source",
        "%{wks.location}/3rdparty/spdlog/include",
        "%{wks.location}/3rdparty/GLFW/include",
        "%{IncludeDir.VulkanSdkDir}",
        "%{wks.location}/3rdparty/glm",
    }
printf("%s", IncludeDir.VulkanSdkDir)
    links
    {
        "FakeRealRuntime",
        "vulkan-1",
    }

    libdirs
    {
        "%{LibraryDir.FakeRealRuntimeLibDir}",
        "%{LibraryDir.GLFWLibDir}",
        "%{LibraryDir.VulkanSdkLibDir}",
    }

    filter "system:windows" --平台配置
        systemversion "latest"
        defines --预编译宏
        {
        }
        postbuildcommands --编译后自定义命令
        {
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"