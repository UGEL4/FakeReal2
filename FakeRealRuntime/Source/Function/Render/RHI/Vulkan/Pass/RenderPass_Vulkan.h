#pragma once

#include "Function/Render/RHI/Pass/RenderPass.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace FakeReal
{
	class VulkanRHI;

	enum
	{
		eMainCameraPassGBufferA = 0,
		eMainCameraPassGBufferB,
		eMainCameraPassGBufferC,
		eMainCameraPassCustomCount = 3,
		eMainCameraPassDepth = 3,
		eMainCameraPassSwapchainImage = 4,
		eMainCameraPassCount
	};

	class RenderPass_Vulkan : public RenderPass
	{
	public:
		RenderPass_Vulkan() = default;
		virtual ~RenderPass_Vulkan();

		virtual void Initialize(RenderPassCommonInfo* pInfo) override;
		virtual void Clear() override;

	public:
		struct Attachment
		{
			VkImage			pImage;
			VkImageView		pImageView;
			VkDeviceMemory	pMemory;
			VkFormat		format;
		};
		struct FrameBuffer
		{
			uint32_t				width;
			uint32_t				height;
			VkRenderPass			pRenderPass;
			std::vector<Attachment> mAttachments;
		};
		struct Descriptor
		{
			VkDescriptorSet			pSet;
			VkDescriptorSetLayout	pLayout;
		};
		struct RenderPipeline
		{
			VkPipeline			pPipeline;
			VkPipelineLayout	pLayout;
		};

	protected:
		FrameBuffer mFrameBuffer;
		SharedPtr<VulkanRHI> m_pVulkanRhi;
		std::vector<Descriptor> mDescriptorInfos;
		std::vector<RenderPipeline> mPipelines;
		std::vector<VkFramebuffer> mSwapchainFramebuffers;
	};
}