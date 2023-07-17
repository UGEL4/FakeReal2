#include "Gpu/GpuApi.h"

#ifdef GPU_USE_VULKAN
    #include "Gpu/Backend/Vulkan/GPUVulkan.h"
#endif

GPU_API GPUInstanceID GPUCreateInstance(const GPUInstanceDescriptor* pDesc)
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

GPU_API void GPUFreeInstance(GPUInstanceID instance)
{
    assert(instance->backend == EGPUBackend::GPUBackend_Vulkan);
    assert(instance->pProcTable->FreeInstance);

    instance->pProcTable->FreeInstance(instance);
}

GPU_API void GPUEnumerateAdapters(GPUInstanceID pInstance, GPUAdapterID *const ppAdapters, uint32_t *adapterCount)
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

GPU_API GPUDeviceID GPUCreateDevice(GPUAdapterID pAdapter, const struct GPUDeviceDescriptor *pDesc)
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

GPU_API void GPUFreeDevice(GPUDeviceID pDevice)
{
    assert(pDevice->pProcTableCache && "No ProcTableCach!");
    assert(pDevice->pProcTableCache->FreeDevice && "FreeDevice not loaded!");
    pDevice->pProcTableCache->FreeDevice(pDevice);
}

GPU_API GPUSurfaceID GPUCreateSurfaceFromNativeView(GPUInstanceID pInstance, void *view)
{
    #if defined(_WIN64)
    return GPUCreateSurfaceFromHWND(pInstance, (HWND)view);
#endif

    return nullptr;
}

GPU_API void GPUFreeSurface(GPUInstanceID pInstance, GPUSurfaceID pSurface)
{
    assert(pInstance && "Null gpu instance!");
    assert(pInstance->pSurfaceProcTable && "Null SurfaceProcTabl!");
    assert(pInstance->pSurfaceProcTable->FreeSurface && "FreeSurface not loaded!");
    pInstance->pSurfaceProcTable->FreeSurface(pInstance, pSurface);
}

#if defined(_WIN64)
GPU_API GPUSurfaceID GPUCreateSurfaceFromHWND(GPUInstanceID pInstance, HWND window)
{
    assert(pInstance && "Null gpu instance!");
    assert(pInstance->pSurfaceProcTable && "Null SurfaceProcTabl!");
    assert(pInstance->pSurfaceProcTable->CreateSurfaceFromHWND  && "CreateSurfaceFromHWND not loaded!");
    return pInstance->pSurfaceProcTable->CreateSurfaceFromHWND(pInstance, window);
}
#endif

GPU_API GPUQueueID GPUGetQueue(GPUDeviceID pDevice, EGPUQueueType queueType, uint32_t queueIndex)
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

GPU_API void GPUFreeQueue(GPUQueueID queue)
{
    assert(queue && "Null gpu queue");
    assert(queue->pDevice && "Null gpu dvice");
    assert(queue->pDevice->pProcTableCache->FreeQueue && "No ProcTableCach!");
    queue->pDevice->pProcTableCache->FreeQueue(queue);
}
