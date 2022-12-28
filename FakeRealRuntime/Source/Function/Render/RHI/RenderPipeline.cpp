#include "FRPch.h"
#include "RenderPipeline.h"
#include "Function/Render/RHI/Pass/RenderPass.h"

namespace FakeReal
{
	RenderPipeline::RenderPipeline()
	{}

	RenderPipeline::~RenderPipeline()
	{}

	void RenderPipeline::PreparePassData()
	{
		m_pMainCameraPass->PreparePassData();
	}

	void RenderPipeline::SetRHI(SharedPtr<RHI> rhi)
	{
		m_pRhi = rhi;
	}

}