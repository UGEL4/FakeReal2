project "MateParser"
    --location "test" --相对路径
    kind "ConsoleApp" --项目类型
    language "c++"
    cppdialect "c++17"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}") --输出目录
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}") --中间临时文件

    files --该项目的文件
    {
        "Source/**.h",
        "Source/**.cpp"
    }

    includedirs --附加包含目录
    {
        "%{wks.location}/MateParser/Source",
        "%{wks.location}/MateParser/3rdparty/cppast/include",
        "%{wks.location}/MateParser/3rdparty/cppast/type_safe-src/include",
        "%{wks.location}/MateParser/3rdparty/cppast/tpl",
        "%{wks.location}/MateParser/3rdparty/mustache",
        "%{wks.location}/MateParser/3rdparty/LLVM/include",
    }

    links
    {
        "libclang",
        "cppast",
        "tiny-process-library",
    }

    libdirs
    {
        "%{wks.location}/MateParser/3rdparty/cppast/lib",
        "%{wks.location}/MateParser/3rdparty/LLVM/lib",
    }

    filter "system:windows" --平台配置
        cppdialect "c++17"
        staticruntime "off"
        systemversion "latest"

        defines --预编译宏
        {
            '_MBCS',
            'WIN32',
            '_WINDOWS',
            'CPPAST_VERSION_MINOR="1"',
            'CPPAST_VERSION_MAJOR="0"',
            'CPPAST_VERSION_STRING="0.1.0"',
            'TYPE_SAFE_ENABLE_ASSERTIONS=0',
            'TYPE_SAFE_ENABLE_PRECONDITION_CHECKS=1',
            'TYPE_SAFE_ENABLE_WRAPPER=1',
            'TYPE_SAFE_ARITHMETIC_POLICY=1',
            'CPPAST_CLANG_BINARY="%{wks.location}/MateParser/3rdparty/LLVM/bin/clang.exe"',
            'CPPAST_CLANG_VERSION_STRING="11.0.1"'
        }

        -- prebuildcommands --编译后自定义命令
        -- {
        --     --("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/FakeRealEditor") --拷贝引擎lib库到FakeRealEditor.exe的同一目录下去
        --     "%{wks.location}cppast.exe %{wks.location}%{prj.name}/src/ClassA.h"
        -- }

        buildmessage "----------build test---------"
        buildcommands
        {
            --"%{wks.location}cppast.exe %{wks.location}%{prj.name}/src/ClassA.h"
            --("{COPY} %{wks.location}%{prj.name}/src/ClassA.h %{wks.location}bin/" .. outputdir)
        }

        buildoutputs
        {
            --"%{wks.location}/bin/aaa.h"
        }
        buildinputs
        {
            --"%{wks.location}/%{prj.name}/src/**.h"
        }
    
    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        buildoptions "/MDd"
    
    filter "configurations:Release"
        runtime "Release"
        optimize "on"
        buildoptions "/MD"