#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "Core/Base/BaseDefine.h"
#include "Function/Render/RenderType.h"
namespace FakeReal
{
	class VulkanRHI;
	class VulkanUtils
	{
	public:
		static void CreateImage(VkPhysicalDevice pPhysicalDevice, VkDevice pDevice, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers,
			VkImageType imageType, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
			VkMemoryPropertyFlags propertie, VkImage& pImage, VkDeviceMemory& pMemory);
		static VkImageView CreateImageView(VkDevice pDevice, VkImage pImage, VkFormat format, VkImageViewType viewType, VkImageAspectFlags aspectMask, uint32_t layerCount, uint32_t mipLevels);
		static void TransitionImageLayout(SharedPtr<VulkanRHI> pRhi, VkImage pImage, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, VkImageAspectFlags aspectFlags);
		static bool HasStencilComponent(VkFormat format);

		static uint32_t FindMemoryType(VkPhysicalDevice pPhysicalDevice, uint32_t fliter, VkMemoryPropertyFlags properties);

		static VkShaderModule CreateShader(const std::string& fileName, VkDevice pDevice);
		static std::vector<char> ReadShaderFile(const std::string& fileName);

		static void CreateBuffer(VkDevice pDevice, VkPhysicalDevice pPhysicalDevice, VkBufferUsageFlags usage, VkDeviceSize size, VkBuffer& pBuffer, VkMemoryPropertyFlags Property, VkDeviceMemory& pMemory);
		static void CopyBuffer(SharedPtr<VulkanRHI> pRhi, VkBuffer pSrcBuffer, VkBuffer pDstBuffer, VkDeviceSize size);
		static void CopyBufferToImage(SharedPtr<VulkanRHI> pRhi, VkBuffer pBuffer, VkImage pImage, uint32_t width, uint32_t height);

		static void CreateGlobalImage(SharedPtr<VulkanRHI> pRhi, VkImage& pImage, VkImageView& pImageView, VmaAllocation& pAllocation,
			uint32_t width, uint32_t height, FR_PIXEL_FORMAT format, const void* pPixels);

		static VkSampler GetOrCreateMipmapSampler(SharedPtr<VulkanRHI> pRhi, uint32_t width, uint32_t height);
	private:
		VulkanUtils() {}
	};
}
