#include "FRPch.h"
#include "RenderPass_Vulkan.h"
#include "Function/Render/RHI/Vulkan/VulkanRHI.h"

namespace FakeReal
{

	RenderPass_Vulkan::~RenderPass_Vulkan()
	{

	}

	void RenderPass_Vulkan::Initialize(RenderPassCommonInfo* pInfo)
	{
		m_pVulkanRhi = StaticPointCast<VulkanRHI>(m_pRHI);
	}

	void RenderPass_Vulkan::Clear()
	{

	}

}