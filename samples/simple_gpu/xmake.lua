
target("test_simple_gpu")
    set_kind("binary")
    add_deps("RenderGraph")
    set_group("test/simple_gpu")
    add_defines("UNICODE")

    if (is_os("windows")) then 
        add_syslinks("advapi32", "user32", "shell32", "Ole32", {public = true})
    end

    add_linkdirs("$(projectdir)/bin/" .. output_dir .. "/RenderGraph")
    add_links("RenderGraph")

    add_files("**.cpp")