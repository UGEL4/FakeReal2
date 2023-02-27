#include "FRPch.h"
#include "RenderPipeline_Vulkan.h"
#include "Function/Render/RHI/Vulkan/Pass/MainCameraPass_Vulkan.h"
#include "Function/Render/RHI/Vulkan/VulkanRHI.h"
#include "Function/Render/Vulkan/VulkanRenderResource.h"
#include <functional>

namespace FakeReal
{
	RenderPipeline_Vulkan::RenderPipeline_Vulkan()
	{
		
	}

	RenderPipeline_Vulkan::~RenderPipeline_Vulkan()
	{

	}

	void RenderPipeline_Vulkan::Initialize()
	{
		m_pMainCameraPass = MakeShared<MainCameraPass_Vulkan>();
		RenderPassCommonInfo info = {};
		info.rhi = m_pRhi;
		m_pMainCameraPass->SetCommonInfo(info);
		m_pMainCameraPass->Initialize(nullptr);
	}

	void RenderPipeline_Vulkan::Clear()
	{
		m_pMainCameraPass->Clear();
		m_pMainCameraPass.reset();
	}

	void RenderPipeline_Vulkan::DeferredRender(SharedPtr<RHI> rhi, SharedPtr<RenderResource> renderResource)
	{
		VulkanRHI* pVulkanRhi = static_cast<VulkanRHI*>(rhi.get());

		pVulkanRhi->WaitForFences();

		pVulkanRhi->ResetCommandPool();

		bool recreate = pVulkanRhi->PrepareFrame(std::bind(&RenderPipeline_Vulkan::PassUpdateAfterRecreateSwapchain, this));
		if (recreate)
		{
			return;
		}

		((MainCameraPass_Vulkan*)m_pMainCameraPass.get())->Draw(pVulkanRhi->mCurSwapchainImageIndex);

		pVulkanRhi->SubmitRendering(std::bind(&RenderPipeline_Vulkan::PassUpdateAfterRecreateSwapchain, this));
	}

	void RenderPipeline_Vulkan::PassUpdateAfterRecreateSwapchain()
	{
		m_pMainCameraPass->PassUpdateAfterRecreateSwapchain();
	}

}