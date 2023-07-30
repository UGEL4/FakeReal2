target("assimp_model_tool")
    set_kind("binary")
    set_languages("cxx20")

    add_deps("OldFakeRealRuntime")

    --[[ add_linkdirs("$(projectdir)/bin/" .. output_dir .. "/FakeRealRuntime")
    add_links("OldFakeRealRuntime") ]]

    --[[ set_objectdir(objdir)
    set_targetdir(targetdir) ]]

    --if(has_config("is_msvc")) then
        add_linkdirs("$(projectdir)/3rdparty/Assimp/lib/debug-vc14.3")
        add_links("assimp")
    --end

    add_includedirs("$(projectdir)/3rdparty/Assimp/include", "$(projectdir)/AssimpModelTool/include")

    add_files("$(projectdir)/AssimpModelTool/src/**.cpp")