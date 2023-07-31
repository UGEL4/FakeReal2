#pragma once
#include "GpuConfig.h"
#include "Flags.h"

#define DEFINE_GPU_OBJECT(name) struct name##Descriptor; typedef const struct name *name##ID;

DEFINE_GPU_OBJECT(GPUSurface)
DEFINE_GPU_OBJECT(GPUInstance)
DEFINE_GPU_OBJECT(GPUAdapter)
DEFINE_GPU_OBJECT(GPUDevice)
DEFINE_GPU_OBJECT(GPUQueue)
DEFINE_GPU_OBJECT(GPUSwapchain)
DEFINE_GPU_OBJECT(GPUTexture)
DEFINE_GPU_OBJECT(GPUTextureView)
DEFINE_GPU_OBJECT(GPUShaderLibrary)
DEFINE_GPU_OBJECT(GPURootSignature)
DEFINE_GPU_OBJECT(GPURenderPipeline)
DEFINE_GPU_OBJECT(GPUSampler)
DEFINE_GPU_OBJECT(GPURootSignaturePool)
DEFINE_GPU_OBJECT(GPUCommandPool)
DEFINE_GPU_OBJECT(GPUCommandBuffer)
DEFINE_GPU_OBJECT(GPUFence)
DEFINE_GPU_OBJECT(GPUSemaphore)
DEFINE_GPU_OBJECT(GPUBuffer)
DEFINE_GPU_OBJECT(GPURenderPassEncoder)
DEFINE_GPU_OBJECT(GPUDescriptorSet)

#ifdef __cplusplus
extern "C" {
#endif

#define GPU_MAX_VERTEX_ATTRIBS 15
#define GPU_MAX_MRT_COUNT 8u

#define GPU_COLOR_MASK_RED 0x1
#define GPU_COLOR_MASK_GREEN 0x2
#define GPU_COLOR_MASK_BLUE 0x4
#define GPU_COLOR_MASK_ALPHA 0x8
#define GPU_COLOR_MASK_ALL GPU_COLOR_MASK_RED | GPU_COLOR_MASK_GREEN | GPU_COLOR_MASK_BLUE | GPU_COLOR_MASK_ALPHA
#define GPU_COLOR_MASK_NONE 0

// instance api
GPU_API GPUInstanceID GPUCreateInstance(const struct GPUInstanceDescriptor *pDesc);
typedef GPUInstanceID (*GPUProcCreateInstance)(const struct GPUInstanceDescriptor *pDesc);
GPU_API void GPUFreeInstance(GPUInstanceID instance);
typedef void (*GPUProcFreeInstance)(GPUInstanceID instance);

// adapter api
GPU_API void GPUEnumerateAdapters(GPUInstanceID pInstance, GPUAdapterID *const ppAdapters, uint32_t *adapterCount);
typedef void (*GPUProcEnumerateAdapters)(GPUInstanceID pInstance, GPUAdapterID *const ppAdapters, uint32_t *adapterCount);

// device api
GPU_API GPUDeviceID GPUCreateDevice(GPUAdapterID pAdapter, const struct GPUDeviceDescriptor *pDesc);
typedef GPUDeviceID (*GPUProcCreateDevice)(GPUAdapterID pAdapter, const struct GPUDeviceDescriptor *pDesc);
GPU_API void GPUFreeDevice(GPUDeviceID pDevice);
typedef void (*GPUProcFreeDevice)(GPUDeviceID pDevice);

// queue
GPU_API uint32_t GPUQueryQueueCount(const GPUAdapterID pAdapter, const EGPUQueueType queueType);
typedef uint32_t (*GPUProcQueryQueueCount)(const GPUAdapterID pAdapter, const EGPUQueueType queueType);
GPU_API GPUQueueID GPUGetQueue(GPUDeviceID pDevice, EGPUQueueType queueType, uint32_t queueIndex);
typedef GPUQueueID (*GPUProcGetQueue)(GPUDeviceID pDevice, EGPUQueueType queueType, uint32_t queueIndex);
GPU_API void GPUFreeQueue(GPUQueueID queue);
typedef void (*GPUProcFreeQueue)(GPUQueueID queue);
GPU_API void GPUSubmitQueue(GPUQueueID queue, const struct GPUQueueSubmitDescriptor *desc);
typedef void (*GPUProcSubmitQueue)(GPUQueueID queue, const struct GPUQueueSubmitDescriptor *desc);
GPU_API void GPUWaitQueueIdle(GPUQueueID queue);
typedef void (*GPUProcWaitQueueIdle)(GPUQueueID queue);
GPU_API void GPUQueuePresent(GPUQueueID queue, const struct GPUQueuePresentDescriptor *desc);
typedef void (*GPUProcQueuePresent)(GPUQueueID queue, const struct GPUQueuePresentDescriptor *desc);

// surface api
GPU_API GPUSurfaceID GPUCreateSurfaceFromNativeView(GPUInstanceID pInstance, void *view);
GPU_API void GPUFreeSurface(GPUInstanceID pInstance, GPUSurfaceID pSurface);
typedef void (*GPUProcFreeSurface)(GPUInstanceID pInstance, GPUSurfaceID pSurface);
#if defined(_WIN64)
    typedef struct HWND__ *HWND;
    GPU_API GPUSurfaceID GPUCreateSurfaceFromHWND(GPUInstanceID pInstance, HWND window);
    typedef GPUSurfaceID (*GPUProcCreateSurfaceFromHWND)(GPUInstanceID pInstance, HWND window);
#endif
typedef struct GPUSurfacesProcTable
{
    GPUProcFreeSurface FreeSurface;
#if defined(_WIN64)
    const GPUProcCreateSurfaceFromHWND CreateSurfaceFromHWND;
#endif
} GPUSurfacesProcTable;

// swapchain api
GPU_API GPUSwapchainID GPUCreateSwapchain(GPUDeviceID pDevice, const struct GPUSwapchainDescriptor *pDesc);
typedef GPUSwapchainID (*GPUProcCreateSwapchain)(GPUDeviceID pDevice, const struct GPUSwapchainDescriptor *pDesc);
GPU_API void GPUFreeSwapchain(GPUSwapchainID pSwapchain);
typedef void (*GPUProcFreeSwapchain)(GPUSwapchainID pSwapchain);
GPU_API uint32_t GPUAcquireNextImage(GPUSwapchainID swapchain, const struct GPUAcquireNextDescriptor *desc);
typedef uint32_t (*GPUProcAcquireNextImage)(GPUSwapchainID swapchain, const struct GPUAcquireNextDescriptor *desc);

// texture & texture_view api
GPU_API GPUTextureViewID GPUCreateTextureView(GPUDeviceID pDevice, const struct GPUTextureViewDescriptor *pDesc);
typedef GPUTextureViewID (*GPUProcCreateTextureView)(GPUDeviceID pDevice, const struct GPUTextureViewDescriptor *pDesc);
GPU_API void GPUFreeTextureView(GPUTextureViewID pTextureView);
typedef void (*GPUProcFreeTextureView)(GPUTextureViewID pTextureView);
GPU_API GPUTextureID GPUCreateTexture(GPUDeviceID device, const struct GPUTextureDescriptor *desc);
typedef GPUTextureID (*GPUProcCreateTexture)(GPUDeviceID device, const struct GPUTextureDescriptor *desc);
GPU_API void GPUFreeTexture(GPUTextureID texture);
typedef void (*GPUProcFreeTexture)(GPUTextureID texture);

// shader api
GPU_API GPUShaderLibraryID GPUCreateShaderLibrary(GPUDeviceID pDevice, const struct GPUShaderLibraryDescriptor *pDesc);
typedef GPUShaderLibraryID (*GPUProcCreateShaderLibrary)(GPUDeviceID pDevice, const struct GPUShaderLibraryDescriptor *pDesc);
GPU_API void GPUFreeShaderLibrary(GPUShaderLibraryID pShader);
typedef void (*GPUProcFreeShaderLibrary)(GPUShaderLibraryID pShader);

// root signature
GPU_API GPURootSignatureID GPUCreateRootSignature(GPUDeviceID device, const struct GPURootSignatureDescriptor *desc);
typedef GPURootSignatureID (*GPUProcCreateRootSignature)(GPUDeviceID device, const struct GPURootSignatureDescriptor *desc);
GPU_API void GPUFreeRootSignature(GPURootSignatureID RS);
typedef void (*GPUProcFreeRootSignature)(GPURootSignatureID RS);

// pipeline
GPU_API GPURenderPipelineID GPUCreateRenderPipeline(GPUDeviceID pDevice, const struct GPURenderPipelineDescriptor *pDesc);
typedef GPURenderPipelineID (*GPUProcCreateRenderPipeline)(GPUDeviceID pDevice, const struct GPURenderPipelineDescriptor *pDesc);
GPU_API void GPUFreeRenderPipeline(GPURenderPipelineID pPipeline);
typedef void (*GPUProcFreeRenderPipeline)(GPURenderPipelineID pPipeline);

// command
GPU_API GPUCommandPoolID GPUCreateCommandPool(GPUQueueID queue);
typedef GPUCommandPoolID (*GPUProcCreateCommandPool)(GPUQueueID queue);
GPU_API void GPUFreeCommandPool(GPUCommandPoolID pool);
typedef void (*GPUProcFreeCommandPool)(GPUCommandPoolID pool);
GPU_API void GPUResetCommandPool(GPUCommandPoolID pool);
typedef void (*GPUProcResetCommandPool)(GPUCommandPoolID pool);
GPU_API GPUCommandBufferID GPUCreateCommandBuffer(GPUCommandPoolID pool, const struct GPUCommandBufferDescriptor *desc);
typedef GPUCommandBufferID (*GPUProcCreateCommandBuffer)(GPUCommandPoolID pool, const struct GPUCommandBufferDescriptor *desc);
GPU_API void GPUFreeCommandBuffer(GPUCommandBufferID cmd);
typedef void (*GPUProcFreeCommandBuffer)(GPUCommandBufferID cmd);
GPU_API void GPUCmdBegin(GPUCommandBufferID cmdBuffer);
typedef void (*GPUProcCmdBegin)(GPUCommandBufferID cmdBuffer);
GPU_API void GPUCmdEnd(GPUCommandBufferID cmdBuffer);
typedef void (*GPUProcCmdEnd)(GPUCommandBufferID cmdBuffer);
GPU_API void GPUCmdResourceBarrier(GPUCommandBufferID cmd, const struct GPUResourceBarrierDescriptor *desc);
typedef void (*GPUProcCmdResourceBarrier)(GPUCommandBufferID cmd, const struct GPUResourceBarrierDescriptor *desc);
GPU_API void GPUCmdTransferBufferToTexture(GPUCommandBufferID cmd, const struct GPUBufferToTextureTransfer *desc);
typedef void (*GPUProcCmdTransferBufferToTexture)(GPUCommandBufferID cmd, const struct GPUBufferToTextureTransfer *desc);
GPU_API void GPUCmdTransferBufferToBuffer(GPUCommandBufferID cmd, const struct GPUBufferToBufferTransfer *desc);
typedef void (*GPUProcCmdTransferBufferToBuffer)(GPUCommandBufferID cmd, const struct GPUBufferToBufferTransfer *desc);
GPU_API void GPUCmdTransferTextureToTexture(GPUCommandBufferID cmd, const struct GPUTextureToTextureTransfer *desc);
typedef void (*GPUProcCmdTransferTextureToTexture)(GPUCommandBufferID cmd, const struct GPUTextureToTextureTransfer *desc);

// fence & semaphore
GPU_API GPUFenceID GPUCreateFence(GPUDeviceID device);
typedef GPUFenceID (*GPUProcCreateFence)(GPUDeviceID device);
GPU_API void GPUFreeFence(GPUFenceID fence);
typedef void (*GPUProcFreeFence)(GPUFenceID fence);
GPU_API void GPUWaitFences(const GPUFenceID *fences, uint32_t fenceCount);
typedef void (*GPUProcWaitFences)(const GPUFenceID *fences, uint32_t fenceCount);
GPU_API EGPUFenceStatus GPUQueryFenceStatus(GPUFenceID fence);
typedef EGPUFenceStatus (*GPUProcQueryFenceStatus)(GPUFenceID fence);
GPU_API GPUSemaphoreID GPUCreateSemaphore(GPUDeviceID device);
typedef GPUSemaphoreID (*GPUProcCreateSemaphore)(GPUDeviceID device);
GPU_API void GPUFreeSemaphore(GPUSemaphoreID semaphore);
typedef void (*GPUProcFreeSemaphore)(GPUSemaphoreID semaphore);

GPU_API GPURenderPassEncoderID GPUCmdBeginRenderPass(GPUCommandBufferID cmd, const struct GPURenderPassDescriptor *desc);
typedef GPURenderPassEncoderID (*GPUProcCmdBeginRenderPass)(GPUCommandBufferID cmd, const struct GPURenderPassDescriptor *desc);
GPU_API void GPUCmdEndRenderPass(GPUCommandBufferID cmd, GPURenderPassEncoderID encoder);
typedef void (*GPUProcCmdEndRenderPass)(GPUCommandBufferID cmd, GPURenderPassEncoderID encoder);
GPU_API void GPURenderEncoderSetViewport(GPURenderPassEncoderID encoder, float x, float y, float width, float height, float min_depth, float max_depth);
typedef void (*GPUProcRenderEncoderSetViewport)(GPURenderPassEncoderID encoder, float x, float y, float width, float height, float min_depth, float max_depth);
GPU_API void GPURenderEncoderSetScissor(GPURenderPassEncoderID encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
typedef void (*GPUProcRenderEncoderSetScissor)(GPURenderPassEncoderID encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
GPU_API void GPURenderEncoderBindPipeline(GPURenderPassEncoderID encoder, GPURenderPipelineID pipeline);
typedef void (*GPUProcRenderEncoderBindPipeline)(GPURenderPassEncoderID encoder, GPURenderPipelineID pipeline);
GPU_API void GPURenderEncoderDraw(GPURenderPassEncoderID encoder, uint32_t vertex_count, uint32_t first_vertex);
typedef void (*GPUProcRenderEncoderDraw)(GPURenderPassEncoderID encoder, uint32_t vertex_count, uint32_t first_vertex);
GPU_API void GPURenderEncoderDrawIndexed(GPURenderPassEncoderID encoder, uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset);
typedef void (*GPUProcRenderEncoderDrawIndexed)(GPURenderPassEncoderID encoder, uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset);
GPU_API void GPURenderEncoderDrawIndexedInstanced(GPURenderPassEncoderID encoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
typedef void (*GPUProcRenderEncoderDrawIndexedInstanced)(GPURenderPassEncoderID encoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
GPU_API void GPURenderEncoderBindVertexBuffers(GPURenderPassEncoderID encoder, uint32_t buffer_count, const GPUBufferID *buffers, const uint32_t *strides, const uint32_t *offsets);
typedef void (*GPUProcRenderEncoderBindVertexBuffers)(GPURenderPassEncoderID encoder, uint32_t buffer_count, const GPUBufferID *buffers, const uint32_t *strides, const uint32_t *offsets);
GPU_API void GPURenderEncoderBindIndexBuffer(GPURenderPassEncoderID encoder, GPUBufferID buffer, uint32_t offset, uint64_t indexStride);
typedef void (*GPUProcRenderEncoderBindIndexBuffer)(GPURenderPassEncoderID encoder, GPUBufferID buffer, uint32_t offset, uint64_t indexStride);
GPU_API void GPURenderEncoderBindDescriptorSet(GPURenderPassEncoderID encoder, GPUDescriptorSetID set);
typedef void (*GPUProcRenderEncoderBindDescriptorSet)(GPURenderPassEncoderID encoder, GPUDescriptorSetID set);

// buffer
GPU_API GPUBufferID GPUCreateBuffer(GPUDeviceID device, const struct GPUBufferDescriptor *desc);
typedef GPUBufferID (*GPUProcCreateBuffer)(GPUDeviceID device, const struct GPUBufferDescriptor *desc);
GPU_API void GPUFreeBuffer(GPUBufferID buffer);
typedef void (*GPUProcFreeBuffer)(GPUBufferID buffer);
GPU_API void GPUMapBuffer(GPUBufferID buffer, const struct GPUBufferRange* range);
typedef void (*GPUProcMapBufer)(GPUBufferID buffer, const struct GPUBufferRange* range);
GPU_API void GPUUnmapBuffer(GPUBufferID buffer);
typedef void (*GPUProcUnmapBuffer)(GPUBufferID buffer);

// sampler
GPU_API GPUSamplerID GPUCreateSampler(GPUDeviceID device, const struct GPUSamplerDescriptor *desc);
typedef GPUSamplerID (*GPUProcCreateSampler)(GPUDeviceID device, const struct GPUSamplerDescriptor *desc);
GPU_API void GPUFreeSampler(GPUSamplerID sampler);
typedef void (*GPUProcFreeSampler)(GPUSamplerID sampler);

GPU_API GPUDescriptorSetID GPUCreateDescriptorSet(GPUDeviceID device, const struct GPUDescriptorSetDescriptor *desc);
typedef GPUDescriptorSetID (*GPUProcCreateDescriptorSet)(GPUDeviceID device, const struct GPUDescriptorSetDescriptor *desc);
GPU_API void GPUFreeDescriptorSet(GPUDescriptorSetID set);
typedef void (*GPUProcFreeDescriptorSet)(GPUDescriptorSetID set);
GPU_API void GPUUpdateDescriptorSet(GPUDescriptorSetID set, const struct GPUDescriptorData *datas, uint32_t count);
typedef void (*GPUProcUpdateDescriptorSet)(GPUDescriptorSetID set, const struct GPUDescriptorData *datas, uint32_t count);

typedef struct GPUProcTable {
    // instance api
    const GPUProcCreateInstance CreateInstance;
    const GPUProcFreeInstance FreeInstance;

    // adapter api
    const GPUProcEnumerateAdapters EnumerateAdapters;

    // device api
    const GPUProcCreateDevice CreateDevice;
    const GPUProcFreeDevice FreeDevice;

    // queue
    const GPUProcQueryQueueCount QueryQueueCount;
    const GPUProcGetQueue GetQueue;
    const GPUProcFreeQueue FreeQueue;
    const GPUProcSubmitQueue SubmitQueue;
    const GPUProcWaitQueueIdle WaitQueueIdle;
    const GPUProcQueuePresent QueuePresent;

    // swapchain api
    const GPUProcCreateSwapchain CreateSwapchain;
    const GPUProcFreeSwapchain FreeSwapchain;
    const GPUProcAcquireNextImage AcquireNextImage;

    // texture & texture_view api
    const GPUProcCreateTextureView CreateTextureView;
    const GPUProcFreeTextureView FreeTextureView;
    const GPUProcCreateTexture CreateTexture;
    const GPUProcFreeTexture FreeTexture;

    // shader
    const GPUProcCreateShaderLibrary CreateShaderLibrary;
    const GPUProcFreeShaderLibrary FreeShaderLibrary;

    //rs
    const GPUProcCreateRootSignature CreateRootSignature;
    const GPUProcFreeRootSignature FreeRootSignature;

    // pipeline
    const GPUProcCreateRenderPipeline CreateRenderPipeline;
    const GPUProcFreeRenderPipeline FreeRenderPipeline;

    // command
    const GPUProcCreateCommandPool CreateCommandPool;
    const GPUProcFreeCommandPool FreeCommandPool;
    const GPUProcResetCommandPool ResetCommandPool;
    const GPUProcCreateCommandBuffer CreateCommandBuffer;
    const GPUProcFreeCommandBuffer FreeCommandBuffer;
    const GPUProcCmdBegin CmdBegin;
    const GPUProcCmdEnd CmdEnd;
    const GPUProcCmdResourceBarrier CmdResourceBarrier;
    const GPUProcCmdTransferBufferToTexture CmdTransferBufferToTexture;
    const GPUProcCmdTransferBufferToBuffer CmdTransferBufferToBuffer;
    const GPUProcCmdTransferTextureToTexture CmdTransferTextureToTexture;

    // fence & semaphore
    const GPUProcCreateFence CreateFence;
    const GPUProcFreeFence FreeFence;
    const GPUProcWaitFences WaitFences;
    const GPUProcQueryFenceStatus QueryFenceStatus;
    const GPUProcCreateSemaphore GpuCreateSemaphore;
    const GPUProcFreeSemaphore GpuFreeSemaphore;

    const GPUProcCmdBeginRenderPass CmdBeginRenderPass;
    const GPUProcCmdEndRenderPass CmdEndRenderPass;
    const GPUProcRenderEncoderSetViewport RenderEncoderSetViewport;
    const GPUProcRenderEncoderSetScissor RenderEncoderSetScissor;
    const GPUProcRenderEncoderBindPipeline RenderEncoderBindPipeline;
    const GPUProcRenderEncoderDraw RenderEncoderDraw;
    const GPUProcRenderEncoderDrawIndexed RenderEncoderDrawIndexed;
    const GPUProcRenderEncoderDrawIndexedInstanced RenderEncoderDrawIndexedInstanced;
    const GPUProcRenderEncoderBindVertexBuffers RenderEncoderBindVertexBuffers;
    const GPUProcRenderEncoderBindIndexBuffer RenderEncoderBindIndexBuffer;
    const GPUProcRenderEncoderBindDescriptorSet RenderEncoderBindDescriptorSet;

    // buffer
    const GPUProcCreateBuffer CreateBuffer;
    const GPUProcFreeBuffer FreeBuffer;
    const GPUProcMapBufer MapBuffer;
    const GPUProcUnmapBuffer UnmapBuffer;

    // sampler
    const GPUProcCreateSampler CreateSampler;
    const GPUProcFreeSampler FreeSampler;

    const GPUProcCreateDescriptorSet CreateDescriptorSet;
    const GPUProcFreeDescriptorSet FreeDescriptorSet;
    const GPUProcUpdateDescriptorSet UpdateDescriptorSet;
} GPUProcTable;

typedef struct CGPUChainedDescriptor
{
    EGPUBackend backend;
} CGPUChainedDescriptor;

typedef struct GPUInstanceDescriptor
{
    const CGPUChainedDescriptor* pChained;
    EGPUBackend backend;
    bool enableDebugLayer;
    bool enableValidation;
} GPUInstanceDescriptor;

typedef struct GPUInstance
{
    const GPUProcTable* pProcTable;
    const GPUSurfacesProcTable* pSurfaceProcTable;

    EGPUBackend backend;
} GPUInstance;

typedef struct GPUFormatSupport
{
    uint8_t shader_read : 1;
    uint8_t shader_write : 1;
    uint8_t render_target_write : 1;
} GPUFormatSupport;

typedef struct GPUAdapterDetail
{
    GPUFormatSupport format_supports[GPU_FORMAT_COUNT];
} GPUAdapterDetail;

typedef struct GPUAdapter
{
    GPUInstanceID pInstance;
    const GPUProcTable* pProcTableCache;
} GPUAdapter;

typedef struct GPUQueueGroupDescriptor
{
    EGPUQueueType queueType;
    uint32_t queueCount;
} GPUQueueGroupDescriptor;

typedef struct GPUDeviceDescriptor
{
    GPUQueueGroupDescriptor* pQueueGroup;
    uint32_t queueGroupCount;
    bool disablePipelineCache;
} GPUDeviceDescriptor;

typedef struct GPUDevice
{
    const GPUAdapterID pAdapter;
    const GPUProcTable* pProcTableCache;
    uint64_t nextTextureId;
} GPUDevice;

typedef struct GPUQueueSubmitDescriptor
{
    GPUCommandBufferID* cmds;
    GPUFenceID signal_fence;
    GPUSemaphoreID* wait_semaphores;
    GPUSemaphoreID* signal_semaphores;
    uint32_t cmds_count;
    uint32_t wait_semaphore_count;
    uint32_t signal_semaphore_count;
} GPUQueueSubmitDescriptor;

typedef struct GPUQueue
{
    GPUDeviceID pDevice;
    EGPUQueueType queueType;
    uint32_t queueIndex;
} GPUQueue;

typedef struct GPUFence
{
    GPUDeviceID device;
} GPUFence;

typedef struct GPUAcquireNextDescriptor
{
    GPUSemaphoreID signal_semaphore;
    GPUFenceID fence;
} GPUAcquireNextDescriptor;

typedef struct GPUSemaphore
{
    GPUDeviceID device;
} GPUSemaphore;

typedef struct GPUSwapchainDescriptor
{
    GPUQueueID* ppPresentQueues;
    uint32_t presentQueuesCount;
    GPUSurfaceID pSurface;
    EGPUFormat format;
    uint32_t width;
    uint32_t height;
    uint32_t imageCount;
    bool enableVSync;
} GPUSwapchainDescriptor;

typedef struct GPUSwapchain
{
    GPUDeviceID pDevice;
    const GPUTextureID* ppBackBuffers;
    uint32_t backBuffersCount;
} GPUSwapchain;

typedef struct GPUTextureDescriptor
{
    /// Texture creation flags (decides memory allocation strategy, sharing
    /// access,...)
    GPUTextureCreationFlags flags;
    /// Optimized clear value (recommended to use this same value when clearing
    /// the rendertarget)
    GPUClearValue clear_value;
    /// Width
    uint32_t width;
    /// Height
    uint32_t height;
    /// Depth (Should be 1 if not a mType is not TEXTURE_TYPE_3D)
    uint32_t depth;
    /// Texture array size (Should be 1 if texture is not a texture array or
    /// cubemap)
    uint32_t array_size;
    ///  image format
    EGPUFormat format;
    /// Number of mip levels
    uint32_t mip_levels;
    /// Number of multisamples per pixel (currently Textures created with mUsage
    /// TEXTURE_USAGE_SAMPLED_IMAGE only support CGPU_SAMPLE_COUNT_1)
    EGPUSampleCount sample_count;
    /// The image quality level. The higher the quality, the lower the
    /// performance. The valid range is between zero and the value appropriate for
    /// mSampleCount
    uint32_t sample_quality;
    /// Owner queue of the resource at creation
    GPUQueueID owner_queue;
    /// What state will the texture get created in
    EGPUResourceState start_state;
    /// Descriptor creation
    GPUResourceTypes descriptors;
    /// Memory Aliasing
    uint32_t is_dedicated;
    uint32_t is_aliasing;
} GPUTextureDescriptor;

typedef struct GPUTexture
{
    GPUDeviceID pDevice;
    uint64_t sizeInBytes;
    EGPUSampleCount sampleCount : 8;
    /// Current state of the buffer
    uint32_t width : 24;
    uint32_t height : 24;
    uint32_t depth : 12;
    uint32_t mipLevels : 6;
    uint32_t arraySizeMinusOne : 12;
    uint32_t format : 12;
    /// Flags specifying which aspects (COLOR,DEPTH,STENCIL) are included in the
    /// pVkImageView
    uint32_t aspectMask : 4;
    uint32_t nodeIndex : 4;
    uint32_t isCube : 1;
    uint32_t isDedicated : 1;
    /// This value will be false if the underlying resource is not owned by the
    /// texture (swapchain textures,...)
    uint32_t ownsImage : 1;
    /// In CGPU concept aliasing resource owns no memory
    uint32_t isAliasing : 1;
    uint32_t canAlias : 1;
    uint32_t isImported : 1;
    uint32_t canExport : 1;
    void* nativeHandle;
    uint64_t uniqueId;
} GPUTexture;

typedef struct GPUTextureViewDescriptor
{
    GPUTextureID pTexture;
    EGPUFormat format;
    EGPUTextureDimension dims;
    uint32_t usages;
    uint32_t aspectMask;
    uint32_t baseMipLevel;
    uint32_t mipLevelCount;
    uint32_t baseArrayLayer;
    uint32_t arrayLayerCount;
} GPUTextureViewDescriptor;

typedef struct GPUTextureView
{
    GPUDeviceID pDevice;
    GPUTextureViewDescriptor desc;
} GPUTextureView;

// Shaders
typedef struct GPUShaderLibraryDescriptor
{
    const char8_t* pName;
    const uint32_t* code;
    uint32_t codeSize;
    EGPUShaderStage stage;
    bool reflectionOnly;
} GPUShaderLibraryDescriptor;

typedef struct GPUShaderResource
{
    const char8_t* name;
    uint64_t name_hash;
    EGPUResourceType type;
    EGPUTextureDimension dim;
    uint32_t set;
    uint32_t binding;
    uint32_t size;
    uint32_t offset;
    GPUShaderStages stages;
} CGPUShaderResource;

typedef struct GPUShaderReflection
{
    const char8_t* entry_name;
    EGPUShaderStage stage;
    struct GPUVertexInput* vertex_inputs;
    GPUShaderResource* shader_resources;
    uint32_t vertex_inputs_count;
    uint32_t shader_resources_count;
    uint32_t thread_group_sizes[3];
} GPUShaderReflection;

typedef struct GPUShaderLibrary
{
    GPUDeviceID pDevice;
    char8_t* name;
    GPUShaderReflection* entry_reflections;
    uint32_t entrys_count;
} CGPUShaderLibrary;

typedef struct GPUShaderEntryDescriptor
{
    GPUShaderLibraryID pLibrary;
    const char8_t* entry;
    EGPUShaderStage stage;
    // ++ constant_specialization
    // const CGPUConstantSpecialization* constants;
    // uint32_t num_constants;
    // -- constant_specialization
} CGPUShaderEntryDescriptor;

typedef struct GPUVertexInput
{
    const char8_t* name;
    const char8_t* semantics;
    EGPUFormat format;
} GPUVertexInput;

typedef struct GPURootSignatureDescriptor
{
    struct GPUShaderEntryDescriptor* shaders;
    uint32_t shader_count;
    const GPUSamplerID* static_samplers;
    const char8_t* const* static_sampler_names;
    uint32_t static_sampler_count;
    const char8_t* const* push_constant_names;
    uint32_t push_constant_count;
    GPURootSignaturePoolID pool;

} GPURootSignatureDescriptor;

typedef struct GPURootSignaturePool
{
    GPUDeviceID device;
    EGPUPipelineType pipeline_type;
} GPURootSignaturePool;

typedef struct GPUParameterTable
{
    // This should be stored here because shader could be destoryed after RS
    // creation
    GPUShaderResource* resources;
    uint32_t resources_count;
    uint32_t set_index;
} GPUParameterTable;

typedef struct GPURootSignature
{
    GPUDeviceID device;
    GPUParameterTable* tables;
    uint32_t table_count;
    CGPUShaderResource* push_constants;
    uint32_t push_constant_count;
    CGPUShaderResource* static_samplers;
    uint32_t static_sampler_count;
    EGPUPipelineType pipeline_type;
    GPURootSignaturePoolID pool;
    GPURootSignatureID pool_sig;
} GPURootSignature;

typedef struct GPUSamplerDescriptor
{
    EGPUFilterType min_filter;
    EGPUFilterType mag_filter;
    EGPUMipMapMode mipmap_mode;
    EGPUAddressMode address_u;
    EGPUAddressMode address_v;
    EGPUAddressMode address_w;
    float mip_lod_bias;
    float max_anisotropy;
    EGPUCompareMode compare_func;
} GPUSamplerDescriptor;

typedef struct GPUSampler
{
    GPUDeviceID device;
} GPUSampler;

typedef struct GPUDepthStateDesc
{
    bool depthTest;
    bool depthWrite;
    EGPUCompareMode depthFunc;
    bool stencilTest;
    uint8_t stencilReadMask;
    uint8_t stencilWriteMask;
    EGPUCompareMode stencilFrontFunc;
    EGPUStencilOp stencilFrontFail;
    EGPUStencilOp depthFrontFail;
    EGPUStencilOp stencilFrontPass;
    EGPUCompareMode stencilBackFunc;
    EGPUStencilOp stencilBackFail;
    EGPUStencilOp depthBackFail;
    EGPUStencilOp stencilBackPass;
} GPUDepthStateDesc;

typedef struct GPURasterizerStateDescriptor
{
    EGPUCullMode cullMode;
    EGPUFillMode fillMode;
    EGPUFrontFace frontFace;
    int32_t depthBias;
    float slopeScaledDepthBias;
    bool enableMultiSample;
    bool enableScissor;
    bool enableDepthClamp;
} GPURasterizerStateDescriptor;

typedef struct GPUBlendStateDescriptor
{
    /// Source blend factor per render target.
    EGPUBlendConstant srcFactors[GPU_MAX_MRT_COUNT];
    /// Destination blend factor per render target.
    EGPUBlendConstant dstFactors[GPU_MAX_MRT_COUNT];
    /// Source alpha blend factor per render target.
    EGPUBlendConstant srcAlphaFactors[GPU_MAX_MRT_COUNT];
    /// Destination alpha blend factor per render target.
    EGPUBlendConstant dstAlphaFactors[GPU_MAX_MRT_COUNT];
    /// Blend mode per render target.
    EGPUBlendMode blendModes[GPU_MAX_MRT_COUNT];
    /// Alpha blend mode per render target.
    EGPUBlendMode blendAlphaModes[GPU_MAX_MRT_COUNT];
    /// Write mask per render target.
    int32_t masks[GPU_MAX_MRT_COUNT];
    /// Set whether alpha to coverage should be enabled.
    bool alphaTCoverage;
    /// Set whether each render target has an unique blend function. When false
    /// the blend function in slot 0 will be used for all render targets.
    bool independentBlend;
} GPUBlendStateDescriptor;

typedef struct GPUVertexAttribute
{
    uint32_t arraySize;
    EGPUFormat format;
    uint32_t binding;
    uint32_t offset;
    uint32_t stride;
    EGPUVertexInputRate rate;
} GPUVertexAttribute;

typedef struct GPUVertexLayout
{
    uint32_t attributeCount;
    GPUVertexAttribute attributes[GPU_MAX_VERTEX_ATTRIBS];
} GPUVertexLayout;

typedef struct GPURenderPipelineDescriptor
{
    GPURootSignatureID pRootSignature;
    const GPUShaderEntryDescriptor* pVertexShader;
    const GPUShaderEntryDescriptor* pFragmentShader;
    const GPUVertexLayout* pVertexLayout;
    const GPUDepthStateDesc* pDepthState;
    const GPURasterizerStateDescriptor* pRasterizerState;
    const GPUBlendStateDescriptor* pBlendState;

    EGPUSampleCount samplerCount;
    EGPUPrimitiveTopology primitiveTopology;

    EGPUFormat* pColorFormats;
    uint32_t renderTargetCount;
    EGPUFormat depthStencilFormat;
} GPURenderPipelineDescriptor;

typedef struct GPURenderPipeline
{
    GPUDeviceID pDevice;
    GPURootSignatureID pRootSignature;
} GPURenderPipeline;

typedef struct GPUCommandPool
{
    GPUQueueID queue;
} GPUCommandPool;

typedef struct GPUCommandBufferDescriptor
{
    bool isSecondary : 1;
} GPUCommandBufferDescriptor;

typedef struct GPUCommandBuffer
{
    GPUDeviceID device;
    GPUCommandPoolID pool;
    EGPUPipelineType currentDispatch;
} GPUCommandBuffer;

typedef struct GPUTextureSubresource
{
    GPUTextureViewAspects aspects;
    uint32_t mip_level;
    uint32_t base_array_layer;
    uint32_t layer_count;
} GPUTextureSubresource;

typedef struct GPUBufferToTextureTransfer
{
    GPUTextureID dst;
    GPUTextureSubresource dst_subresource;
    GPUBufferID src;
    uint64_t src_offset;
} GPUBufferToTextureTransfer;

typedef struct GPUTextureToTextureTransfer
{
    GPUTextureID src;
    GPUTextureID dst;
    GPUTextureSubresource src_subresource;
    GPUTextureSubresource dst_subresource;
} GPUTextureToTextureTransfer;

typedef struct GPUBufferToBufferTransfer
{
    GPUBufferID dst;
    uint64_t dst_offset;
    GPUBufferID src;
    uint64_t src_offset;
    uint64_t size;
} GPUBufferToBufferTransfer;

typedef struct GPUColorAttachment
{
    GPUTextureViewID view;
    GPUTextureViewID resolve_view;
    EGPULoadAction load_action;
    EGPUStoreAction store_action;
    GPUClearValue clear_color;
} GPUColorAttachment;

typedef struct GPUDepthStencilAttachment
{
    GPUTextureViewID view;
    EGPULoadAction depth_load_action;
    EGPUStoreAction depth_store_action;
    float clear_depth;
    uint8_t write_depth;
    EGPULoadAction stencil_load_action;
    EGPUStoreAction stencil_store_action;
    uint32_t clear_stencil;
    uint8_t write_stencil;
} GPUDepthStencilAttachment;

typedef struct GPURenderPassDescriptor
{
    const char* name;
    // TODO: support multi-target & remove this
    EGPUSampleCount sample_count;
    const GPUColorAttachment* color_attachments;
    const GPUDepthStencilAttachment* depth_stencil;
    uint32_t render_target_count;
} GPURenderPassDescriptor;

typedef struct GPURenderPassEncoder
{
    GPUDeviceID device;
} GPURenderPassEncoder;

typedef struct GPUBufferRange
{
    uint64_t offset;
    uint64_t size;
} GPUBufferRange;

typedef struct GPUBufferDescriptor
{
    uint64_t size;
    GPUResourceTypes descriptors; // bffer usage
    /// Memory usage
    /// Decides which memory heap buffer will use (default, upload, readback)
    EGPUMemoryUsage memory_usage;
    EGPUFormat format;
    EGPUResourceState start_state; /// What state will the buffer get created in
    GPUBufferCreationFlags flags;
    GPUQueueID owner_queue; /// Owner queue of the resource at creation
    bool prefer_on_device;
    bool prefer_on_host;
} GPUBufferDescriptor;

typedef struct GPUBuffer
{
    GPUDeviceID device;
    /**
     * CPU address of the mapped buffer.
     * Applicable to buffers created in CPU accessible heaps (CPU, CPU_TO_GPU,
     * GPU_TO_CPU)
     */
    void* cpu_mapped_address;
    uint64_t size : 37;
    uint64_t descriptors : 24;
    uint64_t memory_usage : 3;
} GPUBuffer;

typedef struct GPUTextureBarrier
{
    GPUTextureID texture;
    EGPUResourceState src_state;
    EGPUResourceState dst_state;
    uint8_t queue_acquire;
    uint8_t queue_release;
    EGPUQueueType queue_type;
    /// Specifiy whether following barrier targets particular subresource
    uint8_t subresource_barrier;
    /// Following values are ignored if subresource_barrier is false
    uint8_t mip_level;
    uint16_t array_layer;
    uint8_t d3d12_begin_only;
    uint8_t d3d12_end_only;
} GPUTextureBarrier;

typedef struct GPUBufferBarrier
{
    GPUBufferID buffer;
    EGPUResourceState src_state;
    EGPUResourceState dst_state;
    uint8_t queue_acquire;
    uint8_t queue_release;
    EGPUQueueType queue_type;
    uint8_t d3d12_begin_only;
    uint8_t d3d12_end_only;
} GPUBufferBarrier;

typedef struct GPUResourceBarrierDescriptor
{
    const GPUBufferBarrier* buffer_barriers;
    uint32_t buffer_barriers_count;
    const GPUTextureBarrier* texture_barriers;
    uint32_t texture_barriers_count;
} GPUResourceBarrierDescriptor;

typedef struct GPUQueuePresentDescriptor
{
    GPUSwapchainID swapchain;
    const GPUSemaphoreID* wait_semaphores;
    uint32_t wait_semaphore_count;
    uint8_t index;
} GPUQueuePresentDescriptor;

typedef struct GPUDescriptorSetDescriptor
{
    GPURootSignatureID root_signature;
    uint32_t set_index;
} GPUDescriptorSetDescriptor;

typedef struct GPUDescriptorSet
{
    GPURootSignatureID root_signature;
    uint32_t index;
} GPUDescriptorSet;

typedef struct GPUDescriptorData
{
    // Update Via Shader Reflection.
    const char8_t* name;
    // Update Via Binding Slot.
    uint32_t binding;
    EGPUResourceType binding_type;
    union
    {
        struct
        {
            /// Offset to bind the buffer descriptor
            const uint64_t* offsets;
            const uint64_t* sizes;
        } buffers_params;
        // Descriptor set buffer extraction options
        // TODO: Support descriptor buffer extraction
        // struct
        //{
        //    struct CGPUShaderEntryDescriptor* shader;
        //    uint32_t buffer_index;
        //    ECGPUShaderStage shader_stage;
        //} extraction_params;
        struct
        {
            uint32_t uav_mip_slice;
            bool blend_mip_chain;
        } uav_params;
        bool enable_stencil_resource;
    };
    union
    {
        const void** ptrs;
        /// Array of texture descriptors (srv and uav textures)
        GPUTextureViewID* textures;
        /// Array of sampler descriptors
        GPUSamplerID* samplers;
        /// Array of buffer descriptors (srv, uav and cbv buffers)
        GPUBufferID* buffers;
        /// Array of pipeline descriptors
        GPURenderPipelineID* render_pipelines;
        /// Array of pipeline descriptors
        // GPUComputePipelineID* compute_pipelines;
        /// DescriptorSet buffer extraction
        GPUDescriptorSetID* descriptor_sets;
        /// Custom binding (raytracing acceleration structure ...)
        // CGPUAccelerationStructureId* acceleration_structures;
    };
    uint32_t count;
} GPUDescriptorData;

#ifdef __cplusplus
}
#endif // extern "C"