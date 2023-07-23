#include "RenderSystem.h"
#include "RenderDevice.h"
#include "RenderPrimitivePass.h"
#include "RenderEffect.h"
#include <xmemory>

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

    void RenderSystem::Tick(FakeReal::render_graph::RenderGraph* pRenderGraph)
    {
        //produce draw calls
        for (auto& e : mResources)
        {
            if (e)
            {
                PrimitiveUpdateContext context {};
                context.pRender      = this;
                context.pRenderGraph = pRenderGraph;
                e->OnUpdate(&context);
            }
        }

        for (auto& pass : mPasses)
        {
            for (auto&e : mResources)
            {
                if (pass && e)
                {
                    PrimitiveDrawContext context {};
                    context.pRender      = this;
                    context.pPass        = pass;
                    context.pRenderGraph = pRenderGraph;
                    e->ProduceDraw(&context);
                }
            }
        }

        for (auto& e : mResources)
        {
            if (e)
            {
                PrimitiveUpdateContext context {};
                context.pRender      = this;
                context.pRenderGraph = pRenderGraph;
                e->PostUpdate(&context);
            }
        }

        //execute draw calls
        for (auto& pass : mPasses)
        {
            if (pass)
            {
                RenderPrimitivePassContext context = {};
                context.pRenderSystem              = this;
                context.pRenderGraph               = pRenderGraph;
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

    void RenderSystem::RemoveRenderPass(IRenderPrimitivePass* pass)
    {
        std::erase_if(mPasses, [pass](IRenderPrimitivePass*& p)
        {
            return p == pass;
        });
    }

    void RenderSystem::RegisterRenderEffect(IRenderEffect* effect)
    {
        for (auto& e : mResources)
        {
            if (e == effect)
            {
                assert(false && "Render effect already registered!");
                return;
            }
        }
        mResources.emplace_back(effect);
        effect->OnRegister(this);
    }

    void RenderSystem::RemoveRenderEffect(IRenderEffect* effect)
    {
        std::erase_if(
        mResources, [effect, this](IRenderEffect*& e) {
            if (e == effect)
            {
                e->OnUnregister(this);
                return true;
            }
            return false;
        });
    }
}