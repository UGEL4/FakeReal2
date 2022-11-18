#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
namespace FakeReal
{
	class VulkanUtils
	{
	public:
		static void CreateImage(VkPhysicalDevice pPhysicalDevice, VkDevice pDevice, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers,
			VkImageType imageType, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
			VkMemoryPropertyFlags propertie, VkImage& pImage, VkDeviceMemory& pMemory);
		static VkImageView CreateImageView(VkDevice pDevice, VkImage pImage, VkFormat format, VkImageViewType viewType, VkImageAspectFlags aspectMask, uint32_t layerCount, uint32_t mipLevels);

		static uint32_t FindMemoryType(VkPhysicalDevice pPhysicalDevice, uint32_t fliter, VkMemoryPropertyFlags properties);

		static VkShaderModule CreateShader(const std::string& fileName, VkDevice pDevice);
		static std::vector<char> ReadShaderFile(const std::string& fileName);
	private:
		VulkanUtils() {}
	};
}
