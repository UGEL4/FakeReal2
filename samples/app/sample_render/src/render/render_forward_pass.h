#pragma noce

#include "RenderPrimitivePass.h"

using namespace FakeReal;

class RenderPassForward : public IRenderPrimitivePass
{
public:
    RenderPassForward();
    ~RenderPassForward();

    virtual void Initialize() final;
    virtual void Clear() final;
    virtual void OnUpdate(const RenderPrimitivePassContext* context) final;
    virtual void Execute(const RenderPrimitivePassContext* context) final;
    virtual void PostUpdate(const RenderPrimitivePassContext* context) final;
};