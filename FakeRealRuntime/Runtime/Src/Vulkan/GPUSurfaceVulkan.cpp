#include "Gpu/Backend/Vulkan/GPUSurfaceVulkan.h"

const GPUSurfacesProcTable vkSurfacesTable = {
    .FreeSurface = &FreeSruface_Vulkan,
#if defined(_WIN64)
    .CreateSurfaceFromHWND = &CreateSurfaceFromHWND_Vulkan
#endif
};
const GPUSurfacesProcTable* GPUVulkanSurfacesTable() { return &vkSurfacesTable; }

#if defined(_WIN64)
    #include <Windows.h>

GPUSurfaceID CreateSurfaceFromHWND_Vulkan(GPUInstanceID pInstance, HWND window)
{
    assert(window);

    GPUInstance_Vulkan* pVkInstance = (GPUInstance_Vulkan*)pInstance;

    GPUSurfaceID pSurface = VK_NULL_HANDLE;

    VkWin32SurfaceCreateInfoKHR info = {};
    info.sType                       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    info.hwnd                        = window;
    info.hinstance                   = GetModuleHandle(VK_NULL_HANDLE);

    VkResult result = vkCreateWin32SurfaceKHR(pVkInstance->pInstance, &info, GLOBAL_VkAllocationCallbacks, (VkSurfaceKHR*)&pSurface);
    if (result != VK_SUCCESS)
    {
        assert(0 && "CreateWin32Surface failed!");
        return VK_NULL_HANDLE;
    }

    return pSurface;
}
#endif

void FreeSruface_Vulkan(GPUInstanceID pInstance, GPUSurfaceID pSurface)
{
    GPUInstance_Vulkan* pVkInstance = (GPUInstance_Vulkan*)pInstance;
    VkSurfaceKHR pVkSurface         = (VkSurfaceKHR)pSurface;
    vkDestroySurfaceKHR(pVkInstance->pInstance, pVkSurface, GLOBAL_VkAllocationCallbacks);
}