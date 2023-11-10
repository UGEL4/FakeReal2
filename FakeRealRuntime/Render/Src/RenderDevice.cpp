#include "RenderDevice.h"
#include "Platform/Memory.h"
#include "WindowSystem.h"
#include <vector>

namespace FakeReal
{
    class RENDER_API RenderDeviceImpl : public RenderDevice
    {
    public:
        virtual void Initialize(const CreateInfo& info) override;
        virtual void Finalize() override;
        virtual void RegisterWindow(class WindowSystem* window) override;

        virtual GPUInstanceID GetGpuInstance() const override
        {
            return m_pInstance;
        }

        virtual GPUDeviceID GetGpuDevice() const final
        {
            return m_pDevice;
        }

        virtual GPUQueueID GetGFXQueue() const final
        {
            return m_pGFXQueue;
        }

        virtual GPUQueueID GetCopyQueue(uint32_t index) const final
        {
            if (index < mCopyQueues.size()) return mCopyQueues[index];

            return mCopyQueues[0];
        }

        virtual GPUSwapchainID GetSwapchain() const final
        {
            return m_pSwapchain;
        }

    private:
        GPUInstanceID m_pInstance;
        GPUAdapterID m_pAdapter;
        GPUSurfaceID m_pSurface;
        GPUDeviceID m_pDevice;
        GPUQueueID m_pGFXQueue;
        std::vector<GPUQueueID> mCopyQueues;
        GPUSwapchainID m_pSwapchain;
    };

    static uint32_t MAX_COPY_QUEUE_COUNT = 2;
    void RenderDeviceImpl::Initialize(const CreateInfo& info)
    {
        //instance
        GPUInstanceDescriptor ins_desc{};
        ins_desc.backend          = info.backend;
        ins_desc.enableDebugLayer = info.enableDebugLayer;
        ins_desc.enableValidation = info.enableValidation;
        m_pInstance = GPUCreateInstance(&ins_desc);

        //enumerate adapter
        uint32_t adapterCount = 0;
        GPUEnumerateAdapters(m_pInstance, nullptr, &adapterCount);
        DECLARE_ZERO_VAL(GPUAdapterID, ppAdapters, adapterCount);
        GPUEnumerateAdapters(m_pInstance, ppAdapters, &adapterCount);
        m_pAdapter = ppAdapters[0];

        //create device and queues
        uint32_t copyQueueCount = GPUQueryQueueCount(m_pAdapter, EGPUQueueType::GPU_QUEUE_TYPE_TRANSFER);
        GPUQueueGroupDescriptor groups[2] = {};
        groups[0].queueType  = EGPUQueueType::GPU_QUEUE_TYPE_GRAPHICS;
        groups[0].queueCount = 1;
        groups[1].queueType = EGPUQueueType::GPU_QUEUE_TYPE_TRANSFER;
        groups[1].queueCount = gpu_min(copyQueueCount, MAX_COPY_QUEUE_COUNT);
        if (groups[1].queueCount)
        {
            GPUDeviceDescriptor desc {};
            desc.pQueueGroup     = groups;
            desc.queueGroupCount = 2;
            m_pDevice            = GPUCreateDevice(m_pAdapter, &desc);
            m_pGFXQueue          = GPUGetQueue(m_pDevice, EGPUQueueType::GPU_QUEUE_TYPE_GRAPHICS, 0);
            mCopyQueues.resize(groups[1].queueCount);
            for (uint32_t i = 0; i < groups[1].queueCount; i++)
            {
                mCopyQueues[i] = GPUGetQueue(m_pDevice, EGPUQueueType::GPU_QUEUE_TYPE_TRANSFER, i);
            }
        }
        else
        {
            GPUDeviceDescriptor desc = {
                .pQueueGroup          = groups,
                .queueGroupCount      = 1,
                .disablePipelineCache = false
            };
            m_pDevice   = GPUCreateDevice(m_pAdapter, &desc);
            m_pGFXQueue = GPUGetQueue(m_pDevice, EGPUQueueType::GPU_QUEUE_TYPE_GRAPHICS, 0);
            mCopyQueues.emplace_back(m_pGFXQueue);
        }
    }

    void RenderDeviceImpl::Finalize()
    {
        //swapchain
        if (m_pSwapchain)
        {
            GPUFreeSwapchain(m_pSwapchain);
            m_pSwapchain = nullptr;
        }

        //queues
        for (auto& queue : mCopyQueues)
        {
            if (queue != m_pGFXQueue) GPUFreeQueue(queue);
        }
        mCopyQueues.clear();
        if (m_pGFXQueue)
        {
            GPUFreeQueue(m_pGFXQueue);
            m_pGFXQueue = nullptr;
        }

        // device
        if (m_pDevice)
        {
            GPUFreeDevice(m_pDevice);
            m_pDevice = nullptr;
        }
        
        // surface
        if (m_pSurface)
        {
            GPUFreeSurface(m_pInstance, m_pSurface);
            m_pSurface = nullptr;
        }

        // instance
        if (m_pInstance)
        {
            GPUFreeInstance(m_pInstance);
            m_pInstance = nullptr;
        }
    }

    void RenderDeviceImpl::RegisterWindow(class WindowSystem* window)
    {
        const std::array<int, 2> windowSize = window->GetWindowSize();

        m_pSurface = GPUCreateSurfaceFromNativeView(m_pInstance, window->GetWindow());

        // swapchain
        GPUSwapchainDescriptor swapchainDesc{};
        swapchainDesc.ppPresentQueues    = &m_pGFXQueue;
        swapchainDesc.presentQueuesCount = 1;
        swapchainDesc.pSurface           = m_pSurface;
        swapchainDesc.format             = EGPUFormat::GPU_FORMAT_B8G8R8A8_UNORM;
        swapchainDesc.width              = (uint32_t) windowSize[0];
        swapchainDesc.height             = (uint32_t) windowSize[1];
        swapchainDesc.imageCount         = 3;
        swapchainDesc.enableVSync        = true;
        m_pSwapchain = GPUCreateSwapchain(m_pDevice, &swapchainDesc);
    }

/////////////////////////////////////////////////////////////////////////////////
    RenderDevice* RenderDevice::Create() FR_NOEXCEPT
    {
        return FR_New<RenderDeviceImpl>();
    }

    void RenderDevice::Free(RenderDevice* device) FR_NOEXCEPT
    {
        FR_Delete(device);
    }
}