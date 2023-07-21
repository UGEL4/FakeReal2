#include "RenderSystem.h"
#include "RenderDevice.h"
#include "RenderPrimitivePass.h"

namespace FakeReal
{
    RenderSystem::RenderSystem()
    {

    }

    RenderSystem::~RenderSystem()
    {

    }

    void RenderSystem::Initialize(RenderDeviceInfo info)
    {
        m_pRenderDevice = RenderDevice::Create();
        // register window
        RenderDevice::CreateInfo deviceInfo = {
            .backend          = info.backend,
            .enableDebugLayer = info.enableDebugLayer,
            .enableValidation = info.enableValidation
        };
        m_pRenderDevice->Initialize(deviceInfo);
        m_pRenderDevice->RegisterWindow(info.pWindowSystem.get());
    }

    void RenderSystem::Shutdown()
    {
        mPasses.clear();
        m_pRenderDevice->Finalize();
        RenderDevice::Free(m_pRenderDevice);
        m_pRenderDevice = nullptr;
    }

    void RenderSystem::Tick()
    {
        //produce draw calls
        //execute draw calls
        for (auto& pass : mPasses)
        {
            if (pass)
            {
                RenderPrimitivePassContext context = {};
                context.pRenderSystem = this;
                pass->OnUpdate(&context);
                pass->Execute(&context);
                pass->PostUpdate(&context);
            }
        }
        mPasses.clear();
    }

    void RenderSystem::RegisterRenderPass(IRenderPrimitivePass* pass)
    {
        for (auto& p : mPasses)
        {
            if (p == pass)
            {
                assert(false && "Render pass already registered!");
                return;
            }
        }
        mPasses.emplace_back(pass);
    }
}