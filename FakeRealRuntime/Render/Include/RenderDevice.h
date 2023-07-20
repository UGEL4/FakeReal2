#pragma once
#include "Config.h"
#include "Gpu/GpuApi.h"

namespace FakeReal
{
    class RENDER_API RenderDevice
    {
    public:
        struct CreateInfo
        {
            EGPUBackend backend;
            bool enableDebugLayer;
            bool enableValidation;
        };

        static RenderDevice* Create() FR_NOEXCEPT;
        static void Free(RenderDevice* device) FR_NOEXCEPT;

        virtual ~RenderDevice() = default;

        virtual void Initialize(const CreateInfo& info) = 0;
        virtual void Finalize() = 0;
        virtual void RegisterWindow(class WindowSystem* window) = 0;

        virtual GPUInstanceID GetGpuInstance() const = 0;
        virtual GPUDeviceID GetGpuDevice() const = 0;
        virtual GPUQueueID GetGFXQueue() const = 0;
        virtual GPUQueueID GetCopyQueue(uint32_t index) const = 0;
        virtual GPUSwapchainID GetSwapchain() const = 0;
    };
}