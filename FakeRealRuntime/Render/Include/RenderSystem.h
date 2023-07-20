#pragma omce

#include "Config.h"
#include <memory>

namespace FakeReal
{
    class WindowSystem;
    struct RenderDeviceInfo
    {
        std::shared_ptr<WindowSystem> pWindowSystem;
    };

    class RenderDevice;
    class RENDER_API RenderSystem
    {
    public:
        RenderSystem();
        ~RenderSystem();

        void Initialize(RenderDeviceInfo info);
        void Shutdown();

    private:
        RenderDevice* m_pRenderDevice;
    };
}