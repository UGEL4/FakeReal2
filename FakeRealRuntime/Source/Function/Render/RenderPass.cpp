#include "FRPch.h"
#include "RenderPass.h"

namespace FakeReal
{

	void RenderPass::Initialize(RenderPassCommonInfo* pInfo)
	{
		m_pVulkanRHI = std::static_pointer_cast<VulkanRHI>(m_pRHI);
	}

	void RenderPass::Draw()
	{

	}

}