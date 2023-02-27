#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_mem_alloc.h>
#include "Core/Math/Matrix.h"
#include "Function/Render/RenderType.h"

namespace FakeReal
{
	struct VulkanMesh
	{
		uint32_t		mVertexCount;
		VkBuffer		m_pVertexBuffer;
		VmaAllocation	m_pVertexBufferAllocation;

		uint32_t		mIndexCount;
		VkBuffer		m_pIndexBuffer;
		VmaAllocation	m_pIndexBufferAllocation;
	};

	struct VulkanPBRMaterial
	{
		VkImage			m_pBaseColorImage;
		VkImageView		m_pBaseColorImageView;
		VmaAllocation	m_pBaseColorImageAllocation;

		VkDescriptorSet m_pSet;
	};

	struct VulkanTextureDataToUpdate
	{
		void*			baseColorImagePixels;
		uint32_t		baseColorImageWidth;
		uint32_t		baseColorImageHeight;
		FR_PIXEL_FORMAT baseColorImageFormat;

		VulkanPBRMaterial* pMaterial;
	};

	struct RenderMeshNode
	{
		const Matrix4x4* modelMatrix;
		void* pMesh;
		void* pMaterial;
		uint32_t nodeId;
	};
}