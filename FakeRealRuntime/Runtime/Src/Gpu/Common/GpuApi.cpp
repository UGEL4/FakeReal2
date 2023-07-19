#include "Gpu/GpuApi.h"

#ifdef GPU_USE_VULKAN
    #include "Gpu/Backend/Vulkan/GPUVulkan.h"
#endif

GPUInstanceID GPUCreateInstance(const GPUInstanceDescriptor* pDesc)
{
    assert(pDesc->backend == EGPUBackend::GPUBackend_Vulkan && "Not support GPU backend!");

    const GPUProcTable* pProcTable             = nullptr;
    const GPUSurfacesProcTable* pSurfacesTable = nullptr;

    if (pDesc->backend == GPUBackend_Count)
    {
    }
#ifdef GPU_USE_VULKAN
    else if (pDesc->backend == GPUBackend_Vulkan)
    {
        pProcTable     = GPUVulkanProcTable();
        pSurfacesTable = GPUVulkanSurfacesTable();
    }
#endif
#ifdef GPU_USE_D3D12
    else if (pDesc->backend == GPUBackend_D3D12)
    {
        pProcTable = GPUD3D12ProcTable();
    }
#endif

    GPUInstance* pInstance       = (GPUInstance*)pProcTable->CreateInstance(pDesc);
    pInstance->pProcTable        = pProcTable;
    pInstance->pSurfaceProcTable = pSurfacesTable;
    pInstance->backend           = pDesc->backend;

    return pInstance;
}

void GPUFreeInstance(GPUInstanceID instance)
{
    assert(instance->backend == EGPUBackend::GPUBackend_Vulkan);
    assert(instance->pProcTable->FreeInstance);

    instance->pProcTable->FreeInstance(instance);
}

void GPUEnumerateAdapters(GPUInstanceID pInstance, GPUAdapterID *const ppAdapters, uint32_t *adapterCount)
{
    assert(pInstance != NULL && "null gpu instance!");
    assert(pInstance->pProcTable->EnumerateAdapters != NULL && "GPUEnumerateAdapters not loaded!");

    pInstance->pProcTable->EnumerateAdapters(pInstance, ppAdapters, adapterCount);

    if (ppAdapters != NULL)
    {
        for (uint32_t i = 0; i < *adapterCount; i++)
        {
            *((const GPUProcTable**)&ppAdapters[i]->pProcTableCache) = pInstance->pProcTable;
            *((GPUInstanceID*)&ppAdapters[i]->pInstance)             = pInstance;
        }
    }
}

GPUDeviceID GPUCreateDevice(GPUAdapterID pAdapter, const struct GPUDeviceDescriptor *pDesc)
{
    assert(pAdapter->pProcTableCache && "No ProcTableCach!");
    assert(pAdapter->pProcTableCache->CreateDevice && "CreateDevice not loaded!");

    GPUDeviceID pDevice = pAdapter->pProcTableCache->CreateDevice(pAdapter, pDesc);
    if (pDevice != nullptr)
    {
        *(const GPUProcTable**)&pDevice->pProcTableCache = pAdapter->pProcTableCache;
    }
    ((GPUDevice*)pDevice)->nextTextureId = 0;
    return pDevice;
}

void GPUFreeDevice(GPUDeviceID pDevice)
{
    assert(pDevice->pProcTableCache && "No ProcTableCach!");
    assert(pDevice->pProcTableCache->FreeDevice && "FreeDevice not loaded!");
    pDevice->pProcTableCache->FreeDevice(pDevice);
}

GPUSurfaceID GPUCreateSurfaceFromNativeView(GPUInstanceID pInstance, void *view)
{
    #if defined(_WIN64)
    return GPUCreateSurfaceFromHWND(pInstance, (HWND)view);
#endif

    return nullptr;
}

void GPUFreeSurface(GPUInstanceID pInstance, GPUSurfaceID pSurface)
{
    assert(pInstance && "Null gpu instance!");
    assert(pInstance->pSurfaceProcTable && "Null SurfaceProcTabl!");
    assert(pInstance->pSurfaceProcTable->FreeSurface && "FreeSurface not loaded!");
    pInstance->pSurfaceProcTable->FreeSurface(pInstance, pSurface);
}

#if defined(_WIN64)
GPUSurfaceID GPUCreateSurfaceFromHWND(GPUInstanceID pInstance, HWND window)
{
    assert(pInstance && "Null gpu instance!");
    assert(pInstance->pSurfaceProcTable && "Null SurfaceProcTabl!");
    assert(pInstance->pSurfaceProcTable->CreateSurfaceFromHWND  && "CreateSurfaceFromHWND not loaded!");
    return pInstance->pSurfaceProcTable->CreateSurfaceFromHWND(pInstance, window);
}
#endif

GPUQueueID GPUGetQueue(GPUDeviceID pDevice, EGPUQueueType queueType, uint32_t queueIndex)
{
    assert(pDevice && "Null gpu dvice");
    assert(pDevice->pProcTableCache && "No ProcTableCach!");
    assert(pDevice->pProcTableCache->GetQueue && "GetQueue not loaded!");

    // TODO: try find queue
    // if (q) {warning();return q;}

    GPUQueue* pQueue   = (GPUQueue*)pDevice->pProcTableCache->GetQueue(pDevice, queueType, queueIndex);
    pQueue->pDevice    = pDevice;
    pQueue->queueType  = queueType;
    pQueue->queueIndex = queueIndex;
    return pQueue;
}

void GPUFreeQueue(GPUQueueID queue)
{
    assert(queue && "Null gpu queue");
    assert(queue->pDevice && "Null gpu dvice");
    assert(queue->pDevice->pProcTableCache->FreeQueue && "FreeQueue not loaded!");
    queue->pDevice->pProcTableCache->FreeQueue(queue);
}

void GPUSubmitQueue(GPUQueueID queue, const struct GPUQueueSubmitDescriptor *desc)
{
    assert(queue && "Null gpu queue");
    assert(queue->pDevice && "Null gpu dvice");
    assert(queue->pDevice->pProcTableCache->SubmitQueue && "SubmitQueue not loaded!");
    queue->pDevice->pProcTableCache->SubmitQueue(queue, desc);
}

void GPUWaitQueueIdle(GPUQueueID queue)
{
    assert(queue && "Null gpu queue");
    assert(queue->pDevice && "Null gpu dvice");
    assert(queue->pDevice->pProcTableCache->WaitQueueIdle && "WaitQueueIdle not loaded!");
    queue->pDevice->pProcTableCache->WaitQueueIdle(queue);
}

void GPUQueuePresent(GPUQueueID queue, const struct GPUQueuePresentDescriptor *desc)
{
    assert(queue && "Null gpu queue");
    assert(queue->pDevice && "Null gpu dvice");
    assert(queue->pDevice->pProcTableCache->QueuePresent && "QueuePresent not loaded!");
    queue->pDevice->pProcTableCache->QueuePresent(queue, desc);
}

GPUSwapchainID GPUCreateSwapchain(GPUDeviceID pDevice, const struct GPUSwapchainDescriptor *pDesc)
{
    assert(pDevice && "Null gpu dvice");
    assert(pDevice->pProcTableCache->CreateSwapchain && "CreateSwapchain not loaded!");

    GPUSwapchain* pSwapchain = (GPUSwapchain*)pDevice->pProcTableCache->CreateSwapchain(pDevice, pDesc);
    pSwapchain->pDevice      = pDevice;
    for (uint32_t i = 0; i < pSwapchain->backBuffersCount; i++)
    {
        ((GPUTexture*)pSwapchain->ppBackBuffers[i])->uniqueId = ((GPUDevice*)pDevice)->nextTextureId++;
    }

    return pSwapchain;
}

void GPUFreeSwapchain(GPUSwapchainID pSwapchain)
{
    assert(pSwapchain && "Null gpu swapchain");
    GPUDeviceID pDevice = pSwapchain->pDevice;
    assert(pDevice->pProcTableCache->FreeSwapchain && "FreeSwapchain not loaded!");
    pDevice->pProcTableCache->FreeSwapchain(pSwapchain);
}

uint32_t GPUAcquireNextImage(GPUSwapchainID swapchain, const struct GPUAcquireNextDescriptor *desc)
{
    assert(swapchain && "Null gpu swapchain");
    assert(swapchain->pDevice && "Null gpu device");
    assert(swapchain->pDevice->pProcTableCache->AcquireNextImage && "AcquireNextImage not loaded!");
    return swapchain->pDevice->pProcTableCache->AcquireNextImage(swapchain, desc);
}

GPUTextureViewID GPUCreateTextureView(GPUDeviceID pDevice, const struct GPUTextureViewDescriptor* pDesc)
{
    assert(pDevice && "Null gpu device");
    assert(pDesc && "Null texture view desc");
    assert(pDevice->pProcTableCache->CreateTextureView && "CreateTextureView not loaded!");

    GPUTextureView* pView = (GPUTextureView*)pDevice->pProcTableCache->CreateTextureView(pDevice, pDesc);
    pView->pDevice        = pDevice;
    pView->desc           = *pDesc;

    return pView;
}

void GPUFreeTextureView(GPUTextureViewID pTextureView)
{
    assert(pTextureView && "Null gpu texture view");
    assert(pTextureView->pDevice && "Null gpu device");
    assert(pTextureView->pDevice->pProcTableCache->FreeTextureView && "FreeTextureView not loaded!");
    pTextureView->pDevice->pProcTableCache->FreeTextureView(pTextureView);
}

GPUTextureID GPUCreateTexture(GPUDeviceID device, const GPUTextureDescriptor* desc)
{
    assert(device && "Null gpu device");
    assert(device->pProcTableCache->CreateTexture && "CreateTexture not loaded!");
    GPUTextureDescriptor new_desc{};
    memcpy(&new_desc, desc, sizeof(GPUTextureDescriptor));
    if (desc->array_size == 0) new_desc.array_size = 1;
    if (desc->mip_levels == 0) new_desc.mip_levels = 1;
    if (desc->depth == 0) new_desc.depth = 1;
    if (desc->sample_count == 0) new_desc.sample_count = (EGPUSampleCount)1;
    GPUTexture* T  = (GPUTexture*)device->pProcTableCache->CreateTexture(device, &new_desc);
    T->pDevice     = device;
    T->sampleCount = desc->sample_count;
    return T;
}

void GPUFreeTexture(GPUTextureID texture)
{
    assert(texture && "Null gpu texture");
    assert(texture->pDevice && "Null gpu device");
    assert(texture->pDevice->pProcTableCache->FreeTexture && "FreeTexture not loaded!");
    texture->pDevice->pProcTableCache->FreeTexture(texture);
}

GPUShaderLibraryID GPUCreateShaderLibrary(GPUDeviceID pDevice, const GPUShaderLibraryDescriptor* pDesc)
{
    assert(pDevice && "Null gpu device");
    assert(pDevice->pProcTableCache->CreateShaderLibrary && "CreateShaderLibrary not loaded!");
    GPUShaderLibrary* pShader = (GPUShaderLibrary*)pDevice->pProcTableCache->CreateShaderLibrary(pDevice, pDesc);
    pShader->pDevice          = pDevice;
    const size_t str_len      = strlen((const char*)pDesc->pName);
    const size_t str_size     = str_len + 1;
    pShader->name             = (char8_t*)calloc(1, str_size * sizeof(char8_t));
    memcpy((void*)pShader->name, pDesc->pName, str_size);
    return pShader;
}

void GPUFreeShaderLibrary(GPUShaderLibraryID pShader)
{
    assert(pShader && "Null gpu shader");
    assert(pShader->pDevice && "Null gpu device");
    assert(pShader->pDevice->pProcTableCache->FreeShaderLibrary && "FreeShaderLibrary not loaded!");
    free(pShader->name);
    pShader->pDevice->pProcTableCache->FreeShaderLibrary(pShader);
}

GPURootSignatureID GPUCreateRootSignature(GPUDeviceID device, const struct GPURootSignatureDescriptor* desc)
{
    GPURootSignature* pRST = (GPURootSignature*)device->pProcTableCache->CreateRootSignature(device, desc);
    pRST->device           = device;
    return pRST;
}

void GPUFreeRootSignature(GPURootSignatureID RS)
{
    assert(RS && "Null root signature!");
    assert(RS->device && "Null gpu device!");
    assert(RS->device->pProcTableCache->FreeRootSignature && "FreeRootSignature not loaded!");
    RS->device->pProcTableCache->FreeRootSignature(RS);
}

static const GPUDepthStateDesc sDefaultDepthState = {
    .depthTest  = false,
    .depthWrite = false,
    .stencilTest = false
};

static const GPURasterizerStateDescriptor sDefaultRasterizerState = {
    .cullMode = GPU_CULL_MODE_BACK,
    .fillMode = GPU_FILL_MODE_SOLID,
    .frontFace = GPU_FRONT_FACE_CCW,
    .depthBias = 0,
    .slopeScaledDepthBias = 0.f,
    .enableMultiSample = false,
    .enableScissor = false,
    .enableDepthClamp = false
};

GPURenderPipelineID GPUCreateRenderPipeline(GPUDeviceID pDevice, const GPURenderPipelineDescriptor* pDesc)
{
    static GPUBlendStateDescriptor sDefaulBlendState = {};
    sDefaulBlendState.srcFactors[0]           = GPU_BLEND_CONST_ONE;
    sDefaulBlendState.dstFactors[0]           = GPU_BLEND_CONST_ZERO;
    sDefaulBlendState.srcAlphaFactors[0]      = GPU_BLEND_CONST_ONE;
    sDefaulBlendState.dstAlphaFactors[0]      = GPU_BLEND_CONST_ZERO;
    sDefaulBlendState.blendModes[0]           = GPU_BLEND_MODE_ADD;
    sDefaulBlendState.masks[0]                = GPU_COLOR_MASK_ALL;
    sDefaulBlendState.independentBlend        = false;

    assert(pDevice && "Null gpu device!");
    assert(pDevice->pProcTableCache->CreateRenderPipeline && "CreateRenderPipeline not loaded!");
    GPURenderPipelineDescriptor newDesc{};
    memcpy(&newDesc, pDesc, sizeof(GPURenderPipelineDescriptor));
    if (pDesc->samplerCount == 0) newDesc.samplerCount = GPU_SAMPLE_COUNT_1;
    if (pDesc->pBlendState == NULL) newDesc.pBlendState = &sDefaulBlendState;
    if (pDesc->pDepthState == NULL) newDesc.pDepthState = &sDefaultDepthState;
    if (pDesc->pRasterizerState == NULL) newDesc.pRasterizerState = &sDefaultRasterizerState;
    GPURenderPipeline* pPipeline = NULL;
    pPipeline                    = (GPURenderPipeline*)pDevice->pProcTableCache->CreateRenderPipeline(pDevice, &newDesc);
    pPipeline->pDevice           = pDevice;
    pPipeline->pRootSignature    = pDesc->pRootSignature;
    return pPipeline;
}

void GPUFreeRenderPipeline(GPURenderPipelineID pPipeline)
{
    assert(pPipeline && "Null gpu pipeline!");
    assert(pPipeline->pDevice->pProcTableCache->FreeRenderPipeline && "FreeRenderPipeline not loaded!");
    pPipeline->pDevice->pProcTableCache->FreeRenderPipeline(pPipeline);
}

GPUCommandPoolID GPUCreateCommandPool(GPUQueueID queue)
{
    assert(queue && "Null gpu queue!");
    assert(queue->pDevice && "Null gpu device!");
    assert(queue->pDevice->pProcTableCache->CreateCommandPool && "CreateCommandPool not loaded!");
    GPUCommandPool* P = (GPUCommandPool*)queue->pDevice->pProcTableCache->CreateCommandPool(queue);
    P->queue          = queue;
    return P;
}

void GPUFreeCommandPool(GPUCommandPoolID pool)
{
    assert(pool && "Null gpu command pool!");
    assert(pool->queue && "Null gpu queue!");
    assert(pool->queue->pDevice && "Null gpu device!");
    assert(pool->queue->pDevice->pProcTableCache->FreeCommandPool && "FreeCommandPool not loaded!");
    pool->queue->pDevice->pProcTableCache->FreeCommandPool(pool);
}

void GPUResetCommandPool(GPUCommandPoolID pool)
{
    assert(pool && "Null gpu command pool!");
    assert(pool->queue && "Null gpu queue!");
    assert(pool->queue->pDevice && "Null gpu device!");
    assert(pool->queue->pDevice->pProcTableCache->ResetCommandPool && "ResetCommandPool not loaded!");
    pool->queue->pDevice->pProcTableCache->ResetCommandPool(pool);
}

GPUCommandBufferID GPUCreateCommandBuffer(GPUCommandPoolID pool, const GPUCommandBufferDescriptor* desc)
{
    assert(pool && "Null gpu command pool!");
    assert(pool->queue && "Null gpu queue!");
    assert(pool->queue->pDevice && "Null gpu device!");
    assert(pool->queue->pDevice->pProcTableCache->CreateCommandBuffer && "CreateCommandBuffer not loaded!");
    GPUCommandBuffer* CMD = (GPUCommandBuffer*)pool->queue->pDevice->pProcTableCache->CreateCommandBuffer(pool, desc);
    CMD->device           = pool->queue->pDevice;
    CMD->pool             = pool;
    return CMD;
}

void GPUFreeCommandBuffer(GPUCommandBufferID cmd)
{
    assert(cmd && "Null gpu commandbuffer!");
    assert(cmd->device && "Null gpu device!");
    assert(cmd->device->pProcTableCache->FreeCommandBuffer && "FreeCommandBuffer not loaded!");
    cmd->device->pProcTableCache->FreeCommandBuffer(cmd);
}

void GPUCmdBegin(GPUCommandBufferID cmdBuffer)
{
    assert(cmdBuffer && "Null gpu commandbuffer!");
    assert(cmdBuffer->device && "Null gpu device!");
    assert(cmdBuffer->device->pProcTableCache->CmdBegin && "CmdBegin not loaded!");
    cmdBuffer->device->pProcTableCache->CmdBegin(cmdBuffer);
}

void GPUCmdEnd(GPUCommandBufferID cmdBuffer)
{
    assert(cmdBuffer && "Null gpu commandbuffer!");
    assert(cmdBuffer->device && "Null gpu device!");
    assert(cmdBuffer->device->pProcTableCache->CmdEnd && "CmdEnd not loaded!");
    cmdBuffer->device->pProcTableCache->CmdEnd(cmdBuffer);
}

void GPUCmdResourceBarrier(GPUCommandBufferID cmd, const struct GPUResourceBarrierDescriptor* desc)
{
    assert(cmd && "Null gpu commandbuffer!");
    assert(cmd->device && "Null gpu device!");
    assert(cmd->device->pProcTableCache->CmdResourceBarrier && "CmdResourceBarrier not loaded!");
    cmd->device->pProcTableCache->CmdResourceBarrier(cmd, desc);
}

void GPUCmdTransferBufferToTexture(GPUCommandBufferID cmd, const struct GPUBufferToTextureTransfer* desc)
{
    assert(cmd && "Null gpu commandbuffer!");
    assert(cmd->device && "Null gpu device!");
    assert(cmd->device->pProcTableCache->CmdTransferBufferToTexture && "CmdTransferBufferToTexture not loaded!");
    assert(desc->src != nullptr && "Null src cmdbuffer");
    assert(desc->dst != nullptr && "Null dst cmdbuffer");
    assert(cmd->currentDispatch == GPU_PIPELINE_TYPE_NONE && "fatal: can't call transfer apis on commdn buffer while preparing dispatching!");
    cmd->device->pProcTableCache->CmdTransferBufferToTexture(cmd, desc);
}

void GPUCmdTransferBufferToBuffer(GPUCommandBufferID cmd, const struct GPUBufferToBufferTransfer* desc)
{
    assert(cmd && "Null gpu commandbuffer!");
    assert(cmd->device && "Null gpu device!");
    assert(cmd->device->pProcTableCache->CmdTransferBufferToBuffer && "CmdTransferBufferToBuffer not loaded!");
    assert(desc->src != nullptr && "Null src cmdbuffer");
    assert(desc->dst != nullptr && "Null dst cmdbuffer");
    assert(cmd->currentDispatch == GPU_PIPELINE_TYPE_NONE && "fatal: can't call transfer apis on commdn buffer while preparing dispatching!");
    cmd->device->pProcTableCache->CmdTransferBufferToBuffer(cmd, desc);
}

void GPUCmdTransferTextureToTexture(GPUCommandBufferID cmd, const struct GPUTextureToTextureTransfer* desc)
{
    assert(cmd && "Null gpu commandbuffer!");
    assert(cmd->device && "Null gpu device!");
    assert(cmd->device->pProcTableCache->CmdTransferTextureToTexture && "CmdTransferTextureToTexture not loaded!");
    assert(desc->src != nullptr && "Null src cmdbuffer");
    assert(desc->dst != nullptr && "Null dst cmdbuffer");
    assert(cmd->currentDispatch == GPU_PIPELINE_TYPE_NONE && "fatal: can't call transfer apis on commdn buffer while preparing dispatching!");
    cmd->device->pProcTableCache->CmdTransferTextureToTexture(cmd, desc);
}

GPUFenceID GPUCreateFence(GPUDeviceID device)
{
    assert(device && "Null gpu device!");
    assert(device->pProcTableCache->CreateFence && "CreateFence not loaded!");
    GPUFence* F = (GPUFence*)device->pProcTableCache->CreateFence(device);
    F->device   = device;
    return F;
}

void GPUFreeFence(GPUFenceID fence)
{
    assert(fence && "Null gpu fence!");
    assert(fence->device && "Null gpu device!");
    assert(fence->device->pProcTableCache->FreeFence && "FreeFence not loaded!");
    fence->device->pProcTableCache->FreeFence(fence);
}

void GPUWaitFences(const GPUFenceID* fences, uint32_t fenceCount)
{
    if (fences == NULL || fenceCount == 0) return;
    GPUFenceID fence = fences[0];
    assert(fence && "Null gpu fence!");
    assert(fence->device && "Null gpu device!");
    assert(fence->device->pProcTableCache->WaitFences && "WaitFences not loaded!");
    fence->device->pProcTableCache->WaitFences(fences, fenceCount);
}

EGPUFenceStatus GPUQueryFenceStatus(GPUFenceID fence)
{
    assert(fence && "Null gpu fence!");
    assert(fence->device && "Null gpu device!");
    assert(fence->device->pProcTableCache->QueryFenceStatus && "QueryFenceStatus not loaded!");
    return fence->device->pProcTableCache->QueryFenceStatus(fence);
}

GPUSemaphoreID GPUCreateSemaphore(GPUDeviceID device)
{
    assert(device && "Null gpu device!");
    assert(device->pProcTableCache->GpuCreateSemaphore && "GpuCreateSemaphore not loaded!");
    GPUSemaphore* s = (GPUSemaphore*)device->pProcTableCache->GpuCreateSemaphore(device);
    s->device       = device;
    return s;
}

void GPUFreeSemaphore(GPUSemaphoreID semaphore)
{
    assert(semaphore && "Null gpu semaphore!");
    assert(semaphore->device && "Null gpu device!");
    assert(semaphore->device->pProcTableCache->GpuFreeSemaphore && "GpuFreeSemaphore not loaded!");
    semaphore->device->pProcTableCache->GpuFreeSemaphore(semaphore);
}

GPURenderPassEncoderID GPUCmdBeginRenderPass(GPUCommandBufferID cmd, const struct GPURenderPassDescriptor* desc)
{
    assert(cmd && "Null gpu commandbuffer!");
    assert(cmd->device && "Null gpu device!");
    assert(cmd->device->pProcTableCache->CmdBeginRenderPass && "CmdBeginRenderPass not loaded!");
    GPURenderPassEncoderID id = cmd->device->pProcTableCache->CmdBeginRenderPass(cmd, desc);
    GPUCommandBuffer* b       = (GPUCommandBuffer*)cmd;
    b->currentDispatch        = GPU_PIPELINE_TYPE_GRAPHICS;
    return id;
}

void GPUCmdEndRenderPass(GPUCommandBufferID cmd, GPURenderPassEncoderID encoder)
{
    assert(cmd && "Null gpu commandbuffer!");
    assert(cmd->device && "Null gpu device!");
    assert(cmd->device->pProcTableCache->CmdEndRenderPass && "CmdEndRenderPass not loaded!");
    assert(cmd->currentDispatch == GPU_PIPELINE_TYPE_GRAPHICS);
    cmd->device->pProcTableCache->CmdEndRenderPass(cmd, encoder);
    GPUCommandBuffer* b = (GPUCommandBuffer*)cmd;
    b->currentDispatch  = GPU_PIPELINE_TYPE_NONE;
}

void GPURenderEncoderSetViewport(GPURenderPassEncoderID encoder, float x, float y, float width, float height, float min_depth, float max_depth)
{
    GPUDeviceID D = encoder->device;
    assert(D && "Null gpu device!");
    assert(D->pProcTableCache->RenderEncoderSetViewport && "RenderEncoderSetViewport not loaded!");
    D->pProcTableCache->RenderEncoderSetViewport(encoder, x, y, width, height, min_depth, max_depth);
}

void GPURenderEncoderSetScissor(GPURenderPassEncoderID encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    GPUDeviceID D = encoder->device;
    assert(D && "Null gpu device!");
    assert(D->pProcTableCache->RenderEncoderSetScissor && "RenderEncoderSetScissor not loaded!");
    D->pProcTableCache->RenderEncoderSetScissor(encoder, x, y, width, height);
}

void GPURenderEncoderBindPipeline(GPURenderPassEncoderID encoder, GPURenderPipelineID pipeline)
{
    GPUDeviceID D = encoder->device;
    assert(D && "Null gpu device!");
    assert(D->pProcTableCache->RenderEncoderBindPipeline && "RenderEncoderBindPipeline not loaded!");
    D->pProcTableCache->RenderEncoderBindPipeline(encoder, pipeline);
}

void GPURenderEncoderDraw(GPURenderPassEncoderID encoder, uint32_t vertex_count, uint32_t first_vertex)
{
    GPUDeviceID D = encoder->device;
    assert(D && "Null gpu device!");
    assert(D->pProcTableCache->RenderEncoderDraw && "RenderEncoderDraw not loaded!");
    D->pProcTableCache->RenderEncoderDraw(encoder, vertex_count, first_vertex);
}

void GPURenderEncoderDrawIndexed(GPURenderPassEncoderID encoder, uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset)
{
    GPUDeviceID D = encoder->device;
    assert(D && "Null gpu device!");
    assert(D->pProcTableCache->RenderEncoderDrawIndexed && "RenderEncoderDrawIndexed not loaded!");
    D->pProcTableCache->RenderEncoderDrawIndexed(encoder, indexCount, firstIndex, vertexOffset);
}

void GPURenderEncoderDrawIndexedInstanced(GPURenderPassEncoderID encoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    GPUDeviceID D = encoder->device;
    assert(D && "Null gpu device!");
    assert(D->pProcTableCache->RenderEncoderDrawIndexedInstanced && "RenderEncoderDrawIndexedInstanced not loaded!");
    D->pProcTableCache->RenderEncoderDrawIndexedInstanced(encoder, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void GPURenderEncoderBindVertexBuffers(GPURenderPassEncoderID encoder, uint32_t buffer_count,
                                       const GPUBufferID* buffers, const uint32_t* strides, const uint32_t* offsets)
{
    assert(encoder && "Null renderpass encoder!");
    assert(encoder->device && "Null gpu device!");
    assert(encoder->device->pProcTableCache->RenderEncoderBindVertexBuffers && "RenderEncoderBindVertexBuffers not loaded!");
    assert(buffers && "Null vertex buffers!");
    encoder->device->pProcTableCache->RenderEncoderBindVertexBuffers(encoder, buffer_count, buffers, strides, offsets);
}

void GPURenderEncoderBindIndexBuffer(GPURenderPassEncoderID encoder, GPUBufferID buffer, uint32_t offset, uint64_t indexStride)
{
    assert(encoder && "Null renderpass encoder!");
    assert(encoder->device && "Null gpu device!");
    assert(encoder->device->pProcTableCache->RenderEncoderBindIndexBuffer && "RenderEncoderBindVertexBuffers not loaded!");
    assert(buffer && "Null index buffer!");
    encoder->device->pProcTableCache->RenderEncoderBindIndexBuffer(encoder, buffer, offset, indexStride);
}

void GPURenderEncoderBindDescriptorSet(GPURenderPassEncoderID encoder, GPUDescriptorSetID set)
{
    assert(encoder && "Null renderpass encoder!");
    assert(encoder->device && "Null gpu device!");
    assert(set && "Null descriptorset!");
    assert(encoder->device->pProcTableCache->RenderEncoderBindDescriptorSet && "RenderEncoderBindDescriptorSet not loaded!");
    encoder->device->pProcTableCache->RenderEncoderBindDescriptorSet(encoder, set);
}

GPUBufferID GPUCreateBuffer(GPUDeviceID device, const GPUBufferDescriptor* desc)
{
    assert(device && "Null gpu device!");
    assert(device->pProcTableCache->CreateBuffer && "CreateBuffer not loaded!");
    GPUBufferDescriptor new_desc;
    memcpy(&new_desc, desc, sizeof(GPUBufferDescriptor));
    if (desc->flags == 0)
    {
        new_desc.flags |= GPU_BCF_NONE;
    }
    GPUBuffer* B = (GPUBuffer*)device->pProcTableCache->CreateBuffer(device, &new_desc);
    B->device    = device;
    return B;
}

void GPUFreeBuffer(GPUBufferID buffer)
{
    assert(buffer && "Null gpu buffer!");
    assert(buffer->device && "Null gpu device!");
    assert(buffer->device->pProcTableCache->FreeBuffer && "FreeBuffer not loaded!");
    buffer->device->pProcTableCache->FreeBuffer(buffer);
}

GPUSamplerID GPUCreateSampler(GPUDeviceID device, const struct GPUSamplerDescriptor* desc)
{
    assert(device && "Null gpu device!");
    assert(desc && "Null gpu sampler descriptor!");
    assert(device->pProcTableCache->CreateSampler && "CreateSampler not loaded!");
    GPUSampler* sampler = (GPUSampler*)device->pProcTableCache->CreateSampler(device, desc);
    sampler->device     = device;
    return sampler;
}

void GPUFreeSampler(GPUSamplerID sampler)
{
    assert(sampler && "Null gpu sampler!");
    assert(sampler->device && "Null gpu device!");
    assert(sampler->device->pProcTableCache->FreeSampler && "FreeSampler not loaded!");
    sampler->device->pProcTableCache->FreeSampler(sampler);
}

GPUDescriptorSetID GPUCreateDescriptorSet(GPUDeviceID device, const struct GPUDescriptorSetDescriptor* desc)
{
    assert(device && "Null gpu device!");
    assert(device->pProcTableCache->CreateDescriptorSet && "CreateDescriptorSet not loaded!");
    GPUDescriptorSet* set = (GPUDescriptorSet*)device->pProcTableCache->CreateDescriptorSet(device, desc);
    set->root_signature   = desc->root_signature;
    set->index            = desc->set_index;
    return set;
}
void GPUFreeDescriptorSet(GPUDescriptorSetID set)
{
    assert(set && "Null gpu descriptorset!");
    assert(set->root_signature && "Null gpu root signature!");
    assert(set->root_signature->device && "Null gpu device!");
    assert(set->root_signature->device->pProcTableCache->FreeDescriptorSet && "FreeDescriptorSet not loaded!");
    set->root_signature->device->pProcTableCache->FreeDescriptorSet(set);
}
void GPUUpdateDescriptorSet(GPUDescriptorSetID set, const GPUDescriptorData* datas, uint32_t count)
{
    assert(set && "Null gpu descriptorset!");
    assert(set->root_signature && "Null gpu root signature!");
    assert(set->root_signature->device && "Null gpu device!");
    assert(set->root_signature->device->pProcTableCache->UpdateDescriptorSet && "UpdateDescriptorSet not loaded!");
    assert(datas);
    set->root_signature->device->pProcTableCache->UpdateDescriptorSet(set, datas, count);
}
