#include "FRPch.h"
#include "RenderPipeline.h"
#include "Function/Render/RHI/Pass/RenderPass.h"

namespace FakeReal
{
	RenderPipeline::RenderPipeline()
	{}

	RenderPipeline::~RenderPipeline()
	{}

	void RenderPipeline::PreparePassData(SharedPtr<RenderResource> pRenderResource)
	{
		m_pMainCameraPass->PreparePassData(pRenderResource);
	}

	void RenderPipeline::SetRHI(SharedPtr<RHI> rhi)
	{
		m_pRhi = rhi;
	}

}