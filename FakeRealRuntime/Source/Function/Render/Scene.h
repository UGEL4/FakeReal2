#pragma once
#include <vector>
#include <unordered_map>
#include "Function/Render/RenderObject.h"
#include "Function/Render/RenderGuidAllocator.h"
#include "Function/Render/RenderType.h"
#include "Function/Render/RenderCommon.h"
namespace FakeReal
{
	class RenderEntity;
	class RenderResource;
	class VulkanRenderResource;
	class Scene
	{
	public:
		std::vector<RenderEntity> mRenderEntities;
		std::vector<RenderMeshNode> mCameraVisibleMeshNodes;

		GuidAllocator<GameObjectPartId>& GetInstanceIdAllocator() { return mInstanceIdAllocator; }
		GuidAllocator<MeshResourceDesc>& GetMeshAssetIdAllocator() { return mMeshAssetIdAllocator; }
		GuidAllocator<MaterialResourceDesc>& GetMaterialAssetIdAllocator() { return mMaterialAssetIdAllocator; }

		void AddInstanceToMap(size_t instanceId, GObjId objId);
		void SetVisibleNodeReference();

		void UpdateVisibleObjects(SharedPtr<RenderResource> pRenderResource);
	private:
		void UpdateVisibleObjectsMainCamera(SharedPtr<RenderResource> pRenderResource);
		void VulkanUpdateVisibleObjectsMainCamera(SharedPtr<VulkanRenderResource> pRenderResource);

	private:
		GuidAllocator<GameObjectPartId> mInstanceIdAllocator;
		GuidAllocator<MeshResourceDesc> mMeshAssetIdAllocator;
		GuidAllocator<MaterialResourceDesc> mMaterialAssetIdAllocator;

		std::unordered_map<size_t, GObjId> mMeshObjIdMap;
	};
}