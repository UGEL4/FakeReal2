local targetdir = "$(projectdir)/bin/" .. output_dir .. "/GLFW"
local objdir    = "$(projectdir)/bin-init/" .. output_dir .. "/GLFW"
local glfw_dir  = "$(projectdir)/3rdparty/GLFW"
target("GLFW")
    set_kind("static")
    set_languages("c17")
    set_group("GLFW")
    set_objectdir(objdir)
    set_targetdir(targetdir)
    set_plat(os.host())
    set_arch(os.arch())
    if is_os("windows") then
        add_syslinks("advapi32", "user32", "shell32", "Ole32", {public = true})
        add_defines("_GLFW_WIN32", "_CRT_SECURE_NO_WARNINGS")
        --add_cxflags(project_cxflags, {public = true, force = true})
        add_defines("UNICODE")
        add_files(
           --[[  glfw_dir .. "/src/win32_platform.h",
            glfw_dir .. "/src/win32_joystick.h",
            glfw_dir .. "/src/wgl_context.h",
            glfw_dir .. "/src/egl_context.h",
            glfw_dir .. "/src/osmesa_context.h", ]]

            glfw_dir .. "/src/win32_init.c",
            glfw_dir .. "/src/win32_joystick.c",
            glfw_dir .. "/src/win32_monitor.c",
            glfw_dir .. "/src/win32_time.c",
            glfw_dir .. "/src/win32_thread.c",
            glfw_dir .. "/src/win32_module.c",
            glfw_dir .. "/src/win32_window.c",
            glfw_dir .. "/src/wgl_context.c",
            glfw_dir .. "/src/egl_context.c",
            glfw_dir .. "/src/osmesa_context.c"
        )
    end
    add_files(
        --[[ glfw_dir .. "/include/GLFW/glfw3.h",
        glfw_dir .. "/include/GLFW/glfw3native.h",
        glfw_dir .. "/src/internal.h",
        glfw_dir .. "/src/platform.h",
        glfw_dir .. "/src/mappings.h", ]]

        glfw_dir .. "/src/init.c",
        glfw_dir .. "/src/input.c",
        glfw_dir .. "/src/monitor.c",
        glfw_dir .. "/src/context.c",
        glfw_dir .. "/src/platform.c",
        glfw_dir .. "/src/vulkan.c",
        glfw_dir .. "/src/window.c",

       --[[  glfw_dir .. "/src/null_platform.h",
        glfw_dir .. "/src/null_joystick.h", ]]
        glfw_dir .. "/src/null_init.c",
        glfw_dir .. "/src/null_monitor.c",
        glfw_dir .. "/src/null_window.c",
        glfw_dir .. "/src/null_joystick.c"
    )