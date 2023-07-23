#pragma once
#include "Config.h"

namespace FakeReal
{
    namespace render_graph
    {
        class RenderGraph;
    }
}

namespace FakeReal
{
    class RenderSystem;
    class IRenderPrimitivePass;

    typedef struct PrimitiveDrawContext
    {
        RenderSystem* pRender;
        IRenderPrimitivePass* pPass;
        FakeReal::render_graph::RenderGraph* pRenderGraph;
    } PrimitiveDrawContext;

    typedef struct PrimitiveUpdateContext
    {
        RenderSystem* pRender;
        FakeReal::render_graph::RenderGraph* pRenderGraph;
    } PrimitiveUpdateContext;

    class RENDER_API IRenderEffect
    {
    public:
        virtual ~IRenderEffect() = default;

        virtual void OnRegister(RenderSystem* pRender) = 0;
        virtual void OnUnregister(RenderSystem* pRender) = 0;
        virtual void ProduceDraw(const PrimitiveDrawContext* context) = 0;

        virtual void OnUpdate(const PrimitiveUpdateContext* context) {}
        virtual void PostUpdate(const PrimitiveUpdateContext* context) {}
    };
}