#include "FRPch.h"
#include "VulkanRenderResource.h"
#include "Function/Render/RHI/Vulkan/VulkanRHI.h"
#include "Function/Render/RHI/Vulkan/VulkanUtils.h"
#include "Function/Render/RenderSystem.h"
#include "Function/Global/GlobalRuntimeContext.h"

namespace FakeReal
{

	VulkanRenderResource::VulkanRenderResource()
	{

	}

	VulkanRenderResource::~VulkanRenderResource()
	{
		
	}

	void VulkanRenderResource::ReleaseAllResources()
	{
		VulkanRHI* pVulkanRhi = static_cast<VulkanRHI*>(g_global_runtime_context.m_pRenderSystem->m_pRhi.get());
		for (auto itr : mVulkanMeshes)
		{
			vmaDestroyBuffer(pVulkanRhi->m_pAssetsAllocator, itr.second.m_pVertexBuffer, itr.second.m_pVertexBufferAllocation);
			vmaDestroyBuffer(pVulkanRhi->m_pAssetsAllocator, itr.second.m_pIndexBuffer, itr.second.m_pIndexBufferAllocation);
		}

		for (auto itr : mVulkanPBRMaterials)
		{
			vkDestroyImageView(pVulkanRhi->m_pDevice, itr.second.m_pBaseColorImageView, nullptr);
			vmaDestroyImage(pVulkanRhi->m_pAssetsAllocator, itr.second.m_pBaseColorImage, itr.second.m_pBaseColorImageAllocation);
		}
	}

	void VulkanRenderResource::UploadGameobjectRenderResource(SharedPtr<RHI> rhi, size_t asset_id, RenderMeshData& meshData)
	{
		GetOrCreateVulkanMesh(rhi, asset_id, meshData);
	}

	void VulkanRenderResource::UploadGameobjectRenderResource(SharedPtr<RHI> rhi, size_t asset_id, RenderMaterialData& materialData)
	{
		GetOrCreateVulkanPBRMaterial(rhi, asset_id, materialData);
	}

	void VulkanRenderResource::SetMaterialDescriptorSetLayout(void* pDescriptorSetLayout)
	{
		if (pDescriptorSetLayout)
		{
			VkDescriptorSetLayout pLayout	= (VkDescriptorSetLayout)pDescriptorSetLayout;
			m_pMaterialDescriptorSetLayout	= &pLayout;
		}
	}

	VulkanMesh& VulkanRenderResource::GetOrCreateVulkanMesh(SharedPtr<RHI> rhi, size_t asset_id, RenderMeshData& meshData)
	{
		std::unordered_map<size_t, VulkanMesh>::iterator itr = mVulkanMeshes.find(asset_id);
		if (itr != mVulkanMeshes.end())
		{
			return itr->second;
		}
		else
		{
			SharedPtr<VulkanRHI> pVulkanRhi = std::static_pointer_cast<VulkanRHI>(rhi);

			VulkanMesh tmp;
			auto result = mVulkanMeshes.insert(std::make_pair(asset_id, std::move(tmp)));
			assert(result.second);

			VulkanMesh& nowMesh = result.first->second;
			nowMesh.mVertexCount = (uint32_t)meshData.mStaticMeshData.mVertexBuffer->mSize / sizeof(MeshVertexDataDefinition);
			UpdateVertexBuffer(pVulkanRhi,
				(VkDeviceSize)meshData.mStaticMeshData.mVertexBuffer->mSize,
				(const MeshVertexDataDefinition*)meshData.mStaticMeshData.mVertexBuffer->m_pData,
				nowMesh
			);
			nowMesh.mIndexCount = (uint32_t)meshData.mStaticMeshData.mIndexBuffer->mSize / sizeof(uint16_t);
			UpdateIndexBuffer(pVulkanRhi,
				(VkDeviceSize)meshData.mStaticMeshData.mIndexBuffer->mSize,
				meshData.mStaticMeshData.mIndexBuffer->m_pData,
				nowMesh
			);
			
			//descriptorset

			return nowMesh;
		}
	}

	void VulkanRenderResource::UpdateVertexBuffer(SharedPtr<VulkanRHI> rhi, VkDeviceSize size, MeshVertexDataDefinition const* pVertexData, VulkanMesh& mesh)
	{
		VkBuffer pTempBuffer;
		VkDeviceMemory pTempMemory;
		VulkanUtils::CreateBuffer(
			rhi->m_pDevice, rhi->m_pPhysicalDevice,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, pTempBuffer, 
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, pTempMemory);

		void* pData;
		vkMapMemory(rhi->m_pDevice, pTempMemory, 0, size, 0, &pData);
		memcpy(pData, pVertexData, size);
		vkUnmapMemory(rhi->m_pDevice, pTempMemory);

		VkBufferCreateInfo info = {};
		info.sType	= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size	= size;
		info.usage	= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		vmaCreateBuffer(
			rhi->m_pAssetsAllocator, 
			&info, 
			&allocInfo,
			&mesh.m_pVertexBuffer,
			&mesh.m_pVertexBufferAllocation,
			nullptr
		);
		VulkanUtils::CopyBuffer(rhi, pTempBuffer, mesh.m_pVertexBuffer, size);

		vkDestroyBuffer(rhi->m_pDevice, pTempBuffer, nullptr);
		vkFreeMemory(rhi->m_pDevice, pTempMemory, nullptr);
	}

	void VulkanRenderResource::UpdateIndexBuffer(SharedPtr<VulkanRHI> rhi, VkDeviceSize size, const void* pIndexData, VulkanMesh& mesh)
	{
		VkBuffer pTempBuffer;
		VkDeviceMemory pTempMemory;
		VulkanUtils::CreateBuffer(
			rhi->m_pDevice, rhi->m_pPhysicalDevice,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, pTempBuffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, pTempMemory);

		void* pData;
		vkMapMemory(rhi->m_pDevice, pTempMemory, 0, size, 0, &pData);
		memcpy(pData, pIndexData, size);
		vkUnmapMemory(rhi->m_pDevice, pTempMemory);

		VkBufferCreateInfo info = {};
		info.sType	= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size	= size;
		info.usage	= VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		vmaCreateBuffer(
			rhi->m_pAssetsAllocator,
			&info,
			&allocInfo,
			&mesh.m_pIndexBuffer,
			&mesh.m_pIndexBufferAllocation,
			nullptr
		);
		VulkanUtils::CopyBuffer(rhi, pTempBuffer, mesh.m_pIndexBuffer, size);

		vkDestroyBuffer(rhi->m_pDevice, pTempBuffer, nullptr);
		vkFreeMemory(rhi->m_pDevice, pTempMemory, nullptr);
	}

	FakeReal::VulkanPBRMaterial& VulkanRenderResource::GetOrCreateVulkanPBRMaterial(SharedPtr<RHI> rhi, size_t asset_id, RenderMaterialData& materialData)
	{
		auto itr = mVulkanPBRMaterials.find(asset_id);
		if (itr != mVulkanPBRMaterials.end())
		{
			return itr->second;
		}
		else
		{
			SharedPtr<VulkanRHI> pVulkanRhi = std::static_pointer_cast<VulkanRHI>(rhi);

			VulkanPBRMaterial tmp;
			auto result = mVulkanPBRMaterials.insert(std::make_pair(asset_id, std::move(tmp)));
			assert(result.second);

			float emptyImage[]						= { 0.5f, 0.5f, 0.5f, 0.5f };
			void* baseColorImagePixels				= emptyImage;
			uint32_t baseColorImageWidth			= 1;
			uint32_t baseColorImageHeight			= 1;
			FR_PIXEL_FORMAT baseColorImageFormat	= FR_PIXEL_FORMAT::FR_PIXEL_FORMAT_R8G8B8A8_SRGB;
			if (materialData.mBaseColorTexture)
			{
				baseColorImagePixels = materialData.mBaseColorTexture->m_pPixels;
				baseColorImageWidth = materialData.mBaseColorTexture->mWidth;
				baseColorImageHeight = materialData.mBaseColorTexture->mHeight;
				baseColorImageFormat = materialData.mBaseColorTexture->mFormat;
			}

			VulkanPBRMaterial& nowMaterial = result.first->second;

			VulkanTextureDataToUpdate updateTextureData = {};
			updateTextureData.baseColorImagePixels = baseColorImagePixels;
			updateTextureData.baseColorImageWidth = baseColorImageWidth;
			updateTextureData.baseColorImageHeight = baseColorImageHeight;
			updateTextureData.baseColorImageFormat = baseColorImageFormat;
			updateTextureData.pMaterial = &nowMaterial;
			UpdateTextureImageData(pVulkanRhi, updateTextureData);

			VkDescriptorSetAllocateInfo info = {};
			info.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.descriptorSetCount = 1;
			info.pSetLayouts		= m_pMaterialDescriptorSetLayout;
			info.descriptorPool		= pVulkanRhi->m_pDescriptorPool;
			if (vkAllocateDescriptorSets(pVulkanRhi->m_pDevice, &info, &nowMaterial.m_pSet) != VK_SUCCESS)
			{
				throw std::runtime_error("AllocateDescriptorSets failed : material");
			}

			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView		= nowMaterial.m_pBaseColorImageView;
			imageInfo.sampler		= VulkanUtils::GetOrCreateMipmapSampler(pVulkanRhi, baseColorImageWidth, baseColorImageHeight);
			VkWriteDescriptorSet imageWriteSet = {};
			imageWriteSet.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			imageWriteSet.dstSet			= nowMaterial.m_pSet;
			imageWriteSet.dstBinding		= 1;
			imageWriteSet.dstArrayElement	= 0;
			imageWriteSet.descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			imageWriteSet.descriptorCount	= 1;
			imageWriteSet.pImageInfo		= &imageInfo;

			std::array<VkWriteDescriptorSet, 1> writes = { imageWriteSet };
			vkUpdateDescriptorSets(pVulkanRhi->m_pDevice, (uint32_t)writes.size(), writes.data(), 0, nullptr);

			return nowMaterial;
		}
	}

	void VulkanRenderResource::UpdateTextureImageData(SharedPtr<VulkanRHI> rhi, const VulkanTextureDataToUpdate& textureData)
	{
		VulkanUtils::CreateGlobalImage(
			rhi,
			textureData.pMaterial->m_pBaseColorImage,
			textureData.pMaterial->m_pBaseColorImageView,
			textureData.pMaterial->m_pBaseColorImageAllocation,
			textureData.baseColorImageWidth,
			textureData.baseColorImageHeight,
			textureData.baseColorImageFormat,
			textureData.baseColorImagePixels
		);
	}

}