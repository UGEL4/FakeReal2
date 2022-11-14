#pragma once

#include <vulkan/vulkan.h>
namespace FakeReal
{
	class VulkanUtils
	{
	public:
		static VkImageView CreateImageView(VkDevice pDevice, VkImage pImage, VkFormat format, VkImageViewType viewType, VkImageAspectFlags aspectMask, uint32_t layerCount, uint32_t mipLevels);
	private:
		VulkanUtils() {}
	};
}
