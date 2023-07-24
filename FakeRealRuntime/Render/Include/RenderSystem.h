#pragma once

#include "Config.h"
#include <memory>
#include "Gpu/GpuApi.h"
#include <vector>

namespace FakeReal
{
    class WindowSystem;
    struct RenderDeviceInfo
    {
        EGPUBackend backend;
        bool enableDebugLayer;
        bool enableValidation;
        std::shared_ptr<WindowSystem> pWindowSystem;
    };

    class RenderDevice;
    class IRenderPrimitivePass;
    class IRenderEffect;

    namespace render_graph
    {
        class RenderGraph;
    }

    class RENDER_API RenderSystem
    {
    public:
        RenderSystem();
        ~RenderSystem();

        void Initialize(RenderDeviceInfo info);
        void Shutdown();
        void Tick(FakeReal::render_graph::RenderGraph* pRenderGraph);

        void RegisterRenderPass(IRenderPrimitivePass* pass);
        void RemoveRenderPass(IRenderPrimitivePass* pass);
        void RegisterRenderEffect(IRenderEffect* effect);
        void RemoveRenderEffect(IRenderEffect* effect);

        RenderDevice* GetRenderDevice() const
        {
            return m_pRenderDevice;
        }

    private:
        RenderDevice* m_pRenderDevice;
        std::vector<IRenderPrimitivePass*> mPasses;
        std::vector<IRenderEffect*> mResources;
    };
}