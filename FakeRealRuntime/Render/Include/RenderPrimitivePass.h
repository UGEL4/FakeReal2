#pragma once

namespace FakeReal
{
    namespace render_graph
    {
        class RenderGraph;
    }
} // namespace FakeReal

namespace FakeReal
{
    typedef struct RenderPrimitivePassContext
    {
        class RenderSystem* pRenderSystem;
        FakeReal::render_graph::RenderGraph* pRenderGraph;
    } RenderPrimitivePassContext;

    class IRenderPrimitivePass
    {
    public:
        virtual ~IRenderPrimitivePass() = default;

        virtual void Initialize() = 0;
        virtual void Clear() = 0;
        virtual void OnUpdate(const RenderPrimitivePassContext* context) = 0;
        virtual void Execute(const RenderPrimitivePassContext* context) = 0;
        virtual void PostUpdate(const RenderPrimitivePassContext* context) = 0;
    };
} // namespace FakeReal