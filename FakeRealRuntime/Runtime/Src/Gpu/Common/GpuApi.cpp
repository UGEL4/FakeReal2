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
