#include "Gpu/Backend/Vulkan/GPUVulkan.h"

const GPUProcTable vkTable = {
    .CreateInstance = &GPUCreateInstance_Vulkan,
    .FreeInstance   = &GPUFreeInstance_Vllkan,
    .EnumerateAdapters = &GPUEnumerateAdapters_Vulkan,
    .CreateDevice = &GPUCreateDevice_Vulkan,
    .FreeDevice = &GPUFreeDevice_Vulkan
};

const GPUProcTable* GPUVulkanProcTable()
{
    return &vkTable;
}