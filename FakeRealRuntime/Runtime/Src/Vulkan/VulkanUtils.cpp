#include "Gpu/Backend/Vulkan/VulkanUtils.h"
#include <new>
#include <memory>

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
        VkFormat fmt = GPUFormatToVulkanFormat((EGPUFormat)i);
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
            GPU_LOG_INFO("Vulkan validation verbose layer:\n", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            GPU_LOG_INFO("Vulkan validation info layer:\n", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            GPU_LOG_WARNING("Vulkan validation warning layer:\n", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            GPU_LOG_ERROR("Vulkan validation error layer:\n", pCallbackData->pMessage);
            break;
        default:
            return VK_TRUE;
    }
    return VK_FALSE;
}