#pragma once
#include "Gpu/Backend/Vulkan/GPUVulkan.h"

static const char* s_validationLayerName = "VK_LAYER_KHRONOS_validation";

static const char* s_intanceWantedExtensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif

#if VK_KHR_device_group_creation
    VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME,
#endif
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
};

static const char* s_deviceWantedExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    /************************************************************************/
    // Descriptor Update Template Extension for efficient descriptor set updates
    /************************************************************************/
    VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME
// VK_KHR_MAINTENANCE1_EXTENSION_NAME // Vulkan NDC fixed, vk 1.0
#if VK_KHR_device_group
    ,
    VK_KHR_DEVICE_GROUP_EXTENSION_NAME
#endif
};