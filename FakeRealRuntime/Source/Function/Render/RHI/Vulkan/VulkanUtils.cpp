#include "FRPch.h"
#include "VulkanUtils.h"
#include "Core/Base/Macro.h"
#include <sstream>
#include <fstream>

namespace FakeReal
{

	void VulkanUtils::CreateImage(VkPhysicalDevice pPhysicalDevice, VkDevice pDevice,
		uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers,
		VkImageType imageType, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags propertie, 
		VkImage& pImage, VkDeviceMemory& pMemory
	)
	{
		VkImageCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.imageType = imageType;
		createInfo.extent.width = width;
		createInfo.extent.height = height;
		createInfo.extent.depth = 1;
		createInfo.mipLevels = mipLevels;
		createInfo.arrayLayers = arrayLayers;
		createInfo.format = format;
		createInfo.tiling = tiling;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		createInfo.usage = usage;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;

		if (vkCreateImage(pDevice, &createInfo, nullptr, &pImage) != VK_SUCCESS)
		{
			LOG_ERROR("Create image failed!");
			throw std::runtime_error("Create image failed!");
		}

		VkMemoryRequirements memoryRequirement;
		vkGetImageMemoryRequirements(pDevice, pImage, &memoryRequirement);

		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequirement.size;
		allocateInfo.memoryTypeIndex = FindMemoryType(pPhysicalDevice, memoryRequirement.memoryTypeBits, propertie);

		if (vkAllocateMemory(pDevice, &allocateInfo, nullptr, &pMemory) != VK_SUCCESS)
		{
			LOG_ERROR("Failed to allocate image memory!");
			throw std::runtime_error("Failed to allocate image memory!");
		}

		vkBindImageMemory(pDevice, pImage, pMemory, 0);
	}

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
			LOG_ERROR("CreateImageView");
			throw std::runtime_error("CreateImageView");
		}

		return pImageView;
	}

	uint32_t VulkanUtils::FindMemoryType(VkPhysicalDevice pPhysicalDevice, uint32_t fliter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties deviceMemoryPrpperties;
		vkGetPhysicalDeviceMemoryProperties(pPhysicalDevice, &deviceMemoryPrpperties);
		for (uint32_t i = 0; i < deviceMemoryPrpperties.memoryTypeCount; i++)
		{
			if ((deviceMemoryPrpperties.memoryTypes[i].propertyFlags &properties) == properties && (fliter & (1 << i)))
			{
				return i;
			}
		}
		LOG_ERROR("FindMemoryType");
		throw std::runtime_error("FindMemoryType");
	}

	VkShaderModule VulkanUtils::CreateShader(const std::string& fileName, VkDevice pDevice)
	{
		std::vector<char> shaderCode = ReadShaderFile(fileName);
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		VkShaderModule pShader = VK_NULL_HANDLE;
		if (vkCreateShaderModule(pDevice, &createInfo, nullptr, &pShader) != VK_SUCCESS)
		{
			LOG_ERROR("VkShaderModule create failed: {}", fileName.data());
			throw std::runtime_error("VkShaderModule create failed");
		}
		return pShader;
	}

	std::vector<char> VulkanUtils::ReadShaderFile(const std::string& fileName)
	{
		std::vector<char> data;

		std::ifstream in(fileName.data(), std::ios::ate | std::ios::binary);
		if (!in.is_open())
		{
			LOG_ERROR("Failed open file :{}", fileName.data());
			return data;
		}

		size_t size = (size_t)in.tellg();
		data.resize(size);
		in.seekg(0);
		in.read(data.data(), size);

		in.close();

		return data;
	}

}