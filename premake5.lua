outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "./premake/premake_customization/solution_items.lua"
include "dependencies.lua"

workspace "FakeReal2" --解决方案
    architecture "x86_64"
    startproject "FakeRealEditor"

    configurations
    {
        "Debug",
        "Release"
    }

    platforms
    {
        "Win64"
    }

    solution_items
    {

    }

group "dependences"
    include "premake"
    include "3rdparty/GLFW"
group ""

include "FakeRealRuntime"
include "FakeRealEditor"
include "3rdparty/GLFW"
include "FbxConverter"
include "MateParser"

-- project "FakeRealRuntime"
--     location "FakeRealRuntime" --相对路径
--     kind "StaticLib" --项目类型
--     language "c++"
--     targetdir ("bin/" .. outputdir .. "/%{prj.name}") --输出目录
--     objdir ("bin-ini/" .. outputdir .. "/%{prj.name}") --中间临时文件

--     files --该项目的文件
--     {
--         "%{prj.name}/Source/**.h",
--         "%{prj.name}/Source/**.cpp"
--     }

--     includedirs --附加包含目录
--     {
        
--     }

--     filter "system:windows" --平台配置
--         cppdialect "c++17"
--         staticruntime "On"
--         systemversion "latest"

--         defines --预编译宏
--         {

--         }

--         postbuildcommands --编译后自定义命令
--         {
--             ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/FakeRealEditor") --拷贝引擎lib库到FakeRealEditor.exe的同一目录下去
--         }
    
--     filter "configurations:Debug"
--         runtime "Debug"
--         symbols "on"
    
--     filter "configurations:Release"
--         runtime "Release"
--         optimize "on"

-- project "FakeRealEditor"
--     location "FakeRealEditor" --相对路径
--     kind "ConsoleApp" --项目类型
--     language "c++"
--     targetdir ("bin/" .. outputdir .. "/%{prj.name}") --输出目录
--     objdir ("bin-ini/" .. outputdir .. "/%{prj.name}") --中间临时文件
--     files --该项目的文件
--     {
--         "%{prj.name}/Source/**.h",
--         "%{prj.name}/Source/**.cpp"
--     }
--     includedirs --附加包含目录
--     {
--         "FakeRealRuntime/Source",
--     }

--     links
--     {
--         "FakeRealRuntime"
--     }

--     filter "system:windows" --平台配置
--         cppdialect "c++17"
--         staticruntime "On"
--         systemversion "latest"
--         defines --预编译宏
--         {
--         }
--         postbuildcommands --编译后自定义命令
--         {
--         }

--     filter "configurations:Debug"
--         runtime "Debug"
--         symbols "on"

--     filter "configurations:Release"
--         runtime "Release"
--         optimize "on"