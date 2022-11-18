#include "FRPch.h"
#include "MainCameraPass.h"
#include "Function/Render/RHI/Vulkan/VulkanRHI.h"
#include "Function/Render/RHI/Vulkan/VulkanUtils.h"
#include "Core/Base/Macro.h"

namespace FakeReal
{
	MainCameraPass::MainCameraPass()
	{

	}

	MainCameraPass::~MainCameraPass()
	{

	}

	void MainCameraPass::Initialize(RenderPassCommonInfo* pInfo)
	{
		RenderPass::Initialize(nullptr);
	}

	void MainCameraPass::Draw()
	{

	}

	void MainCameraPass::SetupAttachements()
	{
		mFrameBuffer.attachments.resize(eMainCameraPassAttachmentCount);
		mFrameBuffer.attachments[eMainCameraPassGBufferA].format = VK_FORMAT_R8G8B8A8_UNORM;
		mFrameBuffer.attachments[eMainCameraPassGBufferB].format = VK_FORMAT_R8G8B8A8_UNORM;

		VulkanUtils::CreateImage(
			m_pVulkanRHI->m_pPhysicalDevice,
			m_pVulkanRHI->m_pDevice,
			m_pVulkanRHI->mSwapchainImageExtent.width, m_pVulkanRHI->mSwapchainImageExtent.height, 1, 1, 1,
			VK_IMAGE_TYPE_2D, mFrameBuffer.attachments[eMainCameraPassGBufferA].format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mFrameBuffer.attachments[eMainCameraPassGBufferA].pImage,
			mFrameBuffer.attachments[eMainCameraPassGBufferA].pMemory
		);
		mFrameBuffer.attachments[eMainCameraPassGBufferA].pImageView = VulkanUtils::CreateImageView(
			m_pVulkanRHI->m_pDevice,
			mFrameBuffer.attachments[eMainCameraPassGBufferA].pImage,
			mFrameBuffer.attachments[eMainCameraPassGBufferA].format,
			VK_IMAGE_VIEW_TYPE_2D,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1,
			1
		);

		VulkanUtils::CreateImage(
			m_pVulkanRHI->m_pPhysicalDevice,
			m_pVulkanRHI->m_pDevice,
			m_pVulkanRHI->mSwapchainImageExtent.width, m_pVulkanRHI->mSwapchainImageExtent.height, 1, 1, 1,
			VK_IMAGE_TYPE_2D, mFrameBuffer.attachments[eMainCameraPassGBufferB].format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			mFrameBuffer.attachments[eMainCameraPassGBufferB].pImage,
			mFrameBuffer.attachments[eMainCameraPassGBufferB].pMemory
		);
		mFrameBuffer.attachments[eMainCameraPassGBufferB].pImageView = VulkanUtils::CreateImageView(
			m_pVulkanRHI->m_pDevice,
			mFrameBuffer.attachments[eMainCameraPassGBufferB].pImage,
			mFrameBuffer.attachments[eMainCameraPassGBufferB].format,
			VK_IMAGE_VIEW_TYPE_2D,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1,
			1
		);
	}

	void MainCameraPass::CreateRenderPass()
	{
		std::vector<VkAttachmentDescription> attachments(eMainCameraPassAttachmentCount);

		//depth
		attachments[eMainCameraPassDepthImage].format			= m_pVulkanRHI->mDepthImageFormat;
		attachments[eMainCameraPassDepthImage].samples			= VK_SAMPLE_COUNT_1_BIT;
		attachments[eMainCameraPassDepthImage].loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[eMainCameraPassDepthImage].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
		attachments[eMainCameraPassDepthImage].stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[eMainCameraPassDepthImage].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[eMainCameraPassDepthImage].initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[eMainCameraPassDepthImage].finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		//normal
		attachments[eMainCameraPassGBufferA].format = mFrameBuffer.attachments[eMainCameraPassGBufferA].format;
		attachments[eMainCameraPassGBufferA].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[eMainCameraPassGBufferA].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[eMainCameraPassGBufferA].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[eMainCameraPassGBufferA].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[eMainCameraPassGBufferA].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[eMainCameraPassGBufferA].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[eMainCameraPassGBufferA].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		//diffuse albedo
		attachments[eMainCameraPassGBufferB].format = mFrameBuffer.attachments[eMainCameraPassGBufferA].format;
		attachments[eMainCameraPassGBufferB].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[eMainCameraPassGBufferB].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[eMainCameraPassGBufferB].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[eMainCameraPassGBufferB].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[eMainCameraPassGBufferB].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[eMainCameraPassGBufferB].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[eMainCameraPassGBufferB].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		//swapchain
		attachments[eMainCameraPassSwapchainImage].format			= m_pVulkanRHI->mSwapchainImageFormat;
		attachments[eMainCameraPassSwapchainImage].samples			= VK_SAMPLE_COUNT_1_BIT;
		attachments[eMainCameraPassSwapchainImage].loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[eMainCameraPassSwapchainImage].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
		attachments[eMainCameraPassSwapchainImage].stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[eMainCameraPassSwapchainImage].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[eMainCameraPassSwapchainImage].initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[eMainCameraPassSwapchainImage].finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference basePassColorAttachmentsRefs[2];
		basePassColorAttachmentsRefs[0].attachment = (uint32_t)eMainCameraPassGBufferA;//normal
		basePassColorAttachmentsRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		basePassColorAttachmentsRefs[1].attachment = (uint32_t)eMainCameraPassGBufferB;//diffuse albedo
		basePassColorAttachmentsRefs[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference basePassDepthAttachmentsRef = {};
		basePassDepthAttachmentsRef.attachment = (uint32_t)eMainCameraPassDepthImage;
		basePassDepthAttachmentsRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass[eMainCameraSubpassCount];
		subpass[eMainCameraSubpassBasepass].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass[eMainCameraSubpassBasepass].colorAttachmentCount = sizeof(basePassColorAttachmentsRefs) / sizeof(basePassColorAttachmentsRefs[0]);
		subpass[eMainCameraSubpassBasepass].pColorAttachments = basePassColorAttachmentsRefs;
		subpass[eMainCameraSubpassBasepass].pDepthStencilAttachment = &basePassDepthAttachmentsRef;

		VkAttachmentReference swapchainPassAtachmentRef = {};
		swapchainPassAtachmentRef.attachment = (uint32_t)eMainCameraPassSwapchainImage;
		swapchainPassAtachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//inputattachment
		VkAttachmentReference swapchainPassInputAtachmentRef[3];
		swapchainPassInputAtachmentRef[0].attachment = 0;//normal
		swapchainPassInputAtachmentRef[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		swapchainPassInputAtachmentRef[1].attachment = 0;//diffuse albedo
		swapchainPassInputAtachmentRef[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		swapchainPassInputAtachmentRef[2].attachment = 0;//depth
		swapchainPassInputAtachmentRef[2].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkSubpassDescription& renderSubpass = subpass[eMainCameraSubpassRenderTargetPass];
		renderSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		renderSubpass.colorAttachmentCount = 1;
		renderSubpass.pColorAttachments = &swapchainPassAtachmentRef;
		renderSubpass.inputAttachmentCount = 3;
		renderSubpass.pInputAttachments = swapchainPassInputAtachmentRef;

		std::vector<VkSubpassDependency> dependencies(1);
		dependencies[0].srcSubpass = eMainCameraSubpassBasepass;
		dependencies[0].dstSubpass = eMainCameraSubpassRenderTargetPass;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderCreateInfo = {};
		renderCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderCreateInfo.pAttachments = attachments.data();
		renderCreateInfo.attachmentCount = attachments.size();
		renderCreateInfo.subpassCount = eMainCameraSubpassCount;
		renderCreateInfo.pSubpasses = subpass;
		renderCreateInfo.dependencyCount = dependencies.size();
		renderCreateInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(m_pVulkanRHI->m_pDevice, &renderCreateInfo, nullptr, &mFrameBuffer.pRenderPass) != VK_SUCCESS)
		{
			LOG_ERROR("VkRenderPass create failed!");
			throw std::runtime_error("VkRenderPass create failed!");
		}
	}

	void MainCameraPass::CreateDescriptorSetLayout()
	{
		mDescriptorInfos.resize(1);

		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding binds[] = { uboLayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo uboLayoutInfo = {};
		uboLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		uboLayoutInfo.bindingCount = 2;
		uboLayoutInfo.pBindings = binds;

		if (vkCreateDescriptorSetLayout(m_pVulkanRHI->m_pDevice, &uboLayoutInfo, nullptr, &mDescriptorInfos[0].pDescriptorSetLayout) != VK_SUCCESS)
		{
			LOG_ERROR("uboLayoutBinding create failed!");
			throw std::runtime_error("uboLayoutBinding create failed!");
		}

		/*VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo samplerBindingInfo = {};
		samplerBindingInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		samplerBindingInfo.bindingCount = 1;
		samplerBindingInfo.pBindings = &samplerLayoutBinding;

		if (vkCreateDescriptorSetLayout(m_pVulkanRHI->m_pDevice, &samplerBindingInfo, nullptr, &mDescriptorInfos[1].pDescriptorSetLayout) != VK_SUCCESS)
		{
			LOG_ERROR("samplerLayoutBinding create failed!");
			throw std::runtime_error("samplerLayoutBinding create failed!");
		}*/
	}

	void MainCameraPass::CreateRenderPipeline()
	{
		mRenderPipelines.resize(1);
		//gbuffer
		{
			VkDescriptorSetLayout layouts[] = { mDescriptorInfos[0].pDescriptorSetLayout};

			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
			pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.setLayoutCount = 1;
			pipelineLayoutCreateInfo.pSetLayouts = layouts;
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
			pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

			if (vkCreatePipelineLayout(m_pVulkanRHI->m_pDevice, &pipelineLayoutCreateInfo, nullptr, &mRenderPipelines[0].pPipelineLayout) != VK_SUCCESS)
			{
				LOG_ERROR("VkPipelineLayout create failed!");
				throw std::runtime_error("VkPipelineLayout create failed!");
				return;
			}

			VkShaderModule pVsShader = VulkanUtils::CreateShader("shader/vert.spv", m_pVulkanRHI->m_pDevice);
			VkShaderModule pPsShader = VulkanUtils::CreateShader("shader/frag.spv", m_pVulkanRHI->m_pDevice);
			VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
			vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertexShaderCreateInfo.module = pVsShader;
			vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertexShaderCreateInfo.pName = "main";
			vertexShaderCreateInfo.pSpecializationInfo = nullptr;

			VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
			fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragmentShaderCreateInfo.module = pPsShader;
			fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragmentShaderCreateInfo.pName = "main";
			fragmentShaderCreateInfo.pSpecializationInfo = nullptr;
			VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

			VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
			vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			auto descriptions = VertexData::GetVertexInputAttributeDescriptions();
			vertexInputStateInfo.vertexAttributeDescriptionCount = (uint32_t)descriptions.size();
			vertexInputStateInfo.pVertexAttributeDescriptions = descriptions.data();
			vertexInputStateInfo.vertexBindingDescriptionCount = 1;
			vertexInputStateInfo.pVertexBindingDescriptions = &VertexData::GetBindingDescription();

			VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo = {};
			inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssemblyStateInfo.primitiveRestartEnable = VK_FALSE;

			VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
			viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportStateCreateInfo.pViewports = &m_pVulkanRHI->mViewport;
			viewportStateCreateInfo.viewportCount = 1;
			viewportStateCreateInfo.pScissors = &m_pVulkanRHI->mScissor;
			viewportStateCreateInfo.scissorCount = 1;

			VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = {};
			rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
			rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizationStateInfo.lineWidth = 1.f;
			rasterizationStateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizationStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizationStateInfo.depthClampEnable = VK_FALSE;
			rasterizationStateInfo.depthBiasEnable = VK_FALSE;
			rasterizationStateInfo.depthBiasClamp = 0.f;
			rasterizationStateInfo.depthBiasConstantFactor = 0.f;
			rasterizationStateInfo.depthBiasSlopeFactor = 0.f;

			VkPipelineMultisampleStateCreateInfo multisampleStateInfo = {};
			multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampleStateInfo.sampleShadingEnable = VK_FALSE;
			multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampleStateInfo.minSampleShading = 0.f;
			multisampleStateInfo.pSampleMask = nullptr;
			multisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
			multisampleStateInfo.alphaToOneEnable = VK_FALSE;

			VkPipelineColorBlendAttachmentState colorBlendState = {};
			colorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendState.blendEnable = VK_FALSE;

			VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
			colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
			colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
			colorBlendStateCreateInfo.attachmentCount = 1;
			colorBlendStateCreateInfo.pAttachments = &colorBlendState;
			colorBlendStateCreateInfo.blendConstants[0] = 0.f;
			colorBlendStateCreateInfo.blendConstants[1] = 0.f;
			colorBlendStateCreateInfo.blendConstants[2] = 0.f;
			colorBlendStateCreateInfo.blendConstants[3] = 0.f;

			VkPipelineDepthStencilStateCreateInfo depthInfo = {};
			depthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthInfo.depthTestEnable = VK_TRUE;
			depthInfo.depthWriteEnable = VK_TRUE;
			depthInfo.depthCompareOp = VK_COMPARE_OP_LESS;
			depthInfo.depthBoundsTestEnable = VK_FALSE;
			depthInfo.stencilTestEnable = VK_FALSE;

			VkDynamicState dynamicState[] =
			{
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};
			VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
			dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicStateCreateInfo.dynamicStateCount = 2;
			dynamicStateCreateInfo.pDynamicStates = dynamicState;

			VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
			graphicsPipelineCreateInfo.sType				= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			graphicsPipelineCreateInfo.stageCount			= 2;
			graphicsPipelineCreateInfo.pStages				= shaderStages;
			graphicsPipelineCreateInfo.pColorBlendState		= &colorBlendStateCreateInfo;
			graphicsPipelineCreateInfo.pDepthStencilState	= &depthInfo;
			//graphicsPipelineCreateInfo.pDynamicState		= &dynamicStateCreateInfo;
			graphicsPipelineCreateInfo.pInputAssemblyState	= &inputAssemblyStateInfo;
			graphicsPipelineCreateInfo.pMultisampleState	= &multisampleStateInfo;
			graphicsPipelineCreateInfo.pRasterizationState	= &rasterizationStateInfo;
			graphicsPipelineCreateInfo.pTessellationState	= nullptr;
			graphicsPipelineCreateInfo.pVertexInputState	= &vertexInputStateInfo;
			graphicsPipelineCreateInfo.pViewportState		= &viewportStateCreateInfo;
			graphicsPipelineCreateInfo.renderPass			= mFrameBuffer.pRenderPass;
			graphicsPipelineCreateInfo.subpass				= eMainCameraSubpassBasepass;
			graphicsPipelineCreateInfo.layout				= mRenderPipelines[0].pPipelineLayout;
			graphicsPipelineCreateInfo.basePipelineHandle	= VK_NULL_HANDLE;
			graphicsPipelineCreateInfo.basePipelineIndex	= -1;

			if (vkCreateGraphicsPipelines(m_pVulkanRHI->m_pDevice, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &mRenderPipelines[0].pPipeline) != VK_SUCCESS)
			{
				LOG_ERROR("VkPipeline create failed!");
				throw std::runtime_error("VkPipeline create failed!");
			}

			vkDestroyShaderModule(m_pVulkanRHI->m_pDevice, pVsShader, nullptr);
			vkDestroyShaderModule(m_pVulkanRHI->m_pDevice, pPsShader, nullptr);
		}
	}

	void MainCameraPass::CreateSwapchainFrameBuffer()
	{
		mSwapchainFrameBuffers.resize(m_pVulkanRHI->mSwapchainImageViews.size());
		for (size_t i = 0; i < mSwapchainFrameBuffers.size(); i++)
		{
			VkImageView attachments[] = {
				mFrameBuffer.attachments[eMainCameraPassGBufferA].pImageView,
				mFrameBuffer.attachments[eMainCameraPassGBufferB].pImageView,
				m_pVulkanRHI->m_pDepthImageView,
				m_pVulkanRHI->mSwapchainImageViews[i]
			};
			VkFramebufferCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createInfo.attachmentCount = 4;
			createInfo.pAttachments = attachments;
			createInfo.width = m_pVulkanRHI->mSwapchainImageExtent.width;
			createInfo.height = m_pVulkanRHI->mSwapchainImageExtent.height;
			createInfo.renderPass = mFrameBuffer.pRenderPass;
			createInfo.layers = 1;

			if (vkCreateFramebuffer(m_pVulkanRHI->m_pDevice, &createInfo, nullptr, &mSwapchainFrameBuffers[i]) != VK_SUCCESS)
			{
				LOG_ERROR("VkFramebuffer {} create failed!", i);
				throw std::runtime_error("VkFramebuffer create failed!");
			}
		}
	}

}
