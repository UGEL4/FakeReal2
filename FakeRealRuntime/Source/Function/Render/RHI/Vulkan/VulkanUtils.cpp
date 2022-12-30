#include "FRPch.h"
#include "VulkanUtils.h"
#include "Core/Base/Macro.h"
#include "Function/Render/RHI/Vulkan/VulkanRHI.h"
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

	void VulkanUtils::TransitionImageLayout(
		SharedPtr<VulkanRHI> pRhi, VkImage pImage, 
		VkFormat format, VkImageLayout oldLayout, 
		VkImageLayout newLayout, uint32_t mipLevels, 
		VkImageAspectFlags aspectFlags
	)
	{
		VkCommandBuffer pCommandBuffer = pRhi->BeginSingleTimeCommands();
		{
			VkImageMemoryBarrier barrier			= {};
			barrier.sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image							= pImage;
			barrier.oldLayout						= oldLayout;
			barrier.newLayout						= newLayout;
			barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;//不需要传输队列所有权，必须设置为VK_QUEUE_FAMILY_IGNORED
			barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;//不需要传输队列所有权，必须设置为VK_QUEUE_FAMILY_IGNORED
			barrier.subresourceRange.aspectMask		= aspectFlags;
			barrier.subresourceRange.levelCount		= mipLevels;
			barrier.subresourceRange.baseMipLevel	= 0;
			barrier.subresourceRange.layerCount		= 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				if (HasStencilComponent(format))
				{
					barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				}
			}

			VkPipelineStageFlags srcStageMask;
			VkPipelineStageFlags dstStageMask;
			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
				dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else
			{
				assert(0);
			}

			vkCmdPipelineBarrier(pCommandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}
		pRhi->EndSingleTimeCommands(pCommandBuffer);
	}

	bool VulkanUtils::HasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
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

	void VulkanUtils::CreateBuffer(VkDevice pDevice, VkPhysicalDevice pPhysicalDevice, VkBufferUsageFlags usage, VkDeviceSize size, VkBuffer& pBuffer, VkMemoryPropertyFlags Property, VkDeviceMemory& pMemory)
	{
		VkBufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.usage = usage;
		info.size = size;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(pDevice, &info, nullptr, &pBuffer) != VK_SUCCESS)
		{
			assert(0);
		}

		VkMemoryRequirements memoryRequirement;
		vkGetBufferMemoryRequirements(pDevice, pBuffer, &memoryRequirement);

		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequirement.size;
		allocateInfo.memoryTypeIndex = FindMemoryType(pPhysicalDevice, memoryRequirement.memoryTypeBits, Property);

		if (vkAllocateMemory(pDevice, &allocateInfo, nullptr, &pMemory) != VK_SUCCESS)
		{
			assert(0);
		}

		vkBindBufferMemory(pDevice, pBuffer, pMemory, 0);
	}

	void VulkanUtils::CopyBuffer(SharedPtr<VulkanRHI> pRhi, VkBuffer pSrcBuffer, VkBuffer pDstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer pCommandBuffer = pRhi->BeginSingleTimeCommands();
		{
			VkBufferCopy bufferCopy = {};
			bufferCopy.srcOffset = 0;
			bufferCopy.dstOffset = 0;
			bufferCopy.size = size;

			vkCmdCopyBuffer(pCommandBuffer, pSrcBuffer, pDstBuffer, 1, &bufferCopy);
		}
		pRhi->EndSingleTimeCommands(pCommandBuffer);
	}

	void VulkanUtils::CopyBufferToImage(SharedPtr<VulkanRHI> pRhi, VkBuffer pBuffer, VkImage pImage, uint32_t width, uint32_t height)
	{
		VkCommandBuffer pCommanndBuffer = pRhi->BeginSingleTimeCommands();
		{
			VkBufferImageCopy regions = {};
			regions.bufferOffset					= 0;
			regions.bufferRowLength					= 0;//bufferRowLength和bufferImageHeight成员变量用于指定数据在内存中的存放方式。通过这两个成员变量我们可以对每行图像数据使用额外的空间进行对齐。将这两个成员变量的值都设置为0，数据将会在内存中被紧凑存放。
			regions.bufferImageHeight				= 0;
			regions.imageExtent.width				= width;
			regions.imageExtent.height				= height;
			regions.imageExtent.depth				= 1;
			regions.imageOffset						= { 0, 0, 0 };
			regions.imageSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
			regions.imageSubresource.layerCount		= 1;
			regions.imageSubresource.mipLevel		= 0;
			regions.imageSubresource.baseArrayLayer = 0;
			vkCmdCopyBufferToImage(pCommanndBuffer, pBuffer, pImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &regions);
		}
		pRhi->EndSingleTimeCommands(pCommanndBuffer);
	}

	void VulkanUtils::CreateGlobalImage(SharedPtr<VulkanRHI> pRhi, VkImage& pImage, VkImageView& pImageView, VmaAllocation& pAllocation, uint32_t width, uint32_t height, FR_PIXEL_FORMAT format, const void* pPixels)
	{
		if (!pPixels)
		{
			return;
		}
		
		VkDeviceSize textureByteSize;
		VkFormat textureFormat;
		switch (format)
		{
		case FR_PIXEL_FORMAT::FR_PIXEL_FORMAT_R8G8B8_UNORM:
		{
			textureByteSize = width * height * 3;
			textureFormat	= VK_FORMAT_R8G8B8_UNORM;
		}
		break;
		case FR_PIXEL_FORMAT::FR_PIXEL_FORMAT_R8G8B8_SRGB:
		{
			textureByteSize = width * height * 3;
			textureFormat	= VK_FORMAT_R8G8B8_SRGB;
		}
		break;
		case FR_PIXEL_FORMAT::FR_PIXEL_FORMAT_R8G8B8A8_UNORM:
		{
			textureByteSize = width * height * 4;
			textureFormat	= VK_FORMAT_R8G8B8A8_UNORM;
		}
		break;
		case FR_PIXEL_FORMAT::FR_PIXEL_FORMAT_R8G8B8A8_SRGB:
		{
			textureByteSize = width * height * 4;
			textureFormat	= VK_FORMAT_R8G8B8A8_SRGB;
		}
		break;
		}

		VkBuffer pTempBuffer;
		VkDeviceMemory pTempMemory;
		VulkanUtils::CreateBuffer(pRhi->m_pDevice, pRhi->m_pPhysicalDevice,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			textureByteSize, pTempBuffer,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			pTempMemory
		);
		void* pData;
		vkMapMemory(pRhi->m_pDevice, pTempMemory, 0, textureByteSize, 0, &pData);
		memcpy(pData, pPixels, textureByteSize);
		vkUnmapMemory(pRhi->m_pDevice, pTempMemory);

		VkImageCreateInfo createInfo = {};
		createInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.imageType		= VK_IMAGE_TYPE_2D;
		createInfo.extent.width		= width;
		createInfo.extent.height	= height;
		createInfo.extent.depth		= 1;
		createInfo.mipLevels		= 1;
		createInfo.arrayLayers		= 1;
		createInfo.format			= textureFormat;
		createInfo.tiling			= VK_IMAGE_TILING_OPTIMAL;
		createInfo.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
		createInfo.usage			= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		createInfo.sharingMode		= VK_SHARING_MODE_EXCLUSIVE;
		createInfo.samples			= VK_SAMPLE_COUNT_1_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateImage(
			pRhi->m_pAssetsAllocator,
			&createInfo,
			&allocInfo,
			&pImage,
			&pAllocation,
			nullptr
		);
		//变换布局
		/*
			这里我们创建的图像对象使用VK_IMAGE_LAYOUT_UNDEFINED布局，所以转换图像布局时应该将VK_IMAGE_LAYOUT_UNDEFINED指定为旧布局.
			要注意的是我们之所以这样设置是因为我们不需要读取复制操作之前的图像内容。
		*/
		TransitionImageLayout(
			pRhi, pImage, 
			textureFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, VK_IMAGE_ASPECT_COLOR_BIT);
		CopyBufferToImage(pRhi, pTempBuffer, pImage, width, height);
		TransitionImageLayout(pRhi, pImage, textureFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, VK_IMAGE_ASPECT_COLOR_BIT);

		pImageView = CreateImageView(pRhi->m_pDevice, pImage, textureFormat, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);

		vkDestroyBuffer(pRhi->m_pDevice, pTempBuffer, nullptr);
		vkFreeMemory(pRhi->m_pDevice, pTempMemory, nullptr);
	}

	VkSampler VulkanUtils::GetOrCreateMipmapSampler(SharedPtr<VulkanRHI> pRhi, uint32_t width, uint32_t height)
	{
		{
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(pRhi->m_pPhysicalDevice, &props);

			VkSamplerCreateInfo info		= {};
			info.sType						= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.minFilter					= VK_FILTER_LINEAR;
			info.magFilter					= VK_FILTER_LINEAR;
			info.addressModeU				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeV				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeW				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.anisotropyEnable			= VK_TRUE;
			info.maxAnisotropy				= props.limits.maxSamplerAnisotropy;
			info.borderColor				= VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			info.unnormalizedCoordinates	= VK_FALSE;
			info.mipmapMode					= VK_SAMPLER_MIPMAP_MODE_LINEAR;
			info.mipLodBias					= 0.f;
			info.minLod						= 0.f;
			info.maxLod						= 0.f;
			info.compareEnable				= VK_FALSE;
			info.compareOp					= VK_COMPARE_OP_ALWAYS;

			VkSampler pSampler;
			if (vkCreateSampler(pRhi->m_pDevice, &info, nullptr, &pSampler) != VK_SUCCESS)
			{
				assert(0);
			}

			return pSampler;
		}
	}

}