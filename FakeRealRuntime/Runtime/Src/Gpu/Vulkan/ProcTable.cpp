#include "Gpu/Backend/Vulkan/GPUVulkan.h"

const GPUProcTable vkTable = {
    .CreateInstance = &GPUCreateInstance_Vulkan,
    .FreeInstance   = &GPUFreeInstance_Vllkan,
    .EnumerateAdapters = &GPUEnumerateAdapters_Vulkan,
    .CreateDevice = &GPUCreateDevice_Vulkan,
    .FreeDevice = &GPUFreeDevice_Vulkan,
    .GetQueue = &GPUGetQueue_Vulkan,
    .FreeQueue = &GPUFreeQueue_Vulkan,
    .SubmitQueue = &GPUSubmitQueue_Vulkan,
    .WaitQueueIdle = &GPUWaitQueueIdle_Vulkan,
    .QueuePresent = &GPUQueuePresent_Vulkan,
    .CreateSwapchain = &GPUCreateSwapchain_Vulkan,
    .FreeSwapchain = &GPUFreeSwapchain_Vulkan,
    .AcquireNextImage = &GPUAcquireNextImage_Vulkan,
    .CreateTextureView = &GPUCreateTextureView_Vulkan,
    .FreeTextureView = &GPUFreeTextureView_Vulkan,
    .CreateTexture = &GPUCreateTexture_Vulkan,
    .FreeTexture = &GPUFreeTexture_Vulkan,
    .CreateShaderLibrary = &GPUCreateShaderLibrary_Vulkan,
    .FreeShaderLibrary = &GPUFreeShaderLibrary_Vulkan,
    .CreateRootSignature = &GPUCreateRootSignature_Vulkan,
    .FreeRootSignature = &GPUFreeRootSignature_Vulkan,
    .CreateRenderPipeline = &GPUCreateRenderPipeline_Vulkan,
    .FreeRenderPipeline = &GPUFreeRenderPipeline_Vulkan
};

const GPUProcTable* GPUVulkanProcTable()
{
    return &vkTable;
}