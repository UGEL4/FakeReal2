#pragma once

#include "RenderPassBase.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace FakeReal
{
	class VulkanRHI;

	enum
	{
		eMainCameraPassGBufferA,
		eMainCameraPassGBufferB,
		//eMainCameraPassGBufferC,
		eMainCameraPassDepthImage,
		eMainCameraPassSwapchainImage,
		eMainCameraPassAttachmentCount
	};

	enum
	{
		eMainCameraSubpassBasepass = 0,
		eMainCameraSubpassRenderTargetPass,
		eMainCameraSubpassCount
	};

	struct VertexData
	{
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription GetBindingDescription()
		{
			VkVertexInputBindingDescription bindingDesc = {};
			bindingDesc.binding = 0;
			bindingDesc.stride = sizeof(VertexData);
			bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDesc;
		}

		static std::vector<VkVertexInputAttributeDescription> GetVertexInputAttributeDescriptions()
		{
			std::vector<VkVertexInputAttributeDescription> descs(3);
			VkVertexInputAttributeDescription attributeDesc = {};
			descs[0].location = 0;
			descs[0].binding = 0;
			descs[0].offset = offsetof(VertexData, position);
			descs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			//descs.emplace_back(attributeDesc);

			descs[1].location = 1;
			descs[1].binding = 0;
			descs[1].offset = offsetof(VertexData, color);
			descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			//descs.emplace_back(attributeDesc);

			descs[2].location = 2;
			descs[2].binding = 0;
			descs[2].offset = offsetof(VertexData, texCoord);
			descs[2].format = VK_FORMAT_R32G32_SFLOAT;

			return descs;
		}
	};

	class RenderPass : public RenderPassBase
	{
	public:
		struct FrameBufferAttachment
		{
			VkImage			pImage;
			VkImageView		pImageView;
			VkDeviceMemory	pMemory;
			VkFormat		format;
		};
		struct FrameBuffer
		{
			int									width;
			int									height;
			VkFramebuffer						pFrameBuffer;
			VkRenderPass						pRenderPass;
			std::vector<FrameBufferAttachment>	attachments;
		};

		struct Descriptor
		{
			VkDescriptorSet			pDescriptorSet;
			VkDescriptorSetLayout	pDescriptorSetLayout;
		};

		struct RenderPipelineBase
		{
			VkPipeline			pPipeline;
			VkPipelineLayout	pPipelineLayout;
		};
	public:
		virtual void Initialize(RenderPassCommonInfo* pInfo) override;

		virtual void Draw();

	protected:
		std::shared_ptr<VulkanRHI> m_pVulkanRHI;
		FrameBuffer mFrameBuffer;
		std::vector<Descriptor> mDescriptorInfos;
		std::vector<RenderPipelineBase> mRenderPipelines;
	};
}