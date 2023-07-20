#include "RenderSystem.h"
#include "RenderDevice.h"

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
        //m_pRenderDevice->Initialize(const CreateInfo &info)
    }

    void RenderSystem::Shutdown()
    {
        m_pRenderDevice->Finalize();
        RenderDevice::Free(m_pRenderDevice);
        m_pRenderDevice = nullptr;
    }
}