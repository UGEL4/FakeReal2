project "FakeRealRuntime"
    kind "StaticLib" --项目类型
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
        "Source",
        "%{wks.location}/3rdparty/spdlog/include",
    }

    filter "system:windows" --平台配置
        systemversion "latest"

        defines --预编译宏
        {

        }

        postbuildcommands --编译后自定义命令
        {
            ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/FakeRealEditor") --拷贝引擎lib库到FakeRealEditor.exe的同一目录下去
        }
    
    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
    
    filter "configurations:Release"
        runtime "Release"
        optimize "on"