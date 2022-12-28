#include "FRPch.h"
#include "RenderSystem.h"
#include "RHI/Vulkan/VulkanRHI.h"
#include "RHI/Vulkan/Pipeline/RenderPipeline_Vulkan.h"
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
		m_pRhi = MakeShared<VulkanRHI>();
		m_pRhi->Initialize(info);

		m_pPipeline = MakeShared<RenderPipeline_Vulkan>();
		m_pPipeline->SetRHI(m_pRhi);
		m_pPipeline->Initialize();
	}

	void RenderSystem::Shutdown()
	{
		m_pPipeline->Clear();
		m_pPipeline.reset();

		m_pRhi->Clear();
		m_pRhi.reset();
	}

	void RenderSystem::Tick()
	{
		//render
		m_pRhi->PrepareContext();
		//update preframebuffer
		//prepare pass data
		m_pPipeline->PreparePassData();

		m_pPipeline->DeferredRender();
	}

}