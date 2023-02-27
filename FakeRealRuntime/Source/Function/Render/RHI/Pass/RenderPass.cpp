#include "FRPch.h"
#include "RenderPass.h"
#include "Function/Render/RHI.h"

FakeReal::RenderVisibleNodes FakeReal::RenderPass::mVisibleNodes;
namespace FakeReal
{
	RenderPass::~RenderPass()
	{

	}

	void RenderPass::SetCommonInfo(RenderPassCommonInfo info)
	{
		m_pRHI = info.rhi;
	}

}