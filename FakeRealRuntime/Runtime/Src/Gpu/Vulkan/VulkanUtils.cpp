#include "Gpu/Backend/Vulkan/VulkanUtils.h"
#include <new>
#include <memory>
#include "Utils/Hash/hash.h"

void VulkanUtil_SelectValidationLayers(GPUInstance_Vulkan* pInstance, const char** instanceLayers, uint32_t layersCount)
{
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, VK_NULL_HANDLE);

    if (count > 0)
    {
        pInstance->pLayerProperties = (VkLayerProperties*)calloc(layersCount, sizeof(VkLayerProperties));
        pInstance->pLayerNames      = (const char**)calloc(layersCount, sizeof(const char*));

        // std::vector<VkLayerProperties> props(count);
        DECLARE_ZERO_VAL(VkLayerProperties, layerProps, count);
        vkEnumerateInstanceLayerProperties(&count, layerProps);
        uint32_t filledCount = 0;
        for (uint32_t i = 0; i < layersCount; i++)
        {
            for (uint32_t j = 0; j < count; j++)
            {
                if (strcmp(instanceLayers[i], layerProps[j].layerName) == 0)
                {
                    pInstance->pLayerProperties[filledCount] = layerProps[j];
                    pInstance->pLayerNames[filledCount]      = pInstance->pLayerProperties[filledCount].layerName;
                    filledCount++;
                    break;
                }
            }
        }
        pInstance->layersCount = filledCount;
    }
}

void VulkanUtil_SelectInstanceExtensions(struct GPUInstance_Vulkan* pInstance, const char** instanceExtensions, uint32_t extensionCount)
{
    const char* layerName = VK_NULL_HANDLE;
    uint32_t count        = 0;
    vkEnumerateInstanceExtensionProperties(layerName, &count, VK_NULL_HANDLE);
    if (count > 0)
    {
        pInstance->pExtensonProperties = (VkExtensionProperties*)calloc(extensionCount, sizeof(VkExtensionProperties));
        pInstance->pExtensionNames     = (const char**)calloc(extensionCount, sizeof(const char*));

        DECLARE_ZERO_VAL(VkExtensionProperties, extensions, count);
        vkEnumerateInstanceExtensionProperties(layerName, &count, extensions);
        uint32_t filledCount = 0;
        for (uint32_t i = 0; i < extensionCount; i++)
        {
            for (uint32_t j = 0; j < count; j++)
            {
                if (strcmp(instanceExtensions[i], extensions[j].extensionName) == 0)
                {
                    pInstance->pExtensonProperties[filledCount] = extensions[j];
                    pInstance->pExtensionNames[filledCount]     = pInstance->pExtensonProperties[filledCount].extensionName;
                    filledCount++;
                    break;
                }
            }
        }
        pInstance->extensionCount = filledCount;
    }
}

void VulkanUtil_QueryAllAdapters(struct GPUInstance_Vulkan* pInstance, const char** ppExtensions, uint32_t extensionsCount)
{
    // uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(pInstance->pInstance, &pInstance->adapterCount, VK_NULL_HANDLE);
    if (pInstance->adapterCount > 0)
    {
        pInstance->pAdapters = (GPUAdapter_Vulkan*)calloc(pInstance->adapterCount, sizeof(GPUAdapter_Vulkan));
        DECLARE_ZERO_VAL(VkPhysicalDevice, deviceArray, pInstance->adapterCount);
        vkEnumeratePhysicalDevices(pInstance->pInstance, &pInstance->adapterCount, deviceArray);
        for (uint32_t i = 0; i < pInstance->adapterCount; i++)
        {
            GPUAdapter_Vulkan* pVkAdapter = &pInstance->pAdapters[i];
            pVkAdapter->pPhysicalDevice   = deviceArray[i];
            for (uint32_t q = 0; q < GPU_QUEUE_TYPE_COUNT; q++)
            {
                pVkAdapter->queueFamilyIndices[q] = -1;
            }

            // properties
            pVkAdapter->physicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            {
                void** ppNext                        = &pVkAdapter->physicalDeviceProperties.pNext;
                pVkAdapter->subgroupProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
                pVkAdapter->subgroupProperties.pNext = VK_NULL_HANDLE;
                *ppNext                              = &pVkAdapter->subgroupProperties;
                ppNext                               = &pVkAdapter->subgroupProperties.pNext;
            }
            assert(vkGetPhysicalDeviceProperties2KHR && "load vkGetPhysicalDeviceProperties2KHR failed!");
            vkGetPhysicalDeviceProperties2KHR(pVkAdapter->pPhysicalDevice, &pVkAdapter->physicalDeviceProperties);

            // feature
            pVkAdapter->physicalDeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            {
                void** ppNext = &pVkAdapter->physicalDeviceFeatures.pNext;
                *ppNext       = VK_NULL_HANDLE;
            }
            vkGetPhysicalDeviceFeatures2(pVkAdapter->pPhysicalDevice, &pVkAdapter->physicalDeviceFeatures);

            // extensions
            VulkanUtil_SelectPhysicalDeviceExtensions(pVkAdapter, ppExtensions, extensionsCount);
            // queue family index
            VulkanUtil_SelectQueueFamilyIndex(pVkAdapter);
            // format
            VulkanUtil_EnumFormatSupport(pVkAdapter);
            //TODO
            VulkanUtil_RecordAdaptorDetail(pVkAdapter);
        }
    }
    else
    {
        assert(0 && "Vulkan no gpu can use!");
    }
}

void VulkanUtil_SelectPhysicalDeviceExtensions(GPUAdapter_Vulkan* pAdapter, const char** ppExtensions, uint32_t extensionsCount)
{
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(pAdapter->pPhysicalDevice, VK_NULL_HANDLE, &count, VK_NULL_HANDLE);
    if (count > 0)
    {
        pAdapter->pExtensionProps  = (VkExtensionProperties*)calloc(extensionsCount, sizeof(VkExtensionProperties));
        pAdapter->ppExtensionsName = (const char**)calloc(extensionsCount, sizeof(const char*));
        DECLARE_ZERO_VAL(VkExtensionProperties, props, count);
        vkEnumerateDeviceExtensionProperties(pAdapter->pPhysicalDevice, VK_NULL_HANDLE, &count, props);
        uint32_t filledCount = 0;
        for (uint32_t i = 0; i < extensionsCount; i++)
        {
            for (uint32_t j = 0; j < count; j++)
            {
                if (strcmp(ppExtensions[i], props[j].extensionName) == 0)
                {
                    pAdapter->pExtensionProps[filledCount]  = props[j];
                    pAdapter->ppExtensionsName[filledCount] = pAdapter->pExtensionProps[filledCount].extensionName;
                    filledCount++;
                    break;
                }
            }
        }
        pAdapter->extensionsCount = filledCount;
    }
}

void VulkanUtil_SelectQueueFamilyIndex(GPUAdapter_Vulkan* pAdapter)
{
    vkGetPhysicalDeviceQueueFamilyProperties(pAdapter->pPhysicalDevice, &pAdapter->queueFamiliesCount, VK_NULL_HANDLE);
    if (pAdapter->queueFamiliesCount > 0)
    {
        pAdapter->pQueueFamilyProperties = (VkQueueFamilyProperties*)calloc(pAdapter->queueFamiliesCount, sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(pAdapter->pPhysicalDevice, &pAdapter->queueFamiliesCount, pAdapter->pQueueFamilyProperties);
        for (uint32_t i = 0; i < pAdapter->queueFamiliesCount; i++)
        {
            if (pAdapter->queueFamilyIndices[GPU_QUEUE_TYPE_GRAPHICS] == -1 &&
                pAdapter->pQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                pAdapter->queueFamilyIndices[GPU_QUEUE_TYPE_GRAPHICS] = i;
            }
            else if (pAdapter->queueFamilyIndices[GPU_QUEUE_TYPE_COMPUTE] == -1 &&
                     pAdapter->pQueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                pAdapter->queueFamilyIndices[GPU_QUEUE_TYPE_COMPUTE] = i;
            }
            else if (pAdapter->queueFamilyIndices[GPU_QUEUE_TYPE_TRANSFER] == -1 &&
                     pAdapter->pQueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                pAdapter->queueFamilyIndices[GPU_QUEUE_TYPE_TRANSFER] = i;
            }
        }
    }
    else
    {
        assert(0);
    }
}

void VulkanUtil_EnumFormatSupport(GPUAdapter_Vulkan* pAdapter)
{
    GPUAdapterDetail* pAdapterDetail = &pAdapter->adapterDetail;
    for (uint32_t i = 0; i < EGPUFormat::GPU_FORMAT_COUNT; i++)
    {
        VkFormatProperties formatSupport;
        pAdapterDetail->format_supports[i].shader_read         = 0;
        pAdapterDetail->format_supports[i].shader_write        = 0;
        pAdapterDetail->format_supports[i].render_target_write = 0;
        VkFormat fmt = VulkanUtil_GPUFormatToVulkanFormat((EGPUFormat)i);
        if (fmt == VK_FORMAT_UNDEFINED)
        {
            continue;
        }
        vkGetPhysicalDeviceFormatProperties(pAdapter->pPhysicalDevice, fmt, &formatSupport);
        pAdapterDetail->format_supports[i].shader_read = (formatSupport.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) != 0;
        pAdapterDetail->format_supports[i].shader_write = (formatSupport.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) != 0;
        pAdapterDetail->format_supports[i].render_target_write = (formatSupport.optimalTilingFeatures & (VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) != 0;
    }
}

void VulkanUtil_RecordAdaptorDetail(GPUAdapter_Vulkan* pAdapter)
{
    //TODO
    GPUAdapterDetail* detail          = &pAdapter->adapterDetail;
    VkPhysicalDeviceProperties* props = &pAdapter->physicalDeviceProperties.properties;
    detail->minStorageBufferAligment  = props->limits.minStorageBufferOffsetAlignment;
    detail->maxStorageBufferRange     = props->limits.maxStorageBufferRange;
}

void VulkanUtil_EnableValidationLayers(struct GPUInstance_Vulkan* pInstance, const struct VkDebugUtilsMessengerCreateInfoEXT* pMessengerCreateInfo)
{
    if (pInstance->debugUtils)
    {
        VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};
        messengerInfo.sType       = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messengerInfo.pfnUserCallback = VulkanUtil_DebugUtilsCallback;

        const VkDebugUtilsMessengerCreateInfoEXT* ptr = (pMessengerCreateInfo != VK_NULL_HANDLE) ? pMessengerCreateInfo : &messengerInfo;
        assert(vkCreateDebugUtilsMessengerEXT && "load vkCreateDebugUtilsMessengerEXT failed!");
        VkResult result = vkCreateDebugUtilsMessengerEXT(pInstance->pInstance, ptr, GLOBAL_VkAllocationCallbacks, &pInstance->pDebugUtils);
        if (result != VK_SUCCESS)
        {
            assert(0);
        }
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanUtil_DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                             void* pUserData)
{
    using namespace FakeReal;
    switch (messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            GPU_LOG_INFO("Vulkan validation verbose layer:{}\n", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            GPU_LOG_INFO("Vulkan validation info layer:{}\n", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            GPU_LOG_WARNING("Vulkan validation warning layer:{}\n", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            GPU_LOG_ERROR("Vulkan validation error layer:{}\n", pCallbackData->pMessage);
            break;
        default:
            return VK_TRUE;
    }
    return VK_FALSE;
}

VkFormat VulkanUtil_GPUFormatToVulkanFormat(EGPUFormat format)
{
    switch (format)
    {
        case GPU_FORMAT_B8G8R8A8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case GPU_FORMAT_B8G8R8A8_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case GPU_FORMAT_R8G8B8A8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case GPU_FORMAT_R8G8B8A8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case GPU_FORMAT_R16_UINT:
            return VK_FORMAT_R16_UINT;
        case GPU_FORMAT_R16G16B16_UNORM:
            return VK_FORMAT_R16G16B16_UNORM;
        case GPU_FORMAT_R16G16B16A16_UNORM:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case GPU_FORMAT_R32_UINT:
            return VK_FORMAT_R32_UINT;
        case GPU_FORMAT_R32_SFLOAT:
            return VK_FORMAT_R32_SFLOAT;
        case GPU_FORMAT_R32G32_SFLOAT:
            return VK_FORMAT_R32G32_SFLOAT;
        case GPU_FORMAT_R32G32B32_SFLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case GPU_FORMAT_R32G32B32A32_SFLOAT:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case GPU_FORMAT_D16_UNORM_S8_UINT:
            return VK_FORMAT_D16_UNORM_S8_UINT;
        case GPU_FORMAT_D24_UNORM_S8_UINT:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case GPU_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case GPU_FORMAT_D16_UNORM:
            return VK_FORMAT_D16_UNORM;
        case GPU_FORMAT_D32_SFLOAT:
            return VK_FORMAT_D32_SFLOAT;
        case GPU_FORMAT_R16G16_SFLOAT:
            return VK_FORMAT_R16G16_SFLOAT;
        case GPU_FORMAT_R16G16B16_SFLOAT:
            return VK_FORMAT_R16G16B16_SFLOAT;
        case GPU_FORMAT_R16G16B16A16_SFLOAT:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        default: break;
    }
    return VK_FORMAT_UNDEFINED;
}

EGPUFormat VulkanUtil_VulkanFormatToGPUFormat(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_B8G8R8A8_UNORM:
            return EGPUFormat::GPU_FORMAT_B8G8R8A8_UNORM;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return EGPUFormat::GPU_FORMAT_B8G8R8A8_SRGB;
        case VK_FORMAT_R8G8B8A8_UNORM:
            return EGPUFormat::GPU_FORMAT_R8G8B8A8_UNORM;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return EGPUFormat::GPU_FORMAT_R8G8B8A8_SRGB;
        case VK_FORMAT_R16_UINT:
            return GPU_FORMAT_R16_UINT;
        case VK_FORMAT_R16G16B16_UNORM:
            return GPU_FORMAT_R16G16B16_UNORM;
        case VK_FORMAT_R16G16B16A16_UNORM:
            return GPU_FORMAT_R16G16B16A16_UNORM;
        case VK_FORMAT_R32_UINT:
            return GPU_FORMAT_R32_UINT;
        case VK_FORMAT_R32_SFLOAT:
            return GPU_FORMAT_R32_SFLOAT;
        case VK_FORMAT_R32G32_SFLOAT:
            return GPU_FORMAT_R32G32_SFLOAT;
        case VK_FORMAT_R32G32B32_SFLOAT:
            return GPU_FORMAT_R32G32B32_SFLOAT;
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return GPU_FORMAT_R32G32B32A32_SFLOAT;
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return GPU_FORMAT_D16_UNORM_S8_UINT;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return GPU_FORMAT_D24_UNORM_S8_UINT;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return GPU_FORMAT_D32_SFLOAT_S8_UINT;
        case VK_FORMAT_R16G16_SFLOAT:
            return GPU_FORMAT_R16G16_SFLOAT;
        case VK_FORMAT_R16G16B16_SFLOAT:
            return GPU_FORMAT_R16G16B16_SFLOAT;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return GPU_FORMAT_R16G16B16A16_SFLOAT;
        default: break;
    }
    return GPU_FORMAT_COUNT;
}

VkPrimitiveTopology VulkanUtil_PrimitiveTopologyToVk(EGPUPrimitiveTopology topology)
{
    VkPrimitiveTopology vk_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    switch (topology)
    {
        case GPU_PRIM_TOPO_POINT_LIST:
            vk_topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case GPU_PRIM_TOPO_LINE_LIST:
            vk_topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case GPU_PRIM_TOPO_LINE_STRIP:
            vk_topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case GPU_PRIM_TOPO_TRI_STRIP:
            vk_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        case GPU_PRIM_TOPO_PATCH_LIST:
            vk_topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            break;
        case GPU_PRIM_TOPO_TRI_LIST:
            vk_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        default:
            assert(false);
            break;
    }
    return vk_topology;
}

VkImageAspectFlags VulkanUtil_DeterminAspectMask(VkFormat format, bool includeStencilBit)
{
    VkImageAspectFlags result = 0;
        switch (format)
        {
            // Depth
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT:
            result = VK_IMAGE_ASPECT_DEPTH_BIT;
            break;
            // Stencil
            case VK_FORMAT_S8_UINT:
            result = VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
            // Depth/stencil
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
            result = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (includeStencilBit)
                result |= VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
            // Assume everything else is Color
            default:
            result = VK_IMAGE_ASPECT_COLOR_BIT;
            break;
        }
        return result;
}

VkSampleCountFlagBits VulkanUtil_SampleCountToVk(EGPUSampleCount sampleCount)
{
    VkSampleCountFlagBits result = VK_SAMPLE_COUNT_1_BIT;
    switch (sampleCount)
    {
        case GPU_SAMPLE_COUNT_1:
            result = VK_SAMPLE_COUNT_1_BIT;
            break;
        case GPU_SAMPLE_COUNT_2:
            result = VK_SAMPLE_COUNT_2_BIT;
            break;
        case GPU_SAMPLE_COUNT_4:
            result = VK_SAMPLE_COUNT_4_BIT;
            break;
        case GPU_SAMPLE_COUNT_8:
            result = VK_SAMPLE_COUNT_8_BIT;
            break;
        case GPU_SAMPLE_COUNT_16:
            result = VK_SAMPLE_COUNT_16_BIT;
            break;
        default:
            return result;
    }
    return result;
}

VkCompareOp VulkanUtil_CompareOpToVk(EGPUCompareMode compareMode)
{
    switch (compareMode)
    {
        case GPU_CMP_NEVER:
            return VK_COMPARE_OP_NEVER;
        case GPU_CMP_LESS:
            return VK_COMPARE_OP_LESS;
        case GPU_CMP_EQUAL:
            return VK_COMPARE_OP_EQUAL;
        case GPU_CMP_LEQUAL:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case GPU_CMP_GREATER:
            return VK_COMPARE_OP_GREATER;
        case GPU_CMP_NOTEQUAL:
            return VK_COMPARE_OP_NOT_EQUAL;
        case GPU_CMP_GEQUAL:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case GPU_CMP_ALWAYS:
            return VK_COMPARE_OP_ALWAYS;
        default: break;
    }
    return VK_COMPARE_OP_NEVER;
}

VkStencilOp VulkanUtil_StencilOpToVk(EGPUStencilOp op)
{
    switch (op)
    {
        case GPU_STENCIL_OP_KEEP:
            return VK_STENCIL_OP_KEEP;
        case GPU_STENCIL_OP_SET_ZERO:
            return VK_STENCIL_OP_ZERO;
        case GPU_STENCIL_OP_REPLACE:
            return VK_STENCIL_OP_REPLACE;
        case GPU_STENCIL_OP_INVERT:
            return VK_STENCIL_OP_INVERT;
        case GPU_STENCIL_OP_INCR:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case GPU_STENCIL_OP_DECR:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        case GPU_STENCIL_OP_INCR_SAT:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case GPU_STENCIL_OP_DECR_SAT:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        default: break;
    }
    return VK_STENCIL_OP_KEEP;
}

VkBufferUsageFlags VulkanUtil_DescriptorTypesToImageUsage(GPUResourceTypes descriptors)
{
    VkImageUsageFlags result = 0;
    if (GPU_RESOURCE_TYPE_TEXTURE == (descriptors & GPU_RESOURCE_TYPE_TEXTURE))
        result |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (GPU_RESOURCE_TYPE_RW_TEXTURE == (descriptors & GPU_RESOURCE_TYPE_RW_TEXTURE))
        result |= VK_IMAGE_USAGE_STORAGE_BIT;
    return result;
}

VkBufferUsageFlags VulkanUtil_DescriptorTypesToBufferUsage(GPUResourceTypes descriptors, bool texel)
{
    VkBufferUsageFlags result = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (descriptors & GPU_RESOURCE_TYPE_UNIFORM_BUFFER)
    {
        result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (descriptors & GPU_RESOURCE_TYPE_RW_BUFFER)
    {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (texel) result |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
    }
    if (descriptors & GPU_RESOURCE_TYPE_BUFFER)
    {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (texel) result |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    }
    if (descriptors & GPU_RESOURCE_TYPE_INDEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (descriptors & GPU_RESOURCE_TYPE_VERTEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (descriptors & GPU_RESOURCE_TYPE_INDIRECT_BUFFER)
    {
        result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
#ifdef ENABLE_RAYTRACING
    if (descriptors & CGPU_RESOURCE_TYPE_RAY_TRACING)
    {
        result |= VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
    }
#endif
    return result;
}

VkFormatFeatureFlags VulkanUtil_ImageUsageToFormatFeatures(VkImageUsageFlags usage)
{
    VkFormatFeatureFlags result = (VkFormatFeatureFlags)0;
    if (VK_IMAGE_USAGE_SAMPLED_BIT == (usage & VK_IMAGE_USAGE_SAMPLED_BIT))
    {
        result |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    }
    if (VK_IMAGE_USAGE_STORAGE_BIT == (usage & VK_IMAGE_USAGE_STORAGE_BIT))
    {
        result |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
    }
    if (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT == (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
    {
        result |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    }
    if (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT == (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
    {
        result |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    return result;
}

const char8_t* push_constants_name = u8"push_constants";
void VulkanUtil_InitializeShaderReflection(GPUDeviceID device, GPUShaderLibrary_Vulkan* S, const struct GPUShaderLibraryDescriptor* desc)
{
    S->pReflect             = (SpvReflectShaderModule*)malloc(sizeof(SpvReflectShaderModule));
    SpvReflectResult spvRes = spvReflectCreateShaderModule(desc->codeSize, desc->code, S->pReflect);
    (void)spvRes;
    assert(spvRes == SPV_REFLECT_RESULT_SUCCESS && "Failed to Reflect Shader!");
    uint32_t entry_count       = S->pReflect->entry_point_count;
    S->super.entrys_count      = entry_count;
    S->super.entry_reflections = (GPUShaderReflection*)malloc(entry_count * sizeof(GPUShaderReflection));
    memset(S->super.entry_reflections, 0, entry_count * sizeof(GPUShaderReflection));
    for (uint32_t i = 0; i < entry_count; i++)
    {
        // Initialize Common Reflection Data
        GPUShaderReflection* reflection = &S->super.entry_reflections[i];
        // ATTENTION: We have only one entry point now
        const SpvReflectEntryPoint* entry = spvReflectGetEntryPoint(S->pReflect, S->pReflect->entry_points[i].name);
        reflection->entry_name            = (const char8_t*)entry->name;
        reflection->stage                 = (EGPUShaderStage)entry->shader_stage;
        if (reflection->stage == GPU_SHADER_STAGE_COMPUTE)
        {
            reflection->thread_group_sizes[0] = entry->local_size.x;
            reflection->thread_group_sizes[1] = entry->local_size.y;
            reflection->thread_group_sizes[2] = entry->local_size.z;
        }
        const bool bGLSL = S->pReflect->source_language & SpvSourceLanguageGLSL;
        (void)bGLSL;
        const bool bHLSL = S->pReflect->source_language & SpvSourceLanguageHLSL;
        uint32_t icount;
        spvReflectEnumerateInputVariables(S->pReflect, &icount, NULL);
        if (icount > 0)
        {
            DECLARE_ZERO_VAL(SpvReflectInterfaceVariable*, input_vars, icount)
            spvReflectEnumerateInputVariables(S->pReflect, &icount, input_vars);
            if ((entry->shader_stage & SPV_REFLECT_SHADER_STAGE_VERTEX_BIT))
            {
                reflection->vertex_inputs_count = icount;
                reflection->vertex_inputs       = (GPUVertexInput*)calloc(icount, sizeof(GPUVertexInput));
                // Handle Vertex Inputs
                for (uint32_t i = 0; i < icount; i++)
                {
                    // We use semantic for HLSL sources because DXC is a piece of shit.
                    reflection->vertex_inputs[i].name =
                    bHLSL ? (char8_t*)input_vars[i]->semantic : (char8_t*)input_vars[i]->name;
                    reflection->vertex_inputs[i].format =
                    VulkanUtil_VulkanFormatToGPUFormat((VkFormat)input_vars[i]->format);
                }
            }
        }
        // Handle Descriptor Sets
        uint32_t scount;
        uint32_t ccount;
        spvReflectEnumeratePushConstantBlocks(S->pReflect, &ccount, NULL);
        spvReflectEnumerateDescriptorSets(S->pReflect, &scount, NULL);
        if (scount > 0 || ccount > 0)
        {
            DECLARE_ZERO_VAL(SpvReflectDescriptorSet*, descriptros_sets, scount + 1)
            DECLARE_ZERO_VAL(SpvReflectBlockVariable*, root_sets, ccount + 1)
            spvReflectEnumerateDescriptorSets(S->pReflect, &scount, descriptros_sets);
            spvReflectEnumeratePushConstantBlocks(S->pReflect, &ccount, root_sets);
            uint32_t bcount = 0;
            for (uint32_t i = 0; i < scount; i++)
            {
                bcount += descriptros_sets[i]->binding_count;
            }
            bcount += ccount;
            reflection->shader_resources_count = bcount;
            reflection->shader_resources       = (CGPUShaderResource*)calloc(bcount, sizeof(GPUShaderResource));
            // Fill Shader Resources
            uint32_t i_res = 0;
            for (uint32_t i_set = 0; i_set < scount; i_set++)
            {
                SpvReflectDescriptorSet* current_set = descriptros_sets[i_set];
                for (uint32_t i_binding = 0; i_binding < current_set->binding_count; i_binding++, i_res++)
                {
                    SpvReflectDescriptorBinding* current_binding = current_set->bindings[i_binding];
                    CGPUShaderResource* current_res              = &reflection->shader_resources[i_res];
                    current_res->set                             = current_binding->set;
                    current_res->binding                         = current_binding->binding;
                    current_res->stages                          = S->pReflect->shader_stage;
                    current_res->type                            = RTLut[current_binding->descriptor_type];
                    current_res->name                            = (char8_t*)current_binding->name;
                    current_res->name_hash                       = GPU_NAME_HASH(current_binding->name, strlen(current_binding->name));
                    current_res->size                            = current_binding->count;
                    // Solve Dimension
                    if ((current_binding->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_IMAGE) ||
                        (current_binding->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE))
                    {
                        if (current_binding->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_ARRAY)
                            current_res->dim = ArrDIMLut[current_binding->image.dim];
                        else
                            current_res->dim = DIMLut[current_binding->image.dim];
                        if (current_binding->image.ms)
                        {
                            current_res->dim = current_res->dim & GPU_TEX_DIMENSION_2D ? GPU_TEX_DIMENSION_2DMS : current_res->dim;
                            current_res->dim = current_res->dim & GPU_TEX_DIMENSION_2D_ARRAY ? GPU_TEX_DIMENSION_2DMS_ARRAY : current_res->dim;
                        }
                    }
                }
            }
            // Fill Push Constants
            for (uint32_t i = 0; i < ccount; i++)
            {
                CGPUShaderResource* current_res = &reflection->shader_resources[i_res + i];
                current_res->set                = 0;
                current_res->type               = GPU_RESOURCE_TYPE_PUSH_CONSTANT;
                current_res->binding            = 0;
                current_res->name               = push_constants_name;
                current_res->name_hash          = GPU_NAME_HASH((const char*)current_res->name, strlen((const char*)current_res->name));
                current_res->stages             = S->pReflect->shader_stage;
                current_res->size               = root_sets[i]->size;
                current_res->offset             = root_sets[i]->offset;
            }
        }
    }
}

void VulkanUtil_FreeShaderReflection(GPUShaderLibrary_Vulkan* S)
{
    spvReflectDestroyShaderModule(S->pReflect);
    if (S->super.entry_reflections)
    {
        for (uint32_t i = 0; i < S->super.entrys_count; i++)
        {
            GPUShaderReflection* reflection = S->super.entry_reflections + i;
            if (reflection->vertex_inputs) free(reflection->vertex_inputs);
            if (reflection->shader_resources) free(reflection->shader_resources);
        }
    }
    GPU_SAFE_FREE(S->super.entry_reflections);
    GPU_SAFE_FREE(S->pReflect);
}

VkShaderStageFlags VulkanUtil_TranslateShaderUsages(GPUShaderStages shader_stages)
{
    VkShaderStageFlags result = 0;
    if (GPU_SHADER_STAGE_ALL_GRAPHICS == (shader_stages & GPU_SHADER_STAGE_ALL_GRAPHICS))
    {
        result = VK_SHADER_STAGE_ALL_GRAPHICS;
    }
    else
    {
        if (GPU_SHADER_STAGE_VERT == (shader_stages & GPU_SHADER_STAGE_VERT))
        {
            result |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (GPU_SHADER_STAGE_TESC == (shader_stages & GPU_SHADER_STAGE_TESC))
        {
            result |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }
        if (GPU_SHADER_STAGE_TESE == (shader_stages & GPU_SHADER_STAGE_TESE))
        {
            result |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        }
        if (GPU_SHADER_STAGE_GEOM == (shader_stages & GPU_SHADER_STAGE_GEOM))
        {
            result |= VK_SHADER_STAGE_GEOMETRY_BIT;
        }
        if (GPU_SHADER_STAGE_FRAG == (shader_stages & GPU_SHADER_STAGE_FRAG))
        {
            result |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if (GPU_SHADER_STAGE_COMPUTE == (shader_stages & GPU_SHADER_STAGE_COMPUTE))
        {
            result |= VK_SHADER_STAGE_COMPUTE_BIT;
        }
    }
    return result;
}

VkDescriptorType VulkanUtil_TranslateResourceType(EGPUResourceType type)
{
    switch (type)
    {
        case GPU_RESOURCE_TYPE_NONE:
            assert(0 && "Invalid DescriptorInfo Type");
            return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        case GPU_RESOURCE_TYPE_SAMPLER:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case GPU_RESOURCE_TYPE_TEXTURE:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case GPU_RESOURCE_TYPE_UNIFORM_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case GPU_RESOURCE_TYPE_RW_TEXTURE:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case GPU_RESOURCE_TYPE_BUFFER:
        case GPU_RESOURCE_TYPE_RW_BUFFER:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case GPU_RESOURCE_TYPE_RW_BUFFER_RAW:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        case GPU_RESOURCE_TYPE_INPUT_ATTACHMENT:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        case GPU_RESOURCE_TYPE_TEXEL_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case GPU_RESOURCE_TYPE_RW_TEXEL_BUFFER:
            return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case GPU_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
#ifdef ENABLE_RAYTRACING
        case CGPU_RESOURCE_TYPE_RAY_TRACING:
            return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
#endif
        default:
            assert(0 && "Invalid DescriptorInfo Type");
            return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

VkAccessFlags VulkanUtil_ResourceStateToVkAccessFlags(EGPUResourceState state)
{
    VkAccessFlags ret = 0;
    if (state & GPU_RESOURCE_STATE_COPY_SOURCE)
        ret |= VK_ACCESS_TRANSFER_READ_BIT;
    if (state & GPU_RESOURCE_STATE_COPY_DEST)
        ret |= VK_ACCESS_TRANSFER_WRITE_BIT;
    if (state & GPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
        ret |= VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    if (state & GPU_RESOURCE_STATE_INDEX_BUFFER)
        ret |= VK_ACCESS_INDEX_READ_BIT;
    if (state & GPU_RESOURCE_STATE_UNORDERED_ACCESS)
        ret |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    if (state & GPU_RESOURCE_STATE_INDIRECT_ARGUMENT)
        ret |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    if (state & GPU_RESOURCE_STATE_RENDER_TARGET)
        ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    if (state & GPU_RESOURCE_STATE_RESOLVE_DEST)
        ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    if (state & GPU_RESOURCE_STATE_DEPTH_WRITE)
        ret |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    if (state & GPU_RESOURCE_STATE_SHADER_RESOURCE)
        ret |= VK_ACCESS_SHADER_READ_BIT;
    if (state & GPU_RESOURCE_STATE_PRESENT)
    {
        ret |= VK_ACCESS_MEMORY_READ_BIT;
    }
#ifdef ENABLE_RAYTRACING
    if (state & GPU_RESOURCE_STATE_ACCELERATION_STRUCTURE)
        ret |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
#endif
    return ret;
}

void VulkanUtil_ConsumeDescriptorSets(VulkanUtil_DescriptorPool* pool, const VkDescriptorSetLayout* pLayouts, VkDescriptorSet* pSets, uint32_t setsNum)
{
    VkDescriptorSetAllocateInfo setsAllocInfo{};
    setsAllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setsAllocInfo.descriptorPool     = pool->pVkDescPool;
    setsAllocInfo.descriptorSetCount = setsNum;
    setsAllocInfo.pSetLayouts        = pLayouts;
    VkResult rs = pool->Device->mVkDeviceTable.vkAllocateDescriptorSets(pool->Device->pDevice, &setsAllocInfo, pSets);
    assert(rs == VK_SUCCESS);
}

void VulkanUtil_ReturnDescriptorSets(struct VulkanUtil_DescriptorPool* pPool, VkDescriptorSet* pSets, uint32_t setsNum)
{
    // TODO: It is possible to avoid using that flag by updating descriptor sets instead of deleting them.
    // The application can keep track of recycled descriptor sets and re-use one of them when a new one is requested.
    // Reference: https://arm-software.github.io/vulkan_best_practice_for_mobile_developers/samples/performance/descriptor_management/descriptor_management_tutorial.html
    GPUDevice_Vulkan* D = (GPUDevice_Vulkan*)pPool->Device;
    D->mVkDeviceTable.vkFreeDescriptorSets(D->pDevice, pPool->pVkDescPool, setsNum, pSets);
}

uint32_t VulkanUtil_BitSizeOfBlock(EGPUFormat format)
{
    //todo : other format
    switch (format)
    {
        case EGPUFormat::GPU_FORMAT_R16_UINT: return 16;
        case EGPUFormat::GPU_FORMAT_R32_UINT: return 32;
        case EGPUFormat::GPU_FORMAT_R32_SFLOAT: return 32;
        case EGPUFormat::GPU_FORMAT_R32G32_SFLOAT: return 64;
        case EGPUFormat::GPU_FORMAT_R32G32B32_SFLOAT: return 96;
        case EGPUFormat::GPU_FORMAT_R32G32B32A32_SFLOAT: return 128;
        default: break;
    }
    return 0;
}

VkImageLayout VulkanUtil_ResourceStateToImageLayout(EGPUResourceState usage)
{
    if (usage & GPU_RESOURCE_STATE_COPY_SOURCE)
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    if (usage & GPU_RESOURCE_STATE_COPY_DEST)
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    if (usage & GPU_RESOURCE_STATE_RENDER_TARGET)
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if (usage & GPU_RESOURCE_STATE_RESOLVE_DEST)
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    if (usage & GPU_RESOURCE_STATE_DEPTH_WRITE)
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    if (usage & GPU_RESOURCE_STATE_UNORDERED_ACCESS)
        return VK_IMAGE_LAYOUT_GENERAL;

    if (usage & GPU_RESOURCE_STATE_SHADER_RESOURCE)
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    if (usage & GPU_RESOURCE_STATE_PRESENT)
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    if (usage == GPU_RESOURCE_STATE_COMMON)
        return VK_IMAGE_LAYOUT_GENERAL;

    if (usage == GPU_RESOURCE_STATE_SHADING_RATE_SOURCE)
        return VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkPipelineStageFlags VulkanUtil_DeterminePipelineStageFlags(GPUAdapter_Vulkan* A, VkAccessFlags accessFlags, EGPUQueueType queue_type)
{
    VkPipelineStageFlags flags = 0;

    switch (queue_type)
    {
        case GPU_QUEUE_TYPE_GRAPHICS: {
            if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

            if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
            {
                flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                /*if (A->adapterDetail.support_geom_shader)
                {
                    flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
                }
                if (A->adapterDetail.support_tessellation)
                {
                    flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
                    flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
                }*/
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
#ifdef ENABLE_RAYTRACING
                if (pRenderer->mVulkan.mRaytracingExtension)
                {
                    flags |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV;
                }
#endif
            }
            if ((accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0)
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            if ((accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            if ((accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        }
        case GPU_QUEUE_TYPE_COMPUTE: {
            if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0 ||
                (accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0 ||
                (accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0 ||
                (accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
                return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

            if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

            break;
        }
        case GPU_QUEUE_TYPE_TRANSFER:
            return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        default:
            break;
    }
    // Compatible with both compute and graphics queues
    if ((accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) != 0)
        flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

    if ((accessFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT)) != 0)
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

    if ((accessFlags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT)) != 0)
        flags |= VK_PIPELINE_STAGE_HOST_BIT;

    if (flags == 0)
        flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    return flags;
}

VkFilter VulkanUtil_TranslateFilterType(EGPUFilterType type)
{
    switch (type)
    {
        case GPU_FILTER_TYPE_LINEAR:
            return VK_FILTER_LINEAR;
        case GPU_FILTER_TYPE_NEAREST:
            return VK_FILTER_NEAREST;
        default:
            return VK_FILTER_LINEAR;
    }
}

VkSamplerMipmapMode VulkanUtil_TranslateMipMapMode(EGPUMipMapMode mode)
{
    switch (mode)
    {
        case GPU_MIPMAP_MODE_LINEAR:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        case GPU_MIPMAP_MODE_NEAREST:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        default:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }
}

VkSamplerAddressMode VulkanUtil_TranslateAddressMode(EGPUAddressMode mode)
{
    switch (mode)
    {
        case GPU_ADDRESS_MODE_MIRROR:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case GPU_ADDRESS_MODE_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case GPU_ADDRESS_MODE_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case GPU_ADDRESS_MODE_CLAMP_TO_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}
