#include "FRPch.h"
#include "RenderSystem.h"
#include "RHI/Vulkan/VulkanRHI.h"
namespace FakeReal
{

	RenderSystem::RenderSystem()
	{

	}

	RenderSystem::~RenderSystem()
	{
		m_pRhi.reset();
	}

	void RenderSystem::Initialize(const RHIInitInfo& info)
	{
		//IF VULKAN
		m_pRhi = std::make_shared<VulkanRHI>();
		m_pRhi->Initialize(info);
	}

	void RenderSystem::Shutdown()
	{
		m_pRhi->Clear();
	}

	void RenderSystem::Tick()
	{
		//render
	}

}