#include "FRPch.h"
#include "MainCameraPass_Vulkan.h"
#include "Function/Render/RHI/Vulkan/VulkanRHI.h"
#include "Function/Render/RHI/Vulkan/VulkanUtils.h"
#include "Function/Render/RenderMesh.h"
#include "Core/Base/Macro.h"
#include "Function/Render/stb_image.h"
#include <array>

namespace FakeReal
{
	MainCameraPass_Vulkan::MainCameraPass_Vulkan()
	{

	}

	MainCameraPass_Vulkan::~MainCameraPass_Vulkan()
	{

	}

	void MainCameraPass_Vulkan::Initialize(RenderPassCommonInfo* pInfo)
	{
		RenderPass_Vulkan::Initialize(nullptr);
	}

	void MainCameraPass_Vulkan::SetupAttachements()
	{
		mFrameBuffer.width	= m_pVulkanRhi->mSwapchainImageExtent.width;
		mFrameBuffer.height = m_pVulkanRhi->mSwapchainImageExtent.height;
		mFrameBuffer.mAttachments.resize(eMainCameraPassDepth + 1);
		mFrameBuffer.mAttachments[eMainCameraPassGBufferA].format = VK_FORMAT_R8G8B8A8_UNORM;
		mFrameBuffer.mAttachments[eMainCameraPassGBufferB].format = VK_FORMAT_R8G8B8A8_UNORM;
		mFrameBuffer.mAttachments[eMainCameraPassGBufferC].format = VK_FORMAT_R8G8B8A8_UNORM;
		for (int i = 0; i < eMainCameraPassDepth; i++)
		{
			VulkanUtils::CreateImage(m_pVulkanRhi->m_pPhysicalDevice, m_pVulkanRhi->m_pDevice,
				mFrameBuffer.width, mFrameBuffer.height, 1, 1, 1,
				VK_IMAGE_TYPE_2D, mFrameBuffer.mAttachments[i].format, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				mFrameBuffer.mAttachments[i].pImage, mFrameBuffer.mAttachments[i].pMemory);

			VulkanUtils::CreateImageView(m_pVulkanRhi->m_pDevice,
				mFrameBuffer.mAttachments[i].pImage, mFrameBuffer.mAttachments[i].format,
				VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);
		}
		mFrameBuffer.mAttachments[eMainCameraPassDepth].format		= m_pVulkanRhi->mDepthImageFormat;
		mFrameBuffer.mAttachments[eMainCameraPassDepth].pImage		= m_pVulkanRhi->m_pDepthImage;
		mFrameBuffer.mAttachments[eMainCameraPassDepth].pMemory		= m_pVulkanRhi->m_pDepthImageMemory;
		mFrameBuffer.mAttachments[eMainCameraPassDepth].pImageView	= m_pVulkanRhi->m_pDepthImageView;
	}

	void MainCameraPass_Vulkan::CreateRenderPass()
	{
		std::array<VkAttachmentDescription, eMainCameraPassCount> descriptions = {};
		{
			VkAttachmentDescription& posotion = descriptions[eMainCameraPassGBufferA];
			posotion.format				= mFrameBuffer.mAttachments[eMainCameraPassGBufferA].format;
			posotion.samples			= VK_SAMPLE_COUNT_1_BIT;
			posotion.loadOp				= VK_ATTACHMENT_LOAD_OP_CLEAR;
			posotion.storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			posotion.stencilLoadOp		= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			posotion.stencilStoreOp		= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			posotion.initialLayout		= VK_IMAGE_LAYOUT_UNDEFINED;
			posotion.finalLayout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription& normal = descriptions[eMainCameraPassGBufferB];
			normal.format			= mFrameBuffer.mAttachments[eMainCameraPassGBufferB].format;
			normal.samples			= VK_SAMPLE_COUNT_1_BIT;
			normal.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			normal.storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			normal.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			normal.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			normal.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			normal.finalLayout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription& albedo = descriptions[eMainCameraPassGBufferC];
			albedo.format			= mFrameBuffer.mAttachments[eMainCameraPassGBufferC].format;
			albedo.samples			= VK_SAMPLE_COUNT_1_BIT;
			albedo.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			albedo.storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			albedo.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			albedo.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			albedo.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			albedo.finalLayout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription& depth = descriptions[eMainCameraPassDepth];
			depth.format			= mFrameBuffer.mAttachments[eMainCameraPassDepth].format;
			depth.samples			= VK_SAMPLE_COUNT_1_BIT;
			depth.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			depth.storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth.stencilLoadOp		= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depth.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth.initialLayout		= VK_IMAGE_LAYOUT_UNDEFINED;
			depth.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		{
			VkAttachmentDescription& swapchain = descriptions[eMainCameraPassSwapchainImage];
			swapchain.format			= mFrameBuffer.mAttachments[eMainCameraPassSwapchainImage].format;
			swapchain.samples			= VK_SAMPLE_COUNT_1_BIT;
			swapchain.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			swapchain.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
			swapchain.stencilLoadOp		= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			swapchain.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			swapchain.initialLayout		= VK_IMAGE_LAYOUT_UNDEFINED;
			swapchain.finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}

		VkAttachmentReference basePassColorAttachmentsRefs[3];
		basePassColorAttachmentsRefs[eMainCameraPassGBufferA].attachment	= eMainCameraPassGBufferA;
		basePassColorAttachmentsRefs[eMainCameraPassGBufferA].layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		basePassColorAttachmentsRefs[eMainCameraPassGBufferB].attachment	= eMainCameraPassGBufferB;
		basePassColorAttachmentsRefs[eMainCameraPassGBufferB].layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		basePassColorAttachmentsRefs[eMainCameraPassGBufferC].attachment	= eMainCameraPassGBufferC;
		basePassColorAttachmentsRefs[eMainCameraPassGBufferC].layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference basePassDepthAttachmentsRef = {};
		basePassDepthAttachmentsRef.attachment = eMainCameraPassDepth;
		basePassDepthAttachmentsRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference swapchainColorAttachmentRef = {};
		swapchainColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		swapchainColorAttachmentRef.attachment = eMainCameraPassSwapchainImage;

		std::array<VkAttachmentReference, 4> inputAttachments;
		inputAttachments[eMainCameraPassGBufferA].layout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		inputAttachments[eMainCameraPassGBufferA].attachment	= eMainCameraPassGBufferA;
		inputAttachments[eMainCameraPassGBufferB].layout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		inputAttachments[eMainCameraPassGBufferB].attachment	= eMainCameraPassGBufferB;
		inputAttachments[eMainCameraPassGBufferC].layout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		inputAttachments[eMainCameraPassGBufferC].attachment	= eMainCameraPassGBufferC;
		inputAttachments[eMainCameraPassDepth].layout			= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		inputAttachments[eMainCameraPassDepth].attachment		= eMainCameraPassDepth;

		std::array<VkSubpassDescription, 2> subpassDesc;
		subpassDesc[0] = {};
		subpassDesc[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc[0].colorAttachmentCount = 3;
		subpassDesc[0].pColorAttachments = basePassColorAttachmentsRefs;
		subpassDesc[0].pDepthStencilAttachment = &basePassDepthAttachmentsRef;

		subpassDesc[1] = {};
		subpassDesc[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc[1].colorAttachmentCount = 1;
		subpassDesc[1].pColorAttachments = &swapchainColorAttachmentRef;
		subpassDesc[1].inputAttachmentCount = (uint32_t)inputAttachments.size();
		subpassDesc[1].pInputAttachments = inputAttachments.data();

		VkSubpassDependency depens = {};
		depens.srcSubpass		= 0;
		depens.dstSubpass		= 1;
		depens.srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		depens.srcAccessMask	= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		depens.dstStageMask		= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		depens.dstAccessMask	= VK_ACCESS_SHADER_READ_BIT;
		depens.dependencyFlags	= VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderCreateInfo = {};
		renderCreateInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderCreateInfo.pAttachments		= descriptions.data();
		renderCreateInfo.attachmentCount	= (uint32_t)descriptions.size();
		renderCreateInfo.subpassCount		= (uint32_t)subpassDesc.size();
		renderCreateInfo.pSubpasses			= subpassDesc.data();
		renderCreateInfo.dependencyCount	= 1;
		renderCreateInfo.pDependencies		= &depens;

		if (vkCreateRenderPass(m_pVulkanRhi->m_pDevice, &renderCreateInfo, nullptr, &mFrameBuffer.pRenderPass) != VK_SUCCESS)
		{
			LOG_ERROR("VkRenderPass create failed!");
			throw std::runtime_error("VkRenderPass create failed!");
		}
	}

	void MainCameraPass_Vulkan::CreateDescriptorSetLayout()
	{
		mDescriptorInfos.resize(LT_COUNT);
		{
			//per mesh
			std::array<VkDescriptorSetLayoutBinding, 2> writeBinding = {};
			//vertex shader uniform buffer
			writeBinding[0].binding			= 0;
			writeBinding[0].descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeBinding[0].descriptorCount = 1;
			writeBinding[0].stageFlags		= VK_SHADER_STAGE_VERTEX_BIT;

			//texture sampler
			writeBinding[1].binding			= 1;
			writeBinding[1].descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeBinding[1].descriptorCount = 1;
			writeBinding[1].stageFlags		= VK_SHADER_STAGE_FRAGMENT_BIT;

			VkDescriptorSetLayoutCreateInfo writeLayoutInfo = {};
			writeLayoutInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			writeLayoutInfo.bindingCount	= (uint32_t)writeBinding.size();
			writeLayoutInfo.pBindings		= writeBinding.data();

			if (vkCreateDescriptorSetLayout(m_pVulkanRhi->m_pDevice, &writeLayoutInfo, nullptr, &mDescriptorInfos[LT_PER_MESH].pLayout) != VK_SUCCESS)
			{
				LOG_ERROR("vkCreateDescriptorSetLayout failed! PerMesh!");
				throw std::runtime_error("vkCreateDescriptorSetLayout failed! PerMesh!");
			}
		}
		{
			//deferred lighting
			std::vector<VkDescriptorSetLayoutBinding> deferredBinding(4);
			//position
			deferredBinding[0].binding			= 0;
			deferredBinding[0].descriptorType	= VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			deferredBinding[0].descriptorCount	= 1;
			deferredBinding[0].stageFlags		= VK_SHADER_STAGE_FRAGMENT_BIT;
			//normal
			deferredBinding[1].binding			= 1;
			deferredBinding[1].descriptorType	= VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			deferredBinding[1].descriptorCount	= 1;
			deferredBinding[1].stageFlags		= VK_SHADER_STAGE_FRAGMENT_BIT;
			//albedo
			deferredBinding[2].binding			= 2;
			deferredBinding[2].descriptorType	= VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			deferredBinding[2].descriptorCount	= 1;
			deferredBinding[2].stageFlags		= VK_SHADER_STAGE_FRAGMENT_BIT;
			//depth
			deferredBinding[3].binding			= 3;
			deferredBinding[3].descriptorType	= VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			deferredBinding[3].descriptorCount	= 1;
			deferredBinding[3].stageFlags		= VK_SHADER_STAGE_FRAGMENT_BIT;

			VkDescriptorSetLayoutCreateInfo deferredLayoutInfo = {};
			deferredLayoutInfo.sType		= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			deferredLayoutInfo.bindingCount = (uint32_t)deferredBinding.size();
			deferredLayoutInfo.pBindings	= deferredBinding.data();

			if (vkCreateDescriptorSetLayout(m_pVulkanRhi->m_pDevice, &deferredLayoutInfo, nullptr, &mDescriptorInfos[LT_DEFERRED_LIGHTING].pLayout) != VK_SUCCESS)
			{
				LOG_ERROR("vkCreateDescriptorSetLayout failed! Deferred lighting!");
				throw std::runtime_error("vkCreateDescriptorSetLayout failed! Deferred lighting!");
			}
		}
	}

	void MainCameraPass_Vulkan::CreateRenderPipeline()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo = {};
		inputAssemblyStateInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyStateInfo.topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyStateInfo.primitiveRestartEnable	= VK_FALSE;

		VkViewport viewPort = {};
		viewPort.x			= 0.f;
		viewPort.y			= 0.f;
		viewPort.width		= (float)mFrameBuffer.width;
		viewPort.height		= (float)mFrameBuffer.height;
		viewPort.minDepth	= 0.f;
		viewPort.maxDepth	= 1.f;

		VkRect2D rect = {};
		rect.offset = { 0, 0 };
		rect.extent = { mFrameBuffer.width, mFrameBuffer.height };

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
		viewportStateCreateInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.pViewports		= &viewPort;
		viewportStateCreateInfo.viewportCount	= 1;
		viewportStateCreateInfo.pScissors		= &rect;
		viewportStateCreateInfo.scissorCount	= 1;

		VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = {};
		rasterizationStateInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateInfo.rasterizerDiscardEnable	= VK_FALSE;
		rasterizationStateInfo.polygonMode				= VK_POLYGON_MODE_FILL;
		rasterizationStateInfo.lineWidth				= 1.f;
		rasterizationStateInfo.frontFace				= VK_FRONT_FACE_CLOCKWISE;
		rasterizationStateInfo.cullMode					= VK_CULL_MODE_BACK_BIT;
		rasterizationStateInfo.depthClampEnable			= VK_FALSE;
		rasterizationStateInfo.depthBiasEnable			= VK_FALSE;
		rasterizationStateInfo.depthBiasClamp			= 0.f;
		rasterizationStateInfo.depthBiasConstantFactor	= 0.f;
		rasterizationStateInfo.depthBiasSlopeFactor		= 0.f;

		VkPipelineMultisampleStateCreateInfo multisampleStateInfo = {};
		multisampleStateInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateInfo.sampleShadingEnable	= VK_FALSE;
		multisampleStateInfo.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;
		multisampleStateInfo.minSampleShading		= 0.f;
		multisampleStateInfo.pSampleMask			= nullptr;
		multisampleStateInfo.alphaToCoverageEnable	= VK_FALSE;
		multisampleStateInfo.alphaToOneEnable		= VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo depthInfo = {};
		depthInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthInfo.depthTestEnable		= VK_TRUE;
		depthInfo.depthWriteEnable		= VK_TRUE;
		depthInfo.depthCompareOp		= VK_COMPARE_OP_LESS;
		depthInfo.depthBoundsTestEnable = VK_FALSE;
		depthInfo.stencilTestEnable		= VK_FALSE;

		VkDynamicState dynamicState[] =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
		dynamicStateCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.dynamicStateCount	= 2;
		dynamicStateCreateInfo.pDynamicStates		= dynamicState;

		mPipelines.resize(RPT_COUNT);
		{
			//per mesh
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
			pipelineLayoutCreateInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.setLayoutCount			= 1;
			pipelineLayoutCreateInfo.pSetLayouts			= &mDescriptorInfos[LT_PER_MESH].pLayout;
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
			pipelineLayoutCreateInfo.pPushConstantRanges	= nullptr;

			if (vkCreatePipelineLayout(m_pVulkanRhi->m_pDevice, &pipelineLayoutCreateInfo, nullptr, &mPipelines[RPT_MESH_GBUFFER].pLayout) != VK_SUCCESS)
			{
				LOG_ERROR("VkPipelineLayout create failed : per mesh");
				throw std::runtime_error("VkPipelineLayout create failed : per mesh");
			}

			VkShaderModule pVertexShaderModule		= VulkanUtils::CreateShader("shader/deferred_write.vert.spv", m_pVulkanRhi->m_pDevice);
			VkShaderModule pFragmentShaderModule	= VulkanUtils::CreateShader("shader/deferred_write.frag.spv", m_pVulkanRhi->m_pDevice);
			VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
			vertexShaderCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertexShaderCreateInfo.module				= pVertexShaderModule;
			vertexShaderCreateInfo.stage				= VK_SHADER_STAGE_VERTEX_BIT;
			vertexShaderCreateInfo.pName				= "main";
			vertexShaderCreateInfo.pSpecializationInfo	= nullptr;

			VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
			fragmentShaderCreateInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragmentShaderCreateInfo.module					= pFragmentShaderModule;
			fragmentShaderCreateInfo.stage					= VK_SHADER_STAGE_FRAGMENT_BIT;
			fragmentShaderCreateInfo.pName					= "main";
			fragmentShaderCreateInfo.pSpecializationInfo	= nullptr;
			VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

			VkPipelineColorBlendAttachmentState colorBlendState[3] = {};
			colorBlendState[0].colorWriteMask	= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;//0xf;
			colorBlendState[0].blendEnable		= VK_FALSE;
			colorBlendState[1]					= colorBlendState[0];
			colorBlendState[2]					= colorBlendState[0];

			VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
			colorBlendStateCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendStateCreateInfo.logicOpEnable		= VK_FALSE;
			colorBlendStateCreateInfo.logicOp			= VK_LOGIC_OP_COPY;
			colorBlendStateCreateInfo.attachmentCount	= 3;
			colorBlendStateCreateInfo.pAttachments		= colorBlendState;

			VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
			vertexInputStateInfo.sType								= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			auto attributeDescriptions								= MeshVertex::GetVulkanVertexInputAttributeDescriptions();
			auto bindingDescriptions								= MeshVertex::GetVulkanVertexBindingDescription();
			vertexInputStateInfo.vertexAttributeDescriptionCount	= (uint32_t)attributeDescriptions.size();
			vertexInputStateInfo.pVertexAttributeDescriptions		= attributeDescriptions.data();
			vertexInputStateInfo.vertexBindingDescriptionCount		= (uint32_t)bindingDescriptions.size();
			vertexInputStateInfo.pVertexBindingDescriptions			= bindingDescriptions.data();

			VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
			graphicsPipelineCreateInfo.sType				= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			graphicsPipelineCreateInfo.stageCount			= 2;
			graphicsPipelineCreateInfo.pStages				= shaderStages;
			graphicsPipelineCreateInfo.pColorBlendState		= &colorBlendStateCreateInfo;
			graphicsPipelineCreateInfo.pDepthStencilState	= &depthInfo;
			graphicsPipelineCreateInfo.pDynamicState		= &dynamicStateCreateInfo;
			graphicsPipelineCreateInfo.pInputAssemblyState	= &inputAssemblyStateInfo;
			graphicsPipelineCreateInfo.pVertexInputState	= &vertexInputStateInfo;
			graphicsPipelineCreateInfo.pMultisampleState	= &multisampleStateInfo;
			graphicsPipelineCreateInfo.pRasterizationState	= &rasterizationStateInfo;
			graphicsPipelineCreateInfo.pTessellationState	= nullptr;
			graphicsPipelineCreateInfo.pViewportState		= &viewportStateCreateInfo;
			graphicsPipelineCreateInfo.renderPass			= mFrameBuffer.pRenderPass;
			graphicsPipelineCreateInfo.subpass				= 0;
			graphicsPipelineCreateInfo.layout				= mPipelines[RPT_MESH_GBUFFER].pLayout;
			graphicsPipelineCreateInfo.basePipelineHandle	= VK_NULL_HANDLE;
			graphicsPipelineCreateInfo.basePipelineIndex	= -1;

			if (vkCreateGraphicsPipelines(m_pVulkanRhi->m_pDevice, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &mPipelines[RPT_MESH_GBUFFER].pPipeline) != VK_SUCCESS)
			{
				LOG_ERROR("VkPipeline create failed : per mesh");
				throw std::runtime_error("VkPipeline create failed : per mesh");
			}
		}
		{
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
			pipelineLayoutCreateInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.setLayoutCount			= 1;
			pipelineLayoutCreateInfo.pSetLayouts			= &mDescriptorInfos[RPT_DEFERRED_LIGHTING].pLayout;
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
			pipelineLayoutCreateInfo.pPushConstantRanges	= nullptr;

			if (vkCreatePipelineLayout(m_pVulkanRhi->m_pDevice, &pipelineLayoutCreateInfo, nullptr, &mPipelines[RPT_DEFERRED_LIGHTING].pLayout) != VK_SUCCESS)
			{
				LOG_ERROR("VkPipelineLayout create failed : deferred lighting");
				throw std::runtime_error("VkPipelineLayout create failed : deferred lighting");
			}

			VkShaderModule pVertexShaderModule		= VulkanUtils::CreateShader("shader/deferred_write.vert.spv", m_pVulkanRhi->m_pDevice);
			VkShaderModule pFragmentShaderModule	= VulkanUtils::CreateShader("shader/deferred_write.frag.spv", m_pVulkanRhi->m_pDevice);
			VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
			vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertexShaderCreateInfo.module = pVertexShaderModule;
			vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertexShaderCreateInfo.pName = "main";
			vertexShaderCreateInfo.pSpecializationInfo = nullptr;

			VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
			fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragmentShaderCreateInfo.module = pFragmentShaderModule;
			fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragmentShaderCreateInfo.pName = "main";
			fragmentShaderCreateInfo.pSpecializationInfo = nullptr;
			VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

			VkPipelineColorBlendAttachmentState colorBlendState = {};
			colorBlendState.colorWriteMask = 0xf;
			colorBlendState.blendEnable = VK_FALSE;

			VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
			colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
			colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
			colorBlendStateCreateInfo.attachmentCount = 1;
			colorBlendStateCreateInfo.pAttachments = &colorBlendState;

			VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
			vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
			graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			graphicsPipelineCreateInfo.stageCount = 2;
			graphicsPipelineCreateInfo.pStages = shaderStages;
			graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
			graphicsPipelineCreateInfo.pDepthStencilState = &depthInfo;
			graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
			graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateInfo;
			graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateInfo;
			graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateInfo;
			graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateInfo;
			graphicsPipelineCreateInfo.pTessellationState = nullptr;
			graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
			graphicsPipelineCreateInfo.renderPass = mFrameBuffer.pRenderPass;
			graphicsPipelineCreateInfo.subpass = 1;
			graphicsPipelineCreateInfo.layout = mPipelines[RPT_DEFERRED_LIGHTING].pLayout;
			graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
			graphicsPipelineCreateInfo.basePipelineIndex = -1;

			if (vkCreateGraphicsPipelines(m_pVulkanRhi->m_pDevice, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &mPipelines[RPT_DEFERRED_LIGHTING].pPipeline) != VK_SUCCESS)
			{
				LOG_ERROR("VkPipeline create failed : deferred lighting");
				throw std::runtime_error("VkPipeline create failed : deferred lighting");
			}
		}
	}

	void MainCameraPass_Vulkan::CreateDescriptorSets()
	{
		{
			VkDescriptorSetAllocateInfo info{};
			info.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			info.descriptorSetCount = 1;
			info.pSetLayouts		= &mDescriptorInfos[LT_PER_MESH].pLayout;
			info.descriptorPool		= m_pVulkanRhi->m_pDescriptorPool;
			if (vkAllocateDescriptorSets(m_pVulkanRhi->m_pDevice, &info, &mDescriptorInfos[LT_PER_MESH].pSet) != VK_SUCCESS)
			{
				LOG_ERROR("AllocateDescriptorSets failed : per mesh");
			}

			VulkanUtils::CreateBuffer(
				m_pVulkanRhi->m_pDevice, m_pVulkanRhi->m_pPhysicalDevice,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(UniformBufferObj),
				mUniformBuffer.pBuffer, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				mUniformBuffer.pMem);
			VkDescriptorBufferInfo modelBufferinfo = {};
			modelBufferinfo.buffer = mUniformBuffer.pBuffer;
			modelBufferinfo.offset = 0;
			modelBufferinfo.range = sizeof(UniformBufferObj);
			VkWriteDescriptorSet writeSet = {};
			writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeSet.dstSet = mDescriptorInfos[LT_PER_MESH].pSet;
			writeSet.dstBinding = 0;
			writeSet.dstArrayElement = 0;
			writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeSet.descriptorCount = 1;
			writeSet.pBufferInfo = &modelBufferinfo;

			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = m_pTextureImageView;
			imageInfo.sampler = m_pTextureSampler;
			VkWriteDescriptorSet imageWriteSet = {};
			imageWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			imageWriteSet.dstSet = mDescriptorInfos[LT_PER_MESH].pSet;
			imageWriteSet.dstBinding = 1;
			imageWriteSet.dstArrayElement = 0;
			imageWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			imageWriteSet.descriptorCount = 1;
			imageWriteSet.pImageInfo = &imageInfo;

			VkWriteDescriptorSet writes[] = { writeSet, imageWriteSet };
			vkUpdateDescriptorSets(m_pVulkanRhi->m_pDevice, 2, writes, 0, nullptr);
		}

		{
			//读
			// Image descriptors for the offscreen color attachments
			VkDescriptorImageInfo texPosition{};
			texPosition.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			texPosition.imageView = mFrameBuffer.mAttachments[0].pImageView;
			texPosition.sampler = VK_NULL_HANDLE;
			VkDescriptorImageInfo texNormal{};
			texNormal.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			texNormal.imageView = mFrameBuffer.mAttachments[1].pImageView;
			texNormal.sampler = VK_NULL_HANDLE;
			VkDescriptorImageInfo texAlbedo{};
			texAlbedo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			texAlbedo.imageView = mFrameBuffer.mAttachments[2].pImageView;
			texAlbedo.sampler = VK_NULL_HANDLE;
			VkDescriptorImageInfo texDepth = {};
			texDepth.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			texDepth.imageView = mFrameBuffer.mAttachments[3].pImageView;
			texDepth.sampler = VK_NULL_HANDLE;

			VkDescriptorSetAllocateInfo infoRead{};
			infoRead.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			infoRead.descriptorSetCount = 1;
			infoRead.pSetLayouts = &mDescriptorInfos[LT_DEFERRED_LIGHTING].pLayout;
			infoRead.descriptorPool = m_pVulkanRhi->m_pDescriptorPool;
			if (vkAllocateDescriptorSets(m_pVulkanRhi->m_pDevice, &infoRead, &mDescriptorInfos[LT_DEFERRED_LIGHTING].pSet) != VK_SUCCESS)
			{
				assert(0);
			}
			std::vector<VkWriteDescriptorSet> writeDescSets(4);
			// Binding 0 : Position texture target
			VkWriteDescriptorSet& writePosition = writeDescSets[0];
			writePosition.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writePosition.dstSet = mDescriptorInfos[LT_DEFERRED_LIGHTING].pSet;
			writePosition.dstBinding = 0;
			writePosition.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writePosition.descriptorCount = 1;
			writePosition.pImageInfo = &texPosition;
			// Binding 1 : Normals texture target
			VkWriteDescriptorSet& writeNormal = writeDescSets[1];
			writeNormal.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeNormal.dstSet = mDescriptorInfos[LT_DEFERRED_LIGHTING].pSet;
			writeNormal.dstBinding = 1;
			writeNormal.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writeNormal.descriptorCount = 1;
			writeNormal.pImageInfo = &texNormal;
			// Binding 2 : Albedo texture target
			VkWriteDescriptorSet& writeAlbedo = writeDescSets[2];
			writeAlbedo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeAlbedo.dstSet = mDescriptorInfos[LT_DEFERRED_LIGHTING].pSet;
			writeAlbedo.dstBinding = 2;
			writeAlbedo.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writeAlbedo.descriptorCount = 1;
			writeAlbedo.pImageInfo = &texAlbedo;
			// Binding 3 : Depth texture target
			VkWriteDescriptorSet& writeDepth = writeDescSets[3];
			writeDepth.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDepth.dstSet = mDescriptorInfos[LT_DEFERRED_LIGHTING].pSet;
			writeDepth.dstBinding = 3;
			writeDepth.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writeDepth.descriptorCount = 1;
			writeDepth.pImageInfo = &texDepth;
			vkUpdateDescriptorSets(m_pVulkanRhi->m_pDevice, writeDescSets.size(), writeDescSets.data(), 0, nullptr);
		}
	}

	void MainCameraPass_Vulkan::CreateTextureImage()
	{
		int width, height, comp = 0;
		unsigned char* data = stbi_load("texture/huaji.jpg", &width, &height, &comp, STBI_rgb_alpha);
		VkDeviceSize imageSize = width * height * 4;
		if (!data)
		{
			assert(0);
		}

		VkBuffer pTempBuffer;
		VkDeviceMemory pTempMemory;
		VulkanUtils::CreateBuffer(m_pVulkanRhi->m_pDevice, m_pVulkanRhi->m_pPhysicalDevice, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			imageSize, pTempBuffer, 
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
			pTempMemory);
		void* pData;
		vkMapMemory(m_pVulkanRhi->m_pDevice, pTempMemory, 0, imageSize, 0, &pData);
		memcpy(pData, data, imageSize);
		vkUnmapMemory(m_pVulkanRhi->m_pDevice, pTempMemory);

		stbi_image_free(data);

		VulkanUtils::CreateImage(m_pVulkanRhi->m_pPhysicalDevice,
			m_pVulkanRhi->m_pDevice, width, height, 1, 1, 1, VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_pTextureImage, m_pTextureImageMemory);

		//变换布局
		/*
			这里我们创建的图像对象使用VK_IMAGE_LAYOUT_UNDEFINED布局，所以转换图像布局时应该将VK_IMAGE_LAYOUT_UNDEFINED指定为旧布局.
			要注意的是我们之所以这样设置是因为我们不需要读取复制操作之前的图像内容。
		*/
		VulkanUtils::TransitionImageLayout(m_pVulkanRhi, m_pTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
		VulkanUtils::CopyBufferToImage(m_pVulkanRhi, pTempBuffer, m_pTextureImage, width, height);
		//为了能够在着色器中采样纹理图像数据，我们还要进行1次图像布局变换
		VulkanUtils::TransitionImageLayout(m_pVulkanRhi, m_pTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

		vkDestroyBuffer(m_pVulkanRhi->m_pDevice, pTempBuffer, nullptr);
		vkFreeMemory(m_pVulkanRhi->m_pDevice, pTempMemory, nullptr);
	}

	void MainCameraPass_Vulkan::CreateTextureImageView()
	{
		m_pTextureImageView = VulkanUtils::CreateImageView(
			m_pVulkanRhi->m_pDevice,
			m_pTextureImage,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_VIEW_TYPE_2D,
			VK_IMAGE_ASPECT_COLOR_BIT,
			1,
			1);
	}

	void MainCameraPass_Vulkan::CreateTextureSampler()
	{
		VkSamplerCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.minFilter = VK_FILTER_LINEAR;
		info.magFilter = VK_FILTER_LINEAR;
		info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.anisotropyEnable = VK_TRUE;
		info.maxAnisotropy = 16.f;
		info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		info.unnormalizedCoordinates = VK_FALSE;
		info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		info.mipLodBias = 0.f;
		info.minLod = 0.f;
		info.maxLod = 0.f;
		info.compareEnable = VK_FALSE;
		info.compareOp = VK_COMPARE_OP_ALWAYS;

		if (vkCreateSampler(m_pVulkanRhi->m_pDevice, &info, nullptr, &m_pTextureSampler) != VK_SUCCESS)
		{
			assert(0);
		}
	}

}