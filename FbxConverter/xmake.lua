target("FbxConverter")
    set_kind("binary")
    set_languages("cxx20")

    add_defines("UNICODE", "FBXSDK_SHARED")

    add_linkdirs("$(projectdir)/3rdparty/FbxSDK/lib/debug")
    add_links("libfbxsdk")

    add_includedirs("$(projectdir)/3rdparty/FbxSDK/include", "$(projectdir)/FbxConverter/Source")

    add_files("$(projectdir)/FbxConverter/Source/**.cpp")