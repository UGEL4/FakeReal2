#include "Gpu/Backend/Vulkan/GPUVulkan.h"
#include "Gpu/Backend/Vulkan/VulkanExtensions.h"
#include "Gpu/Backend/Vulkan/VulkanUtils.h"
#include "../Common/CommonUtils.h"
#include "Core/Hash/hash.h"
#include <vector>
#include <algorithm>

class VulkanBlackboard
{
public:
    VulkanBlackboard(const GPUInstanceDescriptor* pDesc)
    {
        const GPUVulkanInstanceDescriptor* vk_desc = (const GPUVulkanInstanceDescriptor*)pDesc->pChained;

        // default
        uint32_t count = sizeof(s_intanceWantedExtensions) / sizeof(const char*);
        mInstanceExtensions.insert(mInstanceExtensions.end(), s_intanceWantedExtensions, s_intanceWantedExtensions + count);
        count = sizeof(s_deviceWantedExtensions) / sizeof(const char*);
        mDeviceExtensions.insert(mDeviceExtensions.end(), s_deviceWantedExtensions, s_deviceWantedExtensions + count);

        if (pDesc->enableDebugLayer)
        {
            mInstanceLayers.push_back(s_validationLayerName);
            mInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        if (vk_desc != VK_NULL_HANDLE)
        {
            if (vk_desc->backend != EGPUBackend::GPUBackend_Vulkan)
            {
                assert(0 && "Not Vulkan Backend Blackboard!");
                vk_desc = VK_NULL_HANDLE;
            }

            if (vk_desc->ppInstanceLayers != VK_NULL_HANDLE && vk_desc->mInstanceLayerCount > 0)
            {
                mInstanceLayers.insert(mInstanceExtensions.end(), vk_desc->ppInstanceLayers, vk_desc->ppInstanceLayers + vk_desc->mInstanceLayerCount);
            }

            if (vk_desc->ppInstanceExtensions != VK_NULL_HANDLE && vk_desc->mInstanceExtensionCount > 0)
            {
                mInstanceExtensions.insert(mInstanceExtensions.end(), vk_desc->ppInstanceExtensions, vk_desc->ppInstanceExtensions + vk_desc->mInstanceExtensionCount);
            }
        }
    }

    const VkDebugUtilsMessengerCreateInfoEXT* pDeubgUtilsMessengerCreateInfo = VK_NULL_HANDLE;
    std::vector<const char*> mInstanceLayers;
    std::vector<const char*> mInstanceExtensions;
    std::vector<const char*> mDeviceExtensions;
};

struct GPUCachedRenderPass
{
    VkRenderPass pPass;
    size_t timestamp;
};

struct GPUCachedFrameBuffer
{
    VkFramebuffer pBuffer;
};

struct GPUVkPassTable
{
    struct VulkanRenderPassDescriptorHasher
    {
        size_t operator()(const VulkanRenderPassDescriptor& a) const
        {
            return Hash64(&a, sizeof(VulkanRenderPassDescriptor), CGPU_NAME_HASH_SEED);
        }
    };

    struct VulkanRenderPassDescriptorEqualTo
    {
        bool operator()(const VulkanRenderPassDescriptor& a, const VulkanRenderPassDescriptor& b)
        {
            if (a.attachmentCount != b.attachmentCount) return false;
            return std::memcmp(&a, &b, sizeof(VulkanRenderPassDescriptor)) == 0;
        }
    };

    struct VulkanFramebufferDesriptorHasher
    {
        size_t operator()(const VulkanFramebufferDesriptor& a) const
        {
            return Hash64(&a, sizeof(VulkanFramebufferDesriptor), CGPU_NAME_HASH_SEED);
        }
    };

    struct VulkanFramebufferDesriptorEqualTo
    {
        bool operator()(const VulkanFramebufferDesriptor& a, const VulkanFramebufferDesriptor& b)
        {
            if (a.pRenderPass != b.pRenderPass) return false;
            if (a.attachmentCount != b.attachmentCount) return false;
            return std::memcmp(&a, &b, sizeof(VulkanFramebufferDesriptor)) == 0;
        }
    };

    std::unordered_map<VulkanRenderPassDescriptor, GPUCachedRenderPass, VulkanRenderPassDescriptorHasher, VulkanRenderPassDescriptorEqualTo> cached_renderpasses;
    std::unordered_map<VulkanFramebufferDesriptor, GPUCachedFrameBuffer, VulkanFramebufferDesriptorHasher, VulkanFramebufferDesriptorEqualTo> cached_framebuffers;
};

GPUInstanceID GPUCreateInstance_Vulkan(const GPUInstanceDescriptor* pDesc)
{
    VulkanBlackboard blackBoard(pDesc);

    GPUInstance_Vulkan* I = (GPUInstance_Vulkan*)calloc(1, sizeof(GPUInstance_Vulkan));

    VkResult result = volkInitialize();
    if (result != VK_SUCCESS)
    {
        assert(0 && "Volk init failed!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "GPU";
    appInfo.apiVersion         = VK_API_VERSION_1_3;
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_API_VERSION(0, 1, 3, 0);

    VulkanUtil_SelectValidationLayers(I, blackBoard.mInstanceLayers.data(), (uint32_t)blackBoard.mInstanceLayers.size());
    VulkanUtil_SelectInstanceExtensions(I, blackBoard.mInstanceExtensions.data(), (uint32_t)blackBoard.mInstanceExtensions.size());

    VkInstanceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledLayerCount       = (uint32_t)blackBoard.mInstanceLayers.size();
    createInfo.ppEnabledLayerNames     = blackBoard.mInstanceLayers.data();
    createInfo.enabledExtensionCount   = (uint32_t)blackBoard.mInstanceExtensions.size();
    createInfo.ppEnabledExtensionNames = blackBoard.mInstanceExtensions.data();

    if (pDesc->enableValidation)
    {
        if (!pDesc->enableDebugLayer)
        {
            assert(0 && "Enable vulkan validation while desabled debug layer!");
        }

        VkValidationFeaturesEXT validationFeatureExt{};
        validationFeatureExt.sType                    = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        VkValidationFeatureEnableEXT enableFeatures[] = {
            VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
            VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT
        };
        validationFeatureExt.pEnabledValidationFeatures    = enableFeatures;
        validationFeatureExt.enabledValidationFeatureCount = (uint32_t)(sizeof(enableFeatures) / sizeof(VkValidationFeatureEnableEXT));
        createInfo.pNext                                   = &validationFeatureExt;
    }

    result = vkCreateInstance(&createInfo, GLOBAL_VkAllocationCallbacks, &I->pInstance);
    if (result != VK_SUCCESS)
    {
        assert(0);
    }

    volkLoadInstance(I->pInstance);

    // Adapters
    VulkanUtil_QueryAllAdapters(I, blackBoard.mDeviceExtensions.data(), (uint32_t)blackBoard.mDeviceExtensions.size());
    std::sort(I->pAdapters, I->pAdapters + I->adapterCount, [](const GPUAdapter_Vulkan& a, const GPUAdapter_Vulkan& b) {
        const uint32_t orders[] = {
            4, 1, 0, 2, 3
        };
        return orders[a.physicalDeviceProperties.properties.deviceType] < orders[b.physicalDeviceProperties.properties.deviceType];
    });

    if (pDesc->enableDebugLayer)
    {
        I->debugUtils = 1; // TODO:
        VulkanUtil_EnableValidationLayers(I, blackBoard.pDeubgUtilsMessengerCreateInfo);
    }

    return &(I->super);
}

void GPUFreeInstance_Vllkan(GPUInstanceID pInstance)
{
    GPUInstance_Vulkan* p = (GPUInstance_Vulkan*)pInstance;

    if (p->pDebugUtils)
    {
        assert(vkDestroyDebugUtilsMessengerEXT && "load vkDestroyDebugUtilsMessengerEXT failed!");
        vkDestroyDebugUtilsMessengerEXT(p->pInstance, p->pDebugUtils, GLOBAL_VkAllocationCallbacks);
    }

    vkDestroyInstance(p->pInstance, GLOBAL_VkAllocationCallbacks);

    GPU_SAFE_FREE(p->pLayerProperties);
    GPU_SAFE_FREE(p->pLayerNames);

    GPU_SAFE_FREE(p->pExtensonProperties);
    GPU_SAFE_FREE(p->pExtensionNames);

    GPU_SAFE_FREE(p);
}

void GPUEnumerateAdapters_Vulkan(GPUInstanceID pInstance, GPUAdapterID* const ppAdapters, uint32_t* adapterCount)
{
    const GPUInstance_Vulkan* pVkInstance = (const GPUInstance_Vulkan*)pInstance;
    *adapterCount                         = pVkInstance->adapterCount;
    if (ppAdapters != VK_NULL_HANDLE)
    {
        for (uint32_t i = 0; i < *adapterCount; i++)
        {
            ppAdapters[i] = &pVkInstance->pAdapters[i].super;
        }
    }
}

const float queuePriorities[] = {
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
};
GPUDeviceID GPUCreateDevice_Vulkan(GPUAdapterID pAdapter, const GPUDeviceDescriptor* pDesc)
{
    GPUInstance_Vulkan* pVkInstance = (GPUInstance_Vulkan*)pAdapter->pInstance;
    GPUDevice_Vulkan* pDevice       = (GPUDevice_Vulkan*)malloc(sizeof(GPUDevice_Vulkan));
    GPUAdapter_Vulkan* pVkAdapter   = (GPUAdapter_Vulkan*)pAdapter;

    *const_cast<GPUAdapterID*>(&pDevice->spuer.pAdapter) = pAdapter;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(pDesc->queueGroupCount);
    for (uint32_t i = 0; i < pDesc->queueGroupCount; i++)
    {
        GPUQueueGroupDescriptor& descriptor = pDesc->pQueueGroup[i];
        VkDeviceQueueCreateInfo& info       = queueCreateInfos[i];
        info.sType                          = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueCount                     = descriptor.queueCount;
        info.queueFamilyIndex               = (uint32_t)pVkAdapter->queueFamilyIndices[descriptor.queueType];
        info.pQueuePriorities               = queuePriorities;

        assert(GPUQueryQueueCount_Vulkan(pAdapter, descriptor.queueType) >= descriptor.queueCount);
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext                = &pVkAdapter->physicalDeviceFeatures;
    createInfo.queueCreateInfoCount = pDesc->queueGroupCount;
    createInfo.pQueueCreateInfos    = queueCreateInfos.data();
    createInfo.pEnabledFeatures     = VK_NULL_HANDLE;
    // layer & extension
    createInfo.enabledExtensionCount   = pVkAdapter->extensionsCount;
    createInfo.ppEnabledExtensionNames = pVkAdapter->ppExtensionsName;

    VkResult result = vkCreateDevice(pVkAdapter->pPhysicalDevice, &createInfo, GLOBAL_VkAllocationCallbacks, &pDevice->pDevice);
    if (result != VK_SUCCESS)
    {
        assert(0);
    }

    volkLoadDeviceTable(&pDevice->mVkDeviceTable, pDevice->pDevice);
    assert(pDevice->mVkDeviceTable.vkCreateSwapchainKHR);

    // pipeline cache

    //descriptor pool
    pDevice->pDescriptorPool = (VkUtil_DescriptorPool*)calloc(1, sizeof(VkUtil_DescriptorPool));
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.poolSizeCount = GPU_VK_DESCRIPTOR_TYPE_RANGE_SIZE;
    poolInfo.pPoolSizes    = gDescriptorPoolSizes;
    poolInfo.maxSets       = 8192;
    result = pDevice->mVkDeviceTable.vkCreateDescriptorPool(pDevice->pDevice, &poolInfo, GLOBAL_VkAllocationCallbacks, &pDevice->pDescriptorPool->pVkDescPool);
    assert(result == VK_SUCCESS);
    pDevice->pDescriptorPool->Device = pDevice;
    pDevice->pDescriptorPool->mFlags = poolInfo.flags;

    //pass table
    pDevice->pPassTable = GPUNew<GPUVkPassTable>();

    //vma
    VmaVulkanFunctions vulkanFunctions = {
        .vkGetPhysicalDeviceProperties       = vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
        .vkAllocateMemory                    = pDevice->mVkDeviceTable.vkAllocateMemory,
        .vkFreeMemory                        = pDevice->mVkDeviceTable.vkFreeMemory,
        .vkMapMemory                         = pDevice->mVkDeviceTable.vkMapMemory,
        .vkUnmapMemory                       = pDevice->mVkDeviceTable.vkUnmapMemory,
        .vkFlushMappedMemoryRanges           = pDevice->mVkDeviceTable.vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges      = pDevice->mVkDeviceTable.vkInvalidateMappedMemoryRanges,
        .vkBindBufferMemory                  = pDevice->mVkDeviceTable.vkBindBufferMemory,
        .vkBindImageMemory                   = pDevice->mVkDeviceTable.vkBindImageMemory,
        .vkGetBufferMemoryRequirements       = pDevice->mVkDeviceTable.vkGetBufferMemoryRequirements,
        .vkGetImageMemoryRequirements        = pDevice->mVkDeviceTable.vkGetImageMemoryRequirements,
        .vkCreateBuffer                      = pDevice->mVkDeviceTable.vkCreateBuffer,
        .vkDestroyBuffer                     = pDevice->mVkDeviceTable.vkDestroyBuffer,
        .vkCreateImage                       = pDevice->mVkDeviceTable.vkCreateImage,
        .vkDestroyImage                      = pDevice->mVkDeviceTable.vkDestroyImage,
        .vkCmdCopyBuffer                     = pDevice->mVkDeviceTable.vkCmdCopyBuffer,
        .vkGetBufferMemoryRequirements2KHR   = pDevice->mVkDeviceTable.vkGetBufferMemoryRequirements2KHR,
        .vkGetImageMemoryRequirements2KHR    = pDevice->mVkDeviceTable.vkGetImageMemoryRequirements2KHR
    };
    VmaAllocatorCreateInfo vma{};
    vma.device               = pDevice->pDevice;
    vma.instance             = pVkInstance->pInstance;
    vma.physicalDevice       = pVkAdapter->pPhysicalDevice;
    vma.pVulkanFunctions     = &vulkanFunctions;
    vma.pAllocationCallbacks = GLOBAL_VkAllocationCallbacks;
    if (vmaCreateAllocator(&vma, &pDevice->pVmaAllocator) != VK_SUCCESS)
    {
        assert(0);
    }

    return &(pDevice->spuer);
}

void GPUFreeDevice_Vulkan(GPUDeviceID pDevice)
{
    GPUDevice_Vulkan* pVkDevice = (GPUDevice_Vulkan*)pDevice;

    for (auto& iter : pVkDevice->pPassTable->cached_renderpasses)
    {
        pVkDevice->mVkDeviceTable.vkDestroyRenderPass(pVkDevice->pDevice, iter.second.pPass, GLOBAL_VkAllocationCallbacks);
    }

    for (auto& iter : pVkDevice->pPassTable->cached_framebuffers)
    {
        pVkDevice->mVkDeviceTable.vkDestroyFramebuffer(pVkDevice->pDevice, iter.second.pBuffer, GLOBAL_VkAllocationCallbacks);
    }

    pVkDevice->mVkDeviceTable.vkDestroyDescriptorPool(pVkDevice->pDevice, pVkDevice->pDescriptorPool->pVkDescPool, GLOBAL_VkAllocationCallbacks);
    vkDestroyDevice(pVkDevice->pDevice, GLOBAL_VkAllocationCallbacks);
    GPUDelete(pVkDevice->pPassTable);
    GPU_SAFE_FREE(pVkDevice->pDescriptorPool);
    GPU_SAFE_FREE(pVkDevice);
}

uint32_t GPUQueryQueueCount_Vulkan(const GPUAdapterID pAdapter, const EGPUQueueType queueType)
{
    const GPUAdapter_Vulkan* ptr = (const GPUAdapter_Vulkan*)pAdapter;
    uint32_t count               = 0;
    switch (queueType)
    {
        case EGPUQueueType::GPU_QUEUE_TYPE_GRAPHICS: {
            for (uint32_t i = 0; i < ptr->queueFamiliesCount; i++)
            {
                VkQueueFamilyProperties& props = ptr->pQueueFamilyProperties[i];
                if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    count += props.queueCount;
                }
            }
        }
        break;
        case EGPUQueueType::GPU_QUEUE_TYPE_COMPUTE: {
            for (uint32_t i = 0; i < ptr->queueFamiliesCount; i++)
            {
                VkQueueFamilyProperties& props = ptr->pQueueFamilyProperties[i];
                if (props.queueFlags & VK_QUEUE_COMPUTE_BIT)
                {
                    if (!(props.queueFlags & VK_QUEUE_GRAPHICS_BIT))
                    {
                        count += props.queueCount;
                    }
                }
            }
        }
        break;
        case EGPUQueueType::GPU_QUEUE_TYPE_TRANSFER: {
            for (uint32_t i = 0; i < ptr->queueFamiliesCount; i++)
            {
                VkQueueFamilyProperties& props = ptr->pQueueFamilyProperties[i];
                if (props.queueFlags & VK_QUEUE_TRANSFER_BIT)
                {
                    if (!(props.queueFlags & VK_QUEUE_COMPUTE_BIT))
                    {
                        if (!(props.queueFlags & VK_QUEUE_GRAPHICS_BIT))
                        {
                            count += props.queueCount;
                        }
                    }
                }
            }
        }
        break;
        default:
            assert(0);
    }
    return count;
}

GPUQueueID GPUGetQueue_Vulkan(GPUDeviceID pDevice, EGPUQueueType queueType, uint32_t queueIndex)
{
    GPUAdapter_Vulkan* pVkAdapter = (GPUAdapter_Vulkan*)pDevice->pAdapter;
    GPUDevice_Vulkan* pVkDevice   = (GPUDevice_Vulkan*)pDevice;
    GPUQueue_Vulkan* pVkQueue     = (GPUQueue_Vulkan*)calloc(1, sizeof(GPUQueue_Vulkan));
    GPUQueue_Vulkan tmpQueue      = {
             .super = {
             .pDevice    = pDevice,
             .queueType  = queueType,
             .queueIndex = queueIndex }
    };
    pVkDevice->mVkDeviceTable.vkGetDeviceQueue(pVkDevice->pDevice, (uint32_t)pVkAdapter->queueFamilyIndices[queueType], queueIndex, &tmpQueue.pQueue);
    tmpQueue.queueFamilyIndex = (uint32_t)pVkAdapter->queueFamilyIndices[queueType];
    memcpy(pVkQueue, &tmpQueue, sizeof(GPUQueue_Vulkan));
    
    pVkQueue->pInnerCmdPool = GPUCreateCommandPool(&pVkQueue->super);
    GPUCommandBufferDescriptor desc{};
    desc.isSecondary          = false;
    pVkQueue->pInnerCmdBuffer = GPUCreateCommandBuffer(pVkQueue->pInnerCmdPool, &desc);
    pVkQueue->pInnerFence     = GPUCreateFence(pDevice);

    return &pVkQueue->super;
}

void GPUFreeQueue_Vulkan(GPUQueueID queue)
{
    GPUQueue_Vulkan* Q = (GPUQueue_Vulkan*)queue;
    if (Q && Q->pInnerCmdBuffer) GPUFreeCommandBuffer(Q->pInnerCmdBuffer);
    if (Q && Q->pInnerCmdPool) GPUFreeCommandPool(Q->pInnerCmdPool);
    if (Q && Q->pInnerFence) GPUFreeFence(Q->pInnerFence);
    free(Q);
}

void GPUSubmitQueue_Vulkan(GPUQueueID queue, const struct GPUQueueSubmitDescriptor* desc)
{
    GPUQueue_Vulkan* Q               = (GPUQueue_Vulkan*)queue;
    GPUDevice_Vulkan* D              = (GPUDevice_Vulkan*)queue->pDevice;
    GPUFence_Vulkan* F               = (GPUFence_Vulkan*)desc->signal_fence;
    uint32_t cmdCount                = desc->cmds_count;
    GPUCommandBuffer_Vulkan** vkCmds = (GPUCommandBuffer_Vulkan**)desc->cmds;
    // cgpu_assert that given cmd list and given params are valid
    assert(cmdCount > 0);
    assert(vkCmds);
    // execute given command list
    assert(Q->pQueue != VK_NULL_HANDLE);
    DECLARE_ZERO_VAL(VkCommandBuffer, cmds, cmdCount);
    for (uint32_t i = 0; i < cmdCount; i++)
    {
        cmds[i] = vkCmds[i]->pVkCmd;
    }

    GPUSemaphore_Vulkan** vkSemaphores = (GPUSemaphore_Vulkan**)desc->wait_semaphores;
    DECLARE_ZERO_VAL(VkSemaphore, ppWaitSemaphore, desc->wait_semaphore_count + 1);
    uint32_t waitCount = 0;
    for (uint32_t i = 0; i < desc->wait_semaphore_count; i++)
    {
        if (vkSemaphores[i]->signaled)
        {
            ppWaitSemaphore[waitCount] = vkSemaphores[i]->pVkSemaphore;
            vkSemaphores[i]->signaled  = false;
            waitCount++;
        }
    }

    GPUSemaphore_Vulkan** vkSignalSemaphores = (GPUSemaphore_Vulkan**)desc->signal_semaphores;
    DECLARE_ZERO_VAL(VkSemaphore, ppSignalSemaphore, desc->signal_semaphore_count + 1);
    uint32_t signalCount = 0;
    for (uint32_t i = 0; i < desc->signal_semaphore_count; i++)
    {
        if (!vkSignalSemaphores[i]->signaled)
        {
            ppSignalSemaphore[signalCount] = vkSignalSemaphores[i]->pVkSemaphore;
            vkSemaphores[i]->signaled    = true;
            signalCount++;
        }
    }

    VkSubmitInfo info{};
    info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.waitSemaphoreCount   = waitCount;
    info.pWaitSemaphores      = waitCount ? ppSignalSemaphore : VK_NULL_HANDLE;
    info.pWaitDstStageMask    = 0;
    info.commandBufferCount   = cmdCount;
    info.pCommandBuffers      = cmds;
    info.signalSemaphoreCount = signalCount;
    info.pSignalSemaphores    = signalCount ? ppWaitSemaphore : VK_NULL_HANDLE;

    VkResult rs = D->mVkDeviceTable.vkQueueSubmit(Q->pQueue, 1, &info, F ? F->pVkFence : VK_NULL_HANDLE);
    if (rs != VK_SUCCESS)
    {
        if (rs == VK_ERROR_DEVICE_LOST)
        {
        }
        else
        {
            assert(0);
        }
    }
    if (F) F->submitted = true;
}

void GPUWaitQueueIdle_Vulkan(GPUQueueID queue)
{
    GPUQueue_Vulkan* Q  = (GPUQueue_Vulkan*)queue;
    GPUDevice_Vulkan* D = (GPUDevice_Vulkan*)queue->pDevice;
    D->mVkDeviceTable.vkQueueWaitIdle(Q->pQueue);
}

void GPUQueuePresent_Vulkan(GPUQueueID queue, const struct GPUQueuePresentDescriptor* desc)
{
    GPUQueue_Vulkan* Q     = (GPUQueue_Vulkan*)queue;
    GPUDevice_Vulkan* D    = (GPUDevice_Vulkan*)queue->pDevice;
    GPUSwapchain_Vulkan* S = (GPUSwapchain_Vulkan*)desc->swapchain;

    if (S)
    {
        uint32_t waitCount = 0;
        DECLARE_ZERO_VAL(VkSemaphore, ppSemaphores, desc->wait_semaphore_count + 1);
        GPUSemaphore_Vulkan** vkSemaphores = (GPUSemaphore_Vulkan**)desc->wait_semaphores;
        for (uint32_t i = 0; i < desc->wait_semaphore_count; i++)
        {
            if (vkSemaphores[i]->signaled)
            {
                ppSemaphores[waitCount]   = vkSemaphores[i]->pVkSemaphore;
                vkSemaphores[i]->signaled = false;
                waitCount++;
            }
        }

        uint32_t presentIndex = desc->index;
        VkPresentInfoKHR info{};
        info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.waitSemaphoreCount = waitCount;
        info.pWaitSemaphores    = ppSemaphores;
        info.swapchainCount     = 1;
        info.pSwapchains        = &S->pVkSwapchain;
        info.pImageIndices      = &presentIndex;
        info.pResults           = VK_NULL_HANDLE;

        VkResult rs = D->mVkDeviceTable.vkQueuePresentKHR(Q->pQueue, &info);
        if (rs != VK_SUCCESS && rs != VK_SUBOPTIMAL_KHR &&
            rs != VK_ERROR_OUT_OF_DATE_KHR)
        {
            assert(0 && "Present failed!");
        }
    }
}
