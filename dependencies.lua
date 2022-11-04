IncludeDir = {}

LibraryDir = {
    ["FakeRealRuntimeLibDir"] = "%{wks.location}/bin/" .. outputdir .. "/FakeRealRuntime",
    ["GLFWLibDir"] = "%{wks.location}/bin/" .. outputdir .. "/FakeRealRuntime",
    ["GLFWLibOutputDir"] = "%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/GLFW",
    ["VulkanSdkDir"] = "%{wks.location}/3rdparty/VulkanSDK/lib/Win32",
}

IncludeDir =
{
    ["VulkanSdkDir"] = "%{wks.location}/3rdparty/VulkanSDK/include",
}
