#include "FRPch.h"
#include "VulkanUtils.h"
#include "Core/Base/Macro.h"

namespace FakeReal
{
	VkImageView VulkanUtils::CreateImageView(
		VkDevice pDevice, 
		VkImage pImage, 
		VkFormat format, 
		VkImageViewType viewType, 
		VkImageAspectFlags aspectMask, 
		uint32_t layerCount, 
		uint32_t mipLevels
	)
	{
		VkImageView pImageView;

		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = pImage;
		createInfo.format = format;
		createInfo.viewType = viewType;
		createInfo.subresourceRange.aspectMask = aspectMask;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = mipLevels;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = layerCount;

		if (vkCreateImageView(pDevice, &createInfo, nullptr, &pImageView) != VK_SUCCESS)
		{
			ASSERT(0);
		}

		return pImageView;
	}

}