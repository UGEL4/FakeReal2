add_rules("mode.debug", "mode.release")

output_dir = "$(mode)/$(plat)/$(arch)"

project_ldflags = {}
project_cxflags = {}
project_mxflags = {}

-- uses utf-8 charset at runtime
if is_host("windows") then
    table.insert(project_cxflags, "/execution-charset:utf-8")
    table.insert(project_cxflags, "/source-charset:utf-8")
end

if(has_config("is_clang")) then
    table.insert(project_cxflags, "-Wno-unused-command-line-argument")
    table.insert(project_cxflags, "-Wno-format")
    -- table.insert(project_cxflags, "-Wno-deprecated-builtins")
    table.insert(project_cxflags, "-Wno-switch")
    table.insert(project_cxflags, "-Wno-misleading-indentation")
    table.insert(project_cxflags, "-Wno-unknown-pragmas")
    table.insert(project_cxflags, "-Wno-unused-function")
    table.insert(project_cxflags, "-Wno-ignored-attributes")
    table.insert(project_cxflags, "-Wno-deprecated-declarations")
    table.insert(project_cxflags, "-Wno-nullability-completeness")
    table.insert(project_cxflags, "-Wno-tautological-undefined-compare")
    table.insert(project_cxflags, "-Werror=return-type")
    -- enable time trace with clang compiler
    table.insert(project_cxflags, "-ftime-trace")
    if(has_config("is_msvc")) then
        table.insert(project_cxflags, "-Wno-microsoft-cast")
        table.insert(project_cxflags, "-Wno-microsoft-enum-forward-reference")
        if (is_mode("asan")) then
            table.insert(project_ldflags, "-fsanitize=address")
        end
    end
end

if(has_config("is_msvc")) then
    table.insert(project_ldflags, "/IGNORE:4217,4286")
    table.insert(project_cxflags, "/Zc:__cplusplus")
    table.insert(project_cxflags, "/FC")
    table.insert(project_cxflags, "/GR-")
    table.insert(project_cxflags, "/wd4251")
    if (is_mode("asan")) then
        table.insert(project_ldflags, "/fsanitize=address")
    end
end

if (is_os("windows")) then 
    add_defines("_GAMING_DESKTOP")
    add_defines("_WINDOWS")
    add_defines("UNICODE")
    add_defines("_UNICODE")
    add_defines("NOMINMAX")
    add_defines("_CRT_SECURE_NO_WARNINGS")
   --[[  if (is_mode("release")) then
        set_runtimes("MD")
    elseif (is_mode("asan")) then
        table.insert(defs_list, "_DISABLE_VECTOR_ANNOTATION")
    else
        set_runtimes("MDd")
    end ]]
else

end

includes("xmake/3rdparty/glfw.lua")
includes("FakeRealRuntime/xmake.lua")
includes("FakeRealEditor/xmake.lua")