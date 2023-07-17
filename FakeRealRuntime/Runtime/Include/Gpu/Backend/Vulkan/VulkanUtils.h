#pragma once

#include <cstdint>
#include "Gpu/Backend/vulkan/GPUVulkan.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VkUtil_DescriptorPool
{
    struct GPUDevice_Vulkan* Device;
    VkDescriptorPool pVkDescPool;
    VkDescriptorPoolCreateFlags mFlags;
} VkUtil_DescriptorPool;

void VulkanUtil_SelectValidationLayers(struct GPUInstance_Vulkan* pInstance, const char** instanceLayers, uint32_t layersCount);
void VulkanUtil_SelectInstanceExtensions(struct GPUInstance_Vulkan* pInstance, const char** instanceExtensions, uint32_t extensionCount);
void VulkanUtil_QueryAllAdapters(struct GPUInstance_Vulkan* pInstance, const char** ppExtensions, uint32_t extensionsCount);
void VulkanUtil_SelectPhysicalDeviceExtensions(GPUAdapter_Vulkan* pAdapter, const char** ppExtensions, uint32_t extensionsCount);
void VulkanUtil_SelectQueueFamilyIndex(GPUAdapter_Vulkan* pAdapter);
void VulkanUtil_EnumFormatSupport(GPUAdapter_Vulkan* pAdapter);
void VulkanUtil_RecordAdaptorDetail(GPUAdapter_Vulkan* pAdapter);
void VulkanUtil_EnableValidationLayers(struct GPUInstance_Vulkan* pInstance, const struct VkDebugUtilsMessengerCreateInfoEXT* pMessengerCreateInfo);
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanUtil_DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                             void* pUserData);

#define GPU_VK_DESCRIPTOR_TYPE_RANGE_SIZE (VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT + 1)
static const VkDescriptorPoolSize gDescriptorPoolSizes[GPU_VK_DESCRIPTOR_TYPE_RANGE_SIZE] = {
    { VK_DESCRIPTOR_TYPE_SAMPLER, 1024 },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 8192 },
    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1024 },
    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1024 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8192 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1 },
    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1 },
};
#ifdef __cplusplus
}
#endif