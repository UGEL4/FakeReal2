#include "Gpu/GpuConfig.h"

#ifdef GPU_USE_VULKAN
    #include "Vulkan/GPUSurfaceVulkan.cpp"
    #include "Vulkan/VulkanUtils.cpp"
    #include "Vulkan/GPUVulkan.cpp"
    #include "Vulkan/GPUVulkanResources.cpp"
    #include "Vulkan/vma.cpp"
#endif // GPU_USE_VULKAN
