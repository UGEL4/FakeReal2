#include "GPU/GpuConfig.h"

#ifdef GPU_USE_VULKAN
    #include "Vulkan/GPUVulkan.cpp"
    
    #define VMA_IMPLEMENTATION
    #define VMA_STATIC_VULKAN_FUNCTIONS 0
    #define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
    #include "Gpu/Backend/Vulkan/vma/vk_mem_alloc.h"
#endif // GPU_USE_VULKAN
