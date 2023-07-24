#pragma once

#include "RenderEffect.h"
#include "Gpu/GpuApi.h"

using namespace FakeReal;

class RenderEffectForward : public IRenderEffect
{
public:
    RenderEffectForward();
    ~RenderEffectForward();

    virtual void OnRegister(RenderSystem* pRender) final;
    virtual void OnUnregister(RenderSystem* pRender) final;
    virtual void ProduceDraw(const PrimitiveDrawContext* context) final;
private:
    void PrepareResources(RenderSystem* pRender);
    void FreeResources();
    void CreatePipeline(RenderSystem* pRender);
private:
    GPUBufferID mVertexBuffer;
    GPUBufferID mIndexBuffer;
    GPURenderPipelineID mPipeline;
};