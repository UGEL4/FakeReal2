#include "FRPch.h"
#include "RenderSystem.h"
#include "RHI/Vulkan/VulkanRHI.h"
#include "RHI/Vulkan/Pipeline/RenderPipeline_Vulkan.h"
#include "Function/Render/Vulkan/VulkanRenderResource.h"
#include "Function/Render/RHI/Vulkan/Pass/MainCameraPass_Vulkan.h"

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

		m_pRenderResource = MakeShared<VulkanRenderResource>();
		VkDescriptorSetLayout pLayout = std::static_pointer_cast<RenderPass_Vulkan>(m_pPipeline->GetMainCameraPass())->mDescriptorInfos[MainCameraPass_Vulkan::LT_MESH_PER_MATERIAL].pLayout;
		m_pRenderResource->SetMaterialDescriptorSetLayout(pLayout);
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
		ProcessSwapData();
		//render
		m_pRhi->PrepareContext();
		//update preframebuffer
		//prepare pass data
		m_pPipeline->PreparePassData();

		m_pPipeline->DeferredRender(m_pRhi, m_pRenderResource);
	}

	void RenderSystem::ProcessSwapData()
	{
		static bool assertLoad = false;
		static size_t asset_id = 1;

		RenderMeshData meshData;
		RenderMaterialData materialData;

		if (!assertLoad)
		{
			assertLoad = true;

			meshData		= m_pRenderResource->LoadMeshData();
			materialData	= m_pRenderResource->LoadMaterialData("texture/huaji.jpg");

			m_pRenderResource->UploadGameobjectRenderResource(m_pRhi, asset_id, meshData);
			m_pRenderResource->UploadGameobjectRenderResource(m_pRhi, asset_id, materialData);
		}
	}

}