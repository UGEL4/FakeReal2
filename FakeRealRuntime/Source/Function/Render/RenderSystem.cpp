#include "FRPch.h"
#include "RenderSystem.h"
#include "RHI/Vulkan/VulkanRHI.h"
#include "RHI/Vulkan/VulkanUtils.h"
#include "RHI/Vulkan/Pipeline/RenderPipeline_Vulkan.h"
#include "Function/Render/Vulkan/VulkanRenderResource.h"
#include "Function/Render/RHI/Vulkan/Pass/MainCameraPass_Vulkan.h"
#include "Function/Render/Scene.h"
#include "Function/Render/RenderEntity.h"

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
		//m_pRenderResource->SetMaterialDescriptorSetLayout(pLayout);
		std::static_pointer_cast<VulkanRenderResource>(m_pRenderResource)->m_pMaterialDescriptorSetLayout = &std::static_pointer_cast<RenderPass_Vulkan>(m_pPipeline->GetMainCameraPass())->mDescriptorInfos[MainCameraPass_Vulkan::LT_MESH_PER_MATERIAL].pLayout;

		m_pRenderScene = MakeShared<Scene>();
		m_pRenderScene->SetVisibleNodeReference();
	}

	void RenderSystem::Shutdown()
	{
		m_pRhi->DeviceWaitIdle();

		//must be first call
		VulkanUtils::ReleaseCacheResources(std::static_pointer_cast<VulkanRHI>(m_pRhi));
		m_pRenderResource->ReleaseAllResources();

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
		//update framebuffer

		m_pRenderScene->UpdateVisibleObjects(m_pRenderResource);
		//prepare pass data
		m_pPipeline->PreparePassData(m_pRenderResource);

		m_pPipeline->DeferredRender(m_pRhi, m_pRenderResource);
	}

	void RenderSystem::SwapRenderData()
	{
		mRenderSwapContext.SwapLogicRenderData();
	}

	void RenderSystem::ProcessSwapData()
	{
		RenderSwapData& swapData = mRenderSwapContext.GetRenderSwapData();

		if (swapData.mGameObjectResourecDesc.has_value())
		{
			while (!swapData.mGameObjectResourecDesc->Empty())
			{
				GameObjectDesc object = swapData.mGameObjectResourecDesc->GetNextGameObject();
				for (size_t partIndex = 0; partIndex < object.GetGameObjectParts().size(); partIndex++)
				{
					const auto& gameObjectPart = object.GetGameObjectParts()[partIndex];
					GameObjectPartId partId = { object.GetId(), partIndex };

					bool isEntityInScene = m_pRenderScene->GetInstanceIdAllocator().HasElement(partId);

					RenderEntity renderEntity;
					renderEntity.mInstanceId = m_pRenderScene->GetInstanceIdAllocator().AllocGuid(partId);

					m_pRenderScene->AddInstanceToMap(renderEntity.mInstanceId, object.GetId());

					//mesh
					//MeshResourceDesc meshResDesc = { gameObjectPart.mMeshDesc.mMeshFile };
					MeshResourceDesc meshResDesc = { "asset/objects/character/default/stromtrooper/component/mesh/Stormtroopermesh.mesh.json" };
					//MeshResourceDesc meshResDesc = { "asset/objects/character/default/cube/component/mesh/cube.mesh.json" };
					bool meshAssetLoaded = m_pRenderScene->GetMeshAssetIdAllocator().HasElement(meshResDesc);

					RenderMeshData meshData;
					if (!meshAssetLoaded)
					{
						meshData = m_pRenderResource->LoadMeshData(meshResDesc);
					}

					renderEntity.mMeshAssetId = m_pRenderScene->GetMeshAssetIdAllocator().AllocGuid(meshResDesc);

					//material
					MaterialResourceDesc materialResDesc;
					if (gameObjectPart.mMaterialDesc.mWithTexture)
					{
						materialResDesc.mBaseColorTextureFile = gameObjectPart.mMaterialDesc.mBaseColorTextureFile;
					}
					else
					{
						//materialResDesc.mBaseColorTextureFile = "texture/huaji.jpg";
						materialResDesc.mBaseColorTextureFile = "asset/objects/character/default/stromtrooper/component/mesh/texture/Stormtrooper_D.png";
						//materialResDesc.mBaseColorTextureFile = "asset/objects/character/default/cube/component/mesh/texture/huaji.jpg";
					}

					bool materialAssetLoaded = m_pRenderScene->GetMaterialAssetIdAllocator().HasElement(materialResDesc);
					RenderMaterialData materialData;
					if (!materialAssetLoaded)
					{
						materialData = m_pRenderResource->LoadMaterialData(materialResDesc);
					}
					renderEntity.mMaterialAssetId = m_pRenderScene->GetMaterialAssetIdAllocator().AllocGuid(materialResDesc);

					if (!meshAssetLoaded)
					{
						m_pRenderResource->UploadGameobjectRenderResource(m_pRhi, renderEntity, meshData);
					}

					if (!materialAssetLoaded)
					{
						m_pRenderResource->UploadGameobjectRenderResource(m_pRhi, renderEntity, materialData);
					}

					if (!isEntityInScene)
					{
						m_pRenderScene->mRenderEntities.emplace_back(renderEntity);
					}
					else
					{
						for (auto& entity : m_pRenderScene->mRenderEntities)
						{
							if (entity.mInstanceId == renderEntity.mInstanceId)
							{
								entity = renderEntity;
								break;
							}
						}
					}
				}//for (size_t partIndex = 0; partIndex < object.GetGameObjectParts().size(); partIndex++)

				swapData.mGameObjectResourecDesc->Pop();
			}// while (!swapData.mGameObjectResourecDesc->Empty())

			mRenderSwapContext.ResetGameObjectResourceDescData();
		}//if (swapData.mGameObjectResourecDesc.has_value())

		/*static bool assertLoad = false;
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
		}*/
	}

}