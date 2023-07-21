#pragma omce

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
    class RENDER_API RenderSystem
    {
    public:
        RenderSystem();
        ~RenderSystem();

        void Initialize(RenderDeviceInfo info);
        void Shutdown();
        void Tick();

        void RegisterRenderPass(IRenderPrimitivePass* pass);

    private:
        RenderDevice* m_pRenderDevice;
        std::vector<IRenderPrimitivePass*> mPasses;
    };
}