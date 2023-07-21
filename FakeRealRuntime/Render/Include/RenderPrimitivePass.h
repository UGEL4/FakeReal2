#pragma once

namespace FakeReal
{
    typedef struct RenderPrimitivePassContext
    {
        class RenderSystem* pRenderSystem;
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
}