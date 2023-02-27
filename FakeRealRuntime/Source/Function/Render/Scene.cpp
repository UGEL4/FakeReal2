#include "FRPch.h"
#include "Function/Render/Scene.h"
#include "Function/Render/RenderEntity.h"
#include "Function/Render/RHI/Pass/RenderPass.h"
#include "Function/Render/Vulkan/VulkanRenderResource.h"

namespace FakeReal
{
	
	void Scene::AddInstanceToMap(size_t instanceId, GObjId objId)
	{
		mMeshObjIdMap[instanceId] = objId;
	}

	void Scene::SetVisibleNodeReference()
	{
		RenderPass::mVisibleNodes.pMainCameraVisibleMeshNodes = &mCameraVisibleMeshNodes;
	}

	void Scene::UpdateVisibleObjects(SharedPtr<RenderResource> pRenderResource)
	{
		UpdateVisibleObjectsMainCamera(pRenderResource);
	}

	void Scene::UpdateVisibleObjectsMainCamera(SharedPtr<RenderResource> pRenderResource)
	{
		//if vulkan
		SharedPtr<VulkanRenderResource> pVulkanRenderResource = StaticPointerCast<VulkanRenderResource>(pRenderResource);
		VulkanUpdateVisibleObjectsMainCamera(pVulkanRenderResource);
	}

	void Scene::VulkanUpdateVisibleObjectsMainCamera(SharedPtr<VulkanRenderResource> pRenderResource)
	{
		mCameraVisibleMeshNodes.clear();
		for (const RenderEntity& entity : mRenderEntities)
		{
			//todo: pickup visible object
			mCameraVisibleMeshNodes.emplace_back();

			RenderMeshNode& node = mCameraVisibleMeshNodes.back();
			node.modelMatrix = &entity.mModelMatrix;

			node.nodeId = entity.mInstanceId;

			VulkanMesh& pVulkanMesh = pRenderResource->GetEntityMesh(entity);
			node.pMesh = &pVulkanMesh;

			VulkanPBRMaterial& pVulkanMaterial = pRenderResource->GetEntityMaterial(entity);
			node.pMaterial = &pVulkanMaterial;
		}
	}

}