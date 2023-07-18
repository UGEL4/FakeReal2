#include "Gpu/GpuConfig.h"

#ifdef GPU_USE_VULKAN
    #include "Gpu/Backend/Vulkan/GPUVulkan.h"
    #include "Vulkan/volk.c"
#endif // GPU_USE_VULKAN
