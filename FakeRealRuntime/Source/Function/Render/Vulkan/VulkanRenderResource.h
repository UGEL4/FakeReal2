#pragma once
#include "Function/Render/RenderResource.h"
#include "Function/Render/RenderCommon.h"
#include <unordered_map>

namespace FakeReal
{
	struct VulkanMesh;
	class VulkanRHI;
	class VulkanRenderResource : public RenderResource
	{
	public:
		VulkanRenderResource();
		~VulkanRenderResource();

	public:
		virtual void ReleaseAllResources() override final;
		virtual void UploadGameobjectRenderResource(SharedPtr<RHI> rhi, size_t asset_id, RenderMeshData& meshData) override final;
		virtual void UploadGameobjectRenderResource(SharedPtr<RHI> rhi, size_t asset_id, RenderMaterialData& materialData) override final;
		virtual void SetMaterialDescriptorSetLayout(void* pDescriptorSetLayout) override final;

	private:
		VulkanMesh& GetOrCreateVulkanMesh(SharedPtr<RHI> rhi, size_t asset_id, RenderMeshData& meshData);
		void UpdateVertexBuffer(SharedPtr<VulkanRHI> rhi, VkDeviceSize size, MeshVertexDataDefinition const* pVertexData, VulkanMesh& mesh);
		void UpdateIndexBuffer(SharedPtr<VulkanRHI> rhi, VkDeviceSize size, const void* pIndexData, VulkanMesh& mesh);

		VulkanPBRMaterial& GetOrCreateVulkanPBRMaterial(SharedPtr<RHI> rhi, size_t asset_id, RenderMaterialData& materialData);
		void UpdateTextureImageData(SharedPtr<VulkanRHI> rhi, const VulkanTextureDataToUpdate& textureData);
	public:
		//cache
		std::unordered_map<size_t, VulkanMesh> mVulkanMeshes;
		std::unordered_map<size_t, VulkanPBRMaterial> mVulkanPBRMaterials;

		const VkDescriptorSetLayout* m_pMaterialDescriptorSetLayout{ nullptr };
	};
}