#pragma once
#include "Gpu/GpuApi.h"

#if defined(_WIN64)
    #define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "volk.h"
#include "vma/vk_mem_alloc.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus extern "C" {

#define GLOBAL_VkAllocationCallbacks VK_NULL_HANDLE

#ifndef VK_USE_VOLK_DEVICE_TABLE
    #define VK_USE_VOLK_DEVICE_TABLE
#endif

#define MAX_PLANE_COUNT 3

const GPUProcTable* GPUVulkanProcTable();
const GPUSurfacesProcTable* GPUVulkanSurfacesTable();

// instance api
GPU_API GPUInstanceID GPUCreateInstance_Vulkan(const GPUInstanceDescriptor* pDesc);
GPU_API void GPUFreeInstance_Vllkan(GPUInstanceID pInstance);

// adapter
GPU_API void GPUEnumerateAdapters_Vulkan(GPUInstanceID pInstance, GPUAdapterID* const ppAdapters, uint32_t* adapterCount);

// device api
GPU_API GPUDeviceID GPUCreateDevice_Vulkan(GPUAdapterID pAdapter, const GPUDeviceDescriptor* pDesc);
GPU_API void GPUFreeDevice_Vulkan(GPUDeviceID pDevice);

// queue
GPU_API uint32_t GPUQueryQueueCount_Vulkan(const GPUAdapterID pAdapter, const EGPUQueueType queueType);
GPU_API GPUQueueID GPUGetQueue_Vulkan(GPUDeviceID pDevice, EGPUQueueType queueType, uint32_t queueIndex);
GPU_API void GPUFreeQueue_Vulkan(GPUQueueID queue);
GPU_API void GPUSubmitQueue_Vulkan(GPUQueueID queue, const struct GPUQueueSubmitDescriptor* desc);
GPU_API void GPUWaitQueueIdle_Vulkan(GPUQueueID queue);
GPU_API void GPUQueuePresent_Vulkan(GPUQueueID queue, const struct GPUQueuePresentDescriptor* desc);

// swapchain
GPU_API GPUSwapchainID GPUCreateSwapchain_Vulkan(GPUDeviceID pDevice, const struct GPUSwapchainDescriptor* pDesc);
GPU_API void GPUFreeSwapchain_Vulkan(GPUSwapchainID pSwapchain);
GPU_API uint32_t GPUAcquireNextImage_Vulkan(GPUSwapchainID swapchain, const struct GPUAcquireNextDescriptor* desc);

// texture & texture_view api
GPU_API GPUTextureViewID GPUCreateTextureView_Vulkan(GPUDeviceID pDevice, const GPUTextureViewDescriptor* pDesc);
GPU_API void GPUFreeTextureView_Vulkan(GPUTextureViewID pTextureView);
GPU_API GPUTextureID GPUCreateTexture_Vulkan(GPUDeviceID device, const GPUTextureDescriptor* desc);
GPU_API void GPUFreeTexture_Vulkan(GPUTextureID texture);

// shader
GPU_API GPUShaderLibraryID GPUCreateShaderLibrary_Vulkan(GPUDeviceID pDevice, const GPUShaderLibraryDescriptor* pDesc);
GPU_API void GPUFreeShaderLibrary_Vulkan(GPUShaderLibraryID pShader);

// rootsignature
GPU_API GPURootSignatureID GPUCreateRootSignature_Vulkan(GPUDeviceID device, const struct GPURootSignatureDescriptor* desc);
GPU_API void GPUFreeRootSignature_Vulkan(GPURootSignatureID RS);

// pipeline
GPU_API GPURenderPipelineID GPUCreateRenderPipeline_Vulkan(GPUDeviceID pDevice, const GPURenderPipelineDescriptor* pDesc);
GPU_API void GPUFreeRenderPipeline_Vulkan(GPURenderPipelineID pPipeline);

// command
GPU_API GPUCommandPoolID GPUCreateCommandPool_Vulkan(GPUQueueID queue);
VkCommandPool AllocateTransientCommandPool(struct GPUDevice_Vulkan* D, GPUQueueID queue);
GPU_API void GPUFreeCommandPool_Vulkan(GPUCommandPoolID pool);
GPU_API void GPUResetCommandPool_Vulkan(GPUCommandPoolID pool);
GPU_API GPUCommandBufferID GPUCreateCommandBuffer_Vulkan(GPUCommandPoolID pool, const GPUCommandBufferDescriptor* desc);
GPU_API void GPUFreeCommandBuffer_Vulkan(GPUCommandBufferID cmd);
GPU_API void GPUCmdBegin_Vulkan(GPUCommandBufferID cmdBuffer);
GPU_API void GPUCmdEnd_Vulkan(GPUCommandBufferID cmdBuffer);
GPU_API void GPUCmdResourceBarrier_Vulkan(GPUCommandBufferID cmd, const GPUResourceBarrierDescriptor* desc);
GPU_API void GPUCmdTransferBufferToTexture_Vulkan(GPUCommandBufferID cmd, const struct GPUBufferToTextureTransfer* desc);
GPU_API void GPUCmdTransferBufferToBuffer_Vulkan(GPUCommandBufferID cmd, const struct GPUBufferToBufferTransfer* desc);
GPU_API void GPUCmdTransferTextureToTexture_Vulkan(GPUCommandBufferID cmd, const struct GPUTextureToTextureTransfer* desc);

// fence & semaphore
GPU_API GPUFenceID GPUCreateFence_Vulkan(GPUDeviceID device);
GPU_API void GPUFreeFence_Vulkan(GPUFenceID fence);
GPU_API void GPUWaitFences_Vulkan(const GPUFenceID* fences, uint32_t fenceCount);
GPU_API EGPUFenceStatus GPUQueryFenceStatus_Vulkan(GPUFenceID fence);
GPU_API GPUSemaphoreID GPUCreateSemaphore_Vulkan(GPUDeviceID device);
GPU_API void GPUFreeSemaphore_Vulkan(GPUSemaphoreID semaphore);

// render pass
GPU_API GPURenderPassEncoderID GPUCmdBeginRenderPass_Vulkan(GPUCommandBufferID cmd, const struct GPURenderPassDescriptor* desc);
GPU_API void GPUCmdEndRenderPass_Vulkan(GPUCommandBufferID cmd, GPURenderPassEncoderID encoder);
GPU_API void GPURenderEncoderSetViewport_Vulkan(GPURenderPassEncoderID encoder, float x, float y, float width, float height, float min_depth, float max_depth);
GPU_API void GPURenderEncoderSetScissor_Vulkan(GPURenderPassEncoderID encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
GPU_API void GPURenderEncoderBindPipeline_Vulkan(GPURenderPassEncoderID encoder, GPURenderPipelineID pipeline);
GPU_API void GPURenderEncoderDraw_Vulkan(GPURenderPassEncoderID encoder, uint32_t vertex_count, uint32_t first_vertex);
GPU_API void GPURenderEncoderDrawIndexed_Vulkan(GPURenderPassEncoderID encoder, uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset);
GPU_API void GPURenderEncoderDrawIndexedInstanced_Vulkan(GPURenderPassEncoderID encoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
GPU_API void GPURenderEncoderBindVertexBuffers_Vulkan(GPURenderPassEncoderID encoder, uint32_t buffer_count, const GPUBufferID* buffers, const uint32_t* strides, const uint32_t* offsets);
GPU_API void GPURenderEncoderBindIndexBuffer_Vulkan(GPURenderPassEncoderID encoder, GPUBufferID buffer, uint32_t offset, uint64_t indexStride);
GPU_API void GPURenderEncoderBindDescriptorSet_Vulkan(GPURenderPassEncoderID encoder, GPUDescriptorSetID set, uint32_t dynamicOffsetCount = 0, const uint32_t* pDynamicOffsets = nullptr);
GPU_API void GPURenderEncoderPushConstant_Vulkan(GPURenderPassEncoderID encoder, GPURootSignatureID rs, void* data);

// buffer
GPU_API GPUBufferID GPUCreateBuffer_Vulkan(GPUDeviceID device, const GPUBufferDescriptor* desc);
GPU_API void GPUFreeBuffer_Vulkan(GPUBufferID buffer);
GPU_API void GPUMapBuffer_Vulkan(GPUBufferID buffer, const struct GPUBufferRange* range);
GPU_API void GPUUnmapBuffer_Vulkan(GPUBufferID buffer);

// sampler
GPU_API GPUSamplerID GPUCreateSampler_Vulkan(GPUDeviceID device, const struct GPUSamplerDescriptor* desc);
GPU_API void GPUFreeSampler_Vulkan(GPUSamplerID sampler);

// descriptor set
GPU_API GPUDescriptorSetID GPUCreateDescriptorSet_Vulkan(GPUDeviceID device, const struct GPUDescriptorSetDescriptor* desc);
GPU_API void GPUFreeDescriptorSet_Vulkan(GPUDescriptorSetID set);
GPU_API void GPUUpdateDescriptorSet_Vulkan(GPUDescriptorSetID set, const GPUDescriptorData* datas, uint32_t count);

typedef struct GPUVulkanInstanceDescriptor
{
    EGPUBackend backend;
    // Additional Instance Layers
    const char** ppInstanceLayers;
    // Count of Additional Instance Layers
    uint32_t mInstanceLayerCount;
    // Additional Instance Extensions
    const char** ppInstanceExtensions;
    // Count of Additional Instance Extensions
    uint32_t mInstanceExtensionCount;
    // Addition Physical Device Extensions
    const char** ppDeviceExtensions;
    // Count of Addition Physical Device Extensions
    uint32_t mDeviceExtensionCount;
    const struct VkDebugUtilsMessengerCreateInfoEXT* pDebugUtilsMessenger;
    const struct VkDebugReportCallbackCreateInfoEXT* pDebugReportMessenger;
} GPUVulkanInstanceDescriptor;

typedef struct GPUInstance_Vulkan
{
    GPUInstance super;
    VkInstance pInstance;
    VkDebugUtilsMessengerEXT pDebugUtils;

    // physica device
    struct GPUAdapter_Vulkan* pAdapters;
    uint32_t adapterCount;

    // layers
    uint32_t layersCount;
    VkLayerProperties* pLayerProperties;
    const char** pLayerNames;

    // extensions
    uint32_t extensionCount;
    VkExtensionProperties* pExtensonProperties;
    const char** pExtensionNames;

    uint32_t debugUtils : 1;
} GPUInstance_Vulkan;

typedef struct GPUAdapter_Vulkan
{
    GPUAdapter super;
    VkPhysicalDevice pPhysicalDevice;
    VkPhysicalDeviceProperties2KHR physicalDeviceProperties;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures;
    VkPhysicalDeviceSubgroupProperties subgroupProperties;

    VkQueueFamilyProperties* pQueueFamilyProperties;
    uint32_t queueFamiliesCount;
    int64_t queueFamilyIndices[GPU_QUEUE_TYPE_COUNT];

    uint32_t extensionsCount;
    const char** ppExtensionsName;
    VkExtensionProperties* pExtensionProps;

    GPUAdapterDetail adapterDetail;
} GPUAdapter_Vulkan;

typedef struct VmaAllocator_T* VmaAllocator;
typedef struct GPUDevice_Vulkan
{
    GPUDevice spuer;
    VkDevice pDevice;
    struct VulkanUtil_DescriptorPool* pDescriptorPool;
    struct VolkDeviceTable mVkDeviceTable;
    struct GPUVkPassTable* pPassTable;
    VmaAllocator pVmaAllocator;
} GPUDevice_Vulkan;

typedef struct GPUQueue_Vulkan
{
    const GPUQueue super;
    VkQueue pQueue;
    uint32_t queueFamilyIndex; // Cmd pool for inner usage like resource transition
    GPUCommandPoolID pInnerCmdPool;
    GPUCommandBufferID pInnerCmdBuffer;
    GPUFenceID pInnerFence;
} GPUQueue_Vulkan;

typedef struct GPUSwapchain_Vulkan
{
    GPUSwapchain super;
    VkSurfaceKHR pVkSurface;
    VkSwapchainKHR pVkSwapchain;
} GPUSwapchain_Vulkan;

typedef struct GPUTexture_Vulkan
{
    GPUTexture super;
    VkImage pVkImage;
    union
    {
        VmaAllocation pVkAllocation;
        VkDeviceMemory pVkDeviceMemory;
    };
} GPUTexture_Vulkan;

typedef struct GPUTextureView_Vulkan
{
    GPUTextureView super;
    VkImageView pVkRTVDSVDescriptor;
    VkImageView pVkSRVDescriptor;
    VkImageView pVkUAVDescriptor;
} GPUTextureView_Vulkan;

typedef struct GPUShaderLibrary_Vulkan
{
    GPUShaderLibrary super;
    VkShaderModule pShader;
    struct SpvReflectShaderModule* pReflect;
} GPUShaderLibrary_Vulkan;

typedef struct SetLayout_Vulkan
{
    VkDescriptorSetLayout pLayout;
    VkDescriptorUpdateTemplate pUpdateTemplate;
    uint32_t updateEntriesCount;
    VkDescriptorSet pEmptyDescSet;
} SetLayout_Vulkan;

typedef struct GPURootSignature_Vulkan
{
    GPURootSignature super;
    VkPipelineLayout pPipelineLayout;
    SetLayout_Vulkan* pSetLayouts;
    VkDescriptorSetLayout* pVkSetLayouts;
    uint32_t setLayoutsCount;
    VkPushConstantRange* pPushConstantRanges;
} GPURootSignature_Vulkan;

typedef struct VulkanRenderPassDescriptor
{
    EGPUFormat pColorFormat[GPU_MAX_MRT_COUNT];
    EGPULoadAction pColorLoadOps[GPU_MAX_MRT_COUNT];
    EGPUStoreAction pColorStoreOps[GPU_MAX_MRT_COUNT];
    uint32_t attachmentCount;
    EGPUFormat depthFormat;
    EGPUSampleCount sampleCount;
    EGPULoadAction depthLoadOp;
    EGPUStoreAction depthStoreOp;
    EGPULoadAction stencilLoadOp;
    EGPUStoreAction stencilStoreOp;
} VulkanRenderPassDescriptor;

typedef struct GPURenderPipeline_Vulkan
{
    GPURenderPipeline super;
    VkPipeline pPipeline;
} GPURenderPipeline_Vulkan;

typedef struct GPUCommandPool_Vulkan
{
    GPUCommandPool super;
    VkCommandPool pPool;
} GPUCommandPool_Vulkan;

typedef struct GPUCommandBuffer_Vulkan
{
    GPUCommandBuffer super;
    VkCommandBuffer pVkCmd;
    VkPipelineLayout pLayout;
    VkRenderPass pPass;
    uint32_t type;
} GPUCommandBuffer_Vulkan;

typedef struct GPUBuffer_Vulkan
{
    GPUBuffer super;
    VkBuffer pVkBuffer;
    VkBufferView pVkStorageTexelView;
    VkBufferView pVkUniformTexelView;
    VmaAllocation pVkAllocation;
    uint64_t mOffset;
} GPUBuffer_Vulkan;

typedef struct GPUFence_Vulkan
{
    GPUFence super;
    VkFence pVkFence;
    uint32_t submitted : 1;
} GPUFence_Vulkan;

typedef struct GPUSemaphore_Vulkan
{
    GPUSemaphore super;
    VkSemaphore pVkSemaphore;
    uint8_t signaled : 1;
} GPUSemaphore_Vulkan;

typedef struct VulkanFramebufferDesriptor
{
    VkRenderPass pRenderPass;
    uint32_t attachmentCount;
    VkImageView pImageViews[GPU_MAX_MRT_COUNT + 1];
    uint32_t width;
    uint32_t height;
    uint32_t layers;
} VulkanFramebufferDesriptor;

typedef struct GPUSampler_Vulkan
{
    GPUSampler super;
    VkSampler pSampler;
} GPUSampler_Vulkan;

typedef struct GPUDescriptorSet_Vulkan
{
    GPUDescriptorSet super;
    VkDescriptorSet pSet;
    union VkDescriptorUpdateData* pUpdateData;
} GPUDescriptorSet_Vulkan;

typedef union VkDescriptorUpdateData
{
    VkDescriptorImageInfo mImageInfo;
    VkDescriptorBufferInfo mBufferInfo;
    VkBufferView pBuferView;
} VkDescriptorUpdateData;

#ifdef __cplusplus
}
#endif //__cplusplus end extern "C" }
