#include "FRPch.h"
#include "MainCameraPass_Vulkan.h"
#include "Function/Render/RHI/Vulkan/VulkanRHI.h"
#include "Function/Render/RHI/Vulkan/VulkanUtils.h"
#include "Function/Render/RenderMesh.h"
#include "Core/Base/Macro.h"
#include "Function/Render/stb_image.h"
#include "Function/Render/Vulkan/VulkanRenderResource.h"
#include <array>

#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

namespace FakeReal
{
	static std::vector<MeshVertex> g_Vertices =
	{
		//前
		{{-0.5f, -0.5f, 0.5f}, {1.f, 0.f, 0.f}, {0.f, 0.f} },
		{{ 0.5f, -0.5f, 0.5f}, {0.f, 1.f, 0.f}, {1.f, 0.f} },
		{{ 0.5f,  0.5f, 0.5f}, {0.f, 0.f, 1.f}, {1.f, 1.f} },
		{{-0.5f,  0.5f, 0.5f}, {1.f, 1.f, 0.f}, {0.f, 1.f} },

		//后
		{{ 0.5f, -0.5f, -0.5f}, {0.f, 1.f, 0.f}, {0.f, 0.f} },
		{{-0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f}, {1.f, 0.f} },
		{{-0.5f,  0.5f, -0.5f}, {1.f, 1.f, 0.f}, {1.f, 1.f} },
		{{ 0.5f,  0.5f, -0.5f}, {0.f, 0.f, 1.f}, {0.f, 1.f} },

		//左
		{{-0.5f,  -0.5f,  0.5f}, {1.f, 1.f, 0.f}, {1.f, 0.f} },
		{{-0.5f,   0.5f,  0.5f}, {1.f, 1.f, 0.f}, {1.f, 1.f} },
		{{-0.5f,   0.5f, -0.5f}, {1.f, 1.f, 0.f}, {0.f, 1.f} },
		{{-0.5f,  -0.5f, -0.5f}, {1.f, 1.f, 0.f}, {0.f, 0.f} },

		//右
		{{0.5f,  -0.5f,  0.5f}, {1.f, 1.f, 0.f}, {0.f, 0.f} },
		{{0.5f,  -0.5f, -0.5f}, {1.f, 1.f, 0.f}, {1.f, 0.f} },
		{{0.5f,   0.5f, -0.5f}, {1.f, 1.f, 0.f}, {1.f, 1.f} },
		{{0.5f,   0.5f,  0.5f}, {1.f, 1.f, 0.f}, {0.f, 1.f} },

		//上
		{{-0.5f,  -0.5f, -0.5f}, {1.f, 1.f, 0.f}, {0.f, 0.f} },
		{{ 0.5f,  -0.5f, -0.5f}, {1.f, 1.f, 0.f}, {1.f, 0.f} },
		{{ 0.5f,  -0.5f,  0.5f}, {1.f, 1.f, 0.f}, {1.f, 1.f} },
		{{-0.5f,  -0.5f,  0.5f}, {1.f, 1.f, 0.f}, {0.f, 1.f} },

		//下
		{{-0.5f,   0.5f,  0.5f}, {1.f, 1.f, 0.f}, {0.f, 0.f} },
		{{ 0.5f,   0.5f,  0.5f}, {1.f, 1.f, 0.f}, {1.f, 0.f} },
		{{ 0.5f,   0.5f, -0.5f}, {1.f, 1.f, 0.f}, {1.f, 1.f} },
		{{-0.5f,   0.5f, -0.5f}, {1.f, 1.f, 0.f}, {0.f, 1.f} },
	};

	static std::vector<uint16_t> g_Indices =
	{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
		8, 9, 10, 10, 11, 8,
		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20
	};
	MainCameraPass_Vulkan::MainCameraPass_Vulkan()
	{

	}

	MainCameraPass_Vulkan::~MainCameraPass_Vulkan()
	{

	}

	void MainCameraPass_Vulkan::Initialize(RenderPassCommonInfo* pInfo)
	{
		RenderPass_Vulkan::Initialize(nullptr);

		SetupAttachements();
		CreateRenderPass();
		CreateDescriptorSetLayout();
		CreateRenderPipeline();

		CreateTextureImage();
		CreateTextureImageView();
		CreateTextureSampler();
		CreateVertexBuffer();
		CreateIndexBuffer();

		CreateDescriptorSets();
		SetupDescriptorSets();
		CreateSwapchainFrameBuffer();
	}

	void MainCameraPass_Vulkan::Clear()
	{
		for (size_t i = 0; i < mFrameBuffer.mAttachments.size(); i++)
		{
			vkDestroyImage(m_pVulkanRhi->m_pDevice, mFrameBuffer.mAttachments[i].pImage, nullptr);
			vkDestroyImageView(m_pVulkanRhi->m_pDevice, mFrameBuffer.mAttachments[i].pImageView, nullptr);
			vkFreeMemory(m_pVulkanRhi->m_pDevice, mFrameBuffer.mAttachments[i].pMemory, nullptr);
		}

		for (size_t i = 0; i < mSwapchainFramebuffers.size(); i++)
		{
			vkDestroyFramebuffer(m_pVulkanRhi->m_pDevice, mSwapchainFramebuffers[i], nullptr);
		}

		for (size_t i = 0; i < mDescriptorInfos.size(); i++)
		{
			vkDestroyDescriptorSetLayout(m_pVulkanRhi->m_pDevice, mDescriptorInfos[i].pLayout, nullptr);
		}

		for (size_t i = 0; i < mPipelines.size(); i++)
		{
			vkDestroyPipeline(m_pVulkanRhi->m_pDevice, mPipelines[i].pPipeline, nullptr);
			vkDestroyPipelineLayout(m_pVulkanRhi->m_pDevice, mPipelines[i].pLayout, nullptr);
		}

		vkDestroyRenderPass(m_pVulkanRhi->m_pDevice, mFrameBuffer.pRenderPass, nullptr);
	}

	void MainCameraPass_Vulkan::PreparePassData()
	{

	}

	void MainCameraPass_Vulkan::PassUpdateAfterRecreateSwapchain()
	{
		for (size_t i = 0; i < mFrameBuffer.mAttachments.size(); i++)
		{
			vkDestroyImage(m_pVulkanRhi->m_pDevice, mFrameBuffer.mAttachments[i].pImage, nullptr);
			vkDestroyImageView(m_pVulkanRhi->m_pDevice, mFrameBuffer.mAttachments[i].pImageView, nullptr);
			vkFreeMemory(m_pVulkanRhi->m_pDevice, mFrameBuffer.mAttachments[i].pMemory, nullptr);
		}

		for (size_t i = 0; i < mSwapchainFramebuffers.size(); i++)
		{
			vkDestroyFramebuffer(m_pVulkanRhi->m_pDevice, mSwapchainFramebuffers[i], nullptr);
		}

		SetupAttachements();
		SetupDescriptorSets();
		CreateSwapchainFrameBuffer();
	}

	void MainCameraPass_Vulkan::Draw(SharedPtr<RenderResource> renderResource)
	{
		UpdateUniformBuffer();

		SharedPtr<VulkanRenderResource> vulkanRenderResource = std::static_pointer_cast<VulkanRenderResource>(renderResource);
		{

			VkRenderPassBeginInfo beginInfo = {};
			beginInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			beginInfo.renderPass	= mFrameBuffer.pRenderPass;
			beginInfo.framebuffer	= mSwapchainFramebuffers[m_pVulkanRhi->mCurSwapchainImageIndex];
			VkRect2D renderArea		= {};
			renderArea.extent		= m_pVulkanRhi->mSwapchainImageExtent;
			renderArea.offset		= { 0, 0 };
			beginInfo.renderArea	= renderArea;
			std::array<VkClearValue, 5> clearVals = {};
			clearVals[0].color = { 0.f, 0.f, 0.f, 0.f };
			clearVals[1].color = { 0.f, 0.f, 0.f, 0.f };
			clearVals[2].color = { 0.f, 0.f, 0.f, 0.f };
			clearVals[3].depthStencil	= { 1.f, 0 };
			clearVals[4].color			= { 0.f, 0.f, 0.f, 0.f };
			beginInfo.clearValueCount	= 5;
			beginInfo.pClearValues		= clearVals.data();

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width	= (float)m_pVulkanRhi->mSwapchainImageExtent.width;
			viewport.height = (float)m_pVulkanRhi->mSwapchainImageExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = m_pVulkanRhi->mSwapchainImageExtent;

			vkCmdBeginRenderPass(m_pVulkanRhi->m_pCommandBuffers[m_pVulkanRhi->mCurrFrame], &beginInfo, VK_SUBPASS_CONTENTS_INLINE); //VK_SUBPASS_CONTENTS_INLINE : 所有要执行的指令都在主要指令缓冲中，没有辅助指令缓冲需要执行

			vkCmdSetViewport(m_pVulkanRhi->m_pCommandBuffers[m_pVulkanRhi->mCurrFrame], 0, 1, &viewport);

			vkCmdSetScissor(m_pVulkanRhi->m_pCommandBuffers[m_pVulkanRhi->mCurrFrame], 0, 1, &scissor);

			{
				vkCmdBindPipeline(m_pVulkanRhi->m_pCommandBuffers[m_pVulkanRhi->mCurrFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines[RPT_MESH_GBUFFER].pPipeline);

				//test begin
				VulkanPBRMaterial& material = vulkanRenderResource->mVulkanPBRMaterials[1];
				vkCmdBindDescriptorSets(m_pVulkanRhi->m_pCurCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines[RPT_MESH_GBUFFER].pLayout, 1, 1,
					&material.m_pSet, 0, nullptr);
				VulkanMesh& mesh = vulkanRenderResource->mVulkanMeshes[1];
				//test end

				VkDeviceSize offset = 0;
				vkCmdBindVertexBuffers(m_pVulkanRhi->m_pCommandBuffers[m_pVulkanRhi->mCurrFrame], 0, 1, &m_pVertexBuffer, &offset);
				vkCmdBindIndexBuffer(m_pVulkanRhi->m_pCommandBuffers[m_pVulkanRhi->mCurrFrame], m_pIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
				vkCmdBindDescriptorSets(m_pVulkanRhi->m_pCommandBuffers[m_pVulkanRhi->mCurrFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines[RPT_MESH_GBUFFER].pLayout, 0, 1, &mDescriptorInfos[LT_PER_MESH].pSet, 0, nullptr);
				vkCmdDrawIndexed(m_pVulkanRhi->m_pCommandBuffers[m_pVulkanRhi->mCurrFrame], g_Indices.size(), 1, 0, 0, 0);
			}

			//next
			vkCmdNextSubpass(m_pVulkanRhi->m_pCommandBuffers[m_pVulkanRhi->mCurrFrame], VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdBindPipeline(m_pVulkanRhi->m_pCommandBuffers[m_pVulkanRhi->mCurrFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines[RPT_DEFERRED_LIGHTING].pPipeline);
				vkCmdBindDescriptorSets(m_pVulkanRhi->m_pCommandBuffers[m_pVulkanRhi->mCurrFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelines[RPT_DEFERRED_LIGHTING].pLayout, 0, 1, &mDescriptorInfos[LT_DEFERRED_LIGHTING].pSet, 0, nullptr);
				vkCmdDraw(m_pVulkanRhi->m_pCommandBuffers[m_pVulkanRhi->mCurrFrame], 3, 1, 0, 0);
			}

			vkCmdEndRenderPass(m_pVulkanRhi->m_pCommandBuffers[m_pVulkanRhi->mCurrFrame]);
		}
	}

	void MainCameraPass_Vulkan::SetupAttachements()
	{
		mFrameBuffer.width	= m_pVulkanRhi->mSwapchainImageExtent.width;
		mFrameBuffer.height = m_pVulkanRhi->mSwapchainImageExtent.height;
		mFrameBuffer.mAttachments.resize(eMainCameraPassCustomCount);
		mFrameBuffer.mAttachments[eMainCameraPassGBufferA].format = VK_FORMAT_R8G8B8A8_UNORM;
		mFrameBuffer.mAttachments[eMainCameraPassGBufferB].format = VK_FORMAT_R8G8B8A8_UNORM;
		mFrameBuffer.mAttachments[eMainCameraPassGBufferC].format = VK_FORMAT_R8G8B8A8_UNORM;
		for (int i = 0; i < eMainCameraPassCustomCount; i++)
		{
			VulkanUtils::CreateImage(m_pVulkanRhi->m_pPhysicalDevice, m_pVulkanRhi->m_pDevice,
				mFrameBuffer.width, mFrameBuffer.height, 1, 1, 1,
				VK_IMAGE_TYPE_2D, mFrameBuffer.mAttachments[i].format, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				mFrameBuffer.mAttachments[i].pImage, mFrameBuffer.mAttachments[i].pMemory);

			mFrameBuffer.mAttachments[i].pImageView = VulkanUtils::CreateImageView(m_pVulkanRhi->m_pDevice,
				mFrameBuffer.mAttachments[i].pImage, mFrameBuffer.mAttachments[i].format,
				VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1);
		}
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
			posotion.finalLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkAttachmentDescription& normal = descriptions[eMainCameraPassGBufferB];
			normal.format			= mFrameBuffer.mAttachments[eMainCameraPassGBufferB].format;
			normal.samples			= VK_SAMPLE_COUNT_1_BIT;
			normal.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			normal.storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			normal.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			normal.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			normal.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			normal.finalLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkAttachmentDescription& albedo = descriptions[eMainCameraPassGBufferC];
			albedo.format			= mFrameBuffer.mAttachments[eMainCameraPassGBufferC].format;
			albedo.samples			= VK_SAMPLE_COUNT_1_BIT;
			albedo.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			albedo.storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			albedo.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			albedo.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			albedo.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			albedo.finalLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkAttachmentDescription& depth = descriptions[eMainCameraPassDepth];
			depth.format			= m_pVulkanRhi->mDepthImageFormat;
			depth.samples			= VK_SAMPLE_COUNT_1_BIT;
			depth.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
			depth.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
			depth.stencilLoadOp		= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depth.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth.initialLayout		= VK_IMAGE_LAYOUT_UNDEFINED;
			depth.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		{
			VkAttachmentDescription& swapchain = descriptions[eMainCameraPassSwapchainImage];
			swapchain.format			= m_pVulkanRhi->mSwapchainImageFormat;
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
		basePassDepthAttachmentsRef.attachment	= eMainCameraPassDepth;
		basePassDepthAttachmentsRef.layout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference swapchainColorAttachmentRef = {};
		swapchainColorAttachmentRef.layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		swapchainColorAttachmentRef.attachment	= eMainCameraPassSwapchainImage;

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
		subpassDesc[0]							= {};
		subpassDesc[0].pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc[0].colorAttachmentCount		= 3;
		subpassDesc[0].pColorAttachments		= basePassColorAttachmentsRefs;
		subpassDesc[0].pDepthStencilAttachment	= &basePassDepthAttachmentsRef;

		subpassDesc[1]							= {};
		subpassDesc[1].pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc[1].colorAttachmentCount		= 1;
		subpassDesc[1].pColorAttachments		= &swapchainColorAttachmentRef;
		subpassDesc[1].inputAttachmentCount		= (uint32_t)inputAttachments.size();
		subpassDesc[1].pInputAttachments		= inputAttachments.data();

		VkSubpassDependency depens[2] = {};
		depens[0].srcSubpass		= VK_SUBPASS_EXTERNAL;
		depens[0].dstSubpass		= 0;
		depens[0].srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		depens[0].srcAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		depens[0].dstStageMask		= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		depens[0].dstAccessMask		= VK_ACCESS_SHADER_READ_BIT;
		depens[0].dependencyFlags	= 0;
		depens[1].srcSubpass		= 0;
		depens[1].dstSubpass		= 1;
		depens[1].srcStageMask		= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		depens[1].srcAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		depens[1].dstStageMask		= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		depens[1].dstAccessMask		= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		depens[1].dependencyFlags	= VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderCreateInfo = {};
		renderCreateInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderCreateInfo.pAttachments		= descriptions.data();
		renderCreateInfo.attachmentCount	= (uint32_t)descriptions.size();
		renderCreateInfo.subpassCount		= (uint32_t)subpassDesc.size();
		renderCreateInfo.pSubpasses			= subpassDesc.data();
		renderCreateInfo.dependencyCount	= sizeof(depens) / sizeof(depens[0]);
		renderCreateInfo.pDependencies		= &depens[0];

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
			std::array<VkDescriptorSetLayoutBinding, 1> writeBinding = {};
			//vertex shader uniform buffer
			writeBinding[0].binding			= 0;
			writeBinding[0].descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeBinding[0].descriptorCount = 1;
			writeBinding[0].stageFlags		= VK_SHADER_STAGE_VERTEX_BIT;

			//texture sampler
			/*writeBinding[1].binding			= 1;
			writeBinding[1].descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeBinding[1].descriptorCount = 1;
			writeBinding[1].stageFlags		= VK_SHADER_STAGE_FRAGMENT_BIT;*/

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
		{
			//mesh per material
			std::array<VkDescriptorSetLayoutBinding, 1> binding = {};
			//texture sampler
			binding[0].binding			= 1;
			binding[0].descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding[0].descriptorCount	= 1;
			binding[0].stageFlags		= VK_SHADER_STAGE_FRAGMENT_BIT;

			VkDescriptorSetLayoutCreateInfo layoutInfo = {};
			layoutInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount		= (uint32_t)binding.size();
			layoutInfo.pBindings		= binding.data();

			if (vkCreateDescriptorSetLayout(m_pVulkanRhi->m_pDevice, &layoutInfo, nullptr, &mDescriptorInfos[LT_MESH_PER_MATERIAL].pLayout) != VK_SUCCESS)
			{
				LOG_ERROR("vkCreateDescriptorSetLayout failed! Mesh per material!");
				throw std::runtime_error("vkCreateDescriptorSetLayout failed! Mesh per material!");
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

			VkShaderModule pVertexShaderModule		= VulkanUtils::CreateShader("shader/deferred_read.vert.spv", m_pVulkanRhi->m_pDevice);
			VkShaderModule pFragmentShaderModule	= VulkanUtils::CreateShader("shader/deferred_read.frag.spv", m_pVulkanRhi->m_pDevice);
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

			VkPipelineColorBlendAttachmentState colorBlendState = {};
			colorBlendState.colorWriteMask = 0xf;
			colorBlendState.blendEnable = VK_FALSE;

			VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
			colorBlendStateCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendStateCreateInfo.logicOpEnable		= VK_FALSE;
			colorBlendStateCreateInfo.logicOp			= VK_LOGIC_OP_COPY;
			colorBlendStateCreateInfo.attachmentCount	= 1;
			colorBlendStateCreateInfo.pAttachments		= &colorBlendState;

			VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
			vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

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
			graphicsPipelineCreateInfo.subpass				= 1;
			graphicsPipelineCreateInfo.layout				= mPipelines[RPT_DEFERRED_LIGHTING].pLayout;
			graphicsPipelineCreateInfo.basePipelineHandle	= VK_NULL_HANDLE;
			graphicsPipelineCreateInfo.basePipelineIndex	= -1;

			if (vkCreateGraphicsPipelines(m_pVulkanRhi->m_pDevice, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &mPipelines[RPT_DEFERRED_LIGHTING].pPipeline) != VK_SUCCESS)
			{
				LOG_ERROR("VkPipeline create failed : deferred lighting");
				throw std::runtime_error("VkPipeline create failed : deferred lighting");
			}
		}
	}

	void MainCameraPass_Vulkan::CreateDescriptorSets()
	{
		VkDescriptorSetAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		info.descriptorSetCount = 1;
		info.pSetLayouts = &mDescriptorInfos[LT_PER_MESH].pLayout;
		info.descriptorPool = m_pVulkanRhi->m_pDescriptorPool;
		if (vkAllocateDescriptorSets(m_pVulkanRhi->m_pDevice, &info, &mDescriptorInfos[LT_PER_MESH].pSet) != VK_SUCCESS)
		{
			LOG_ERROR("AllocateDescriptorSets failed : per mesh");
			throw std::runtime_error("AllocateDescriptorSets failed : per mesh");
		}
		VulkanUtils::CreateBuffer(
			m_pVulkanRhi->m_pDevice, m_pVulkanRhi->m_pPhysicalDevice,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(UniformBufferObj),
			mUniformBuffer.pBuffer,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			mUniformBuffer.pMem);

		VkDescriptorSetAllocateInfo infoRead{};
		infoRead.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		infoRead.descriptorSetCount = 1;
		infoRead.pSetLayouts = &mDescriptorInfos[LT_DEFERRED_LIGHTING].pLayout;
		infoRead.descriptorPool = m_pVulkanRhi->m_pDescriptorPool;
		if (vkAllocateDescriptorSets(m_pVulkanRhi->m_pDevice, &infoRead, &mDescriptorInfos[LT_DEFERRED_LIGHTING].pSet) != VK_SUCCESS)
		{
			LOG_ERROR("AllocateDescriptorSets failed : deferred lighting");
			throw std::runtime_error("AllocateDescriptorSets failed : deferred lighting");
		}
	}

	void MainCameraPass_Vulkan::SetupDescriptorSets()
	{
		{
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

			/*VkDescriptorImageInfo imageInfo = {};
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
			imageWriteSet.pImageInfo = &imageInfo;*/

			std::array<VkWriteDescriptorSet, 1> writes = { writeSet };
			vkUpdateDescriptorSets(m_pVulkanRhi->m_pDevice, (uint32_t)writes.size(), writes.data(), 0, nullptr);
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
			texDepth.imageView = m_pVulkanRhi->m_pDepthImageView;
			texDepth.sampler = VK_NULL_HANDLE;

			std::vector<VkWriteDescriptorSet> writeDescSets(4);
			// Binding 0
			VkWriteDescriptorSet& writePosition = writeDescSets[0];
			writePosition.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writePosition.dstSet = mDescriptorInfos[LT_DEFERRED_LIGHTING].pSet;
			writePosition.dstBinding = 0;
			writePosition.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writePosition.descriptorCount = 1;
			writePosition.pImageInfo = &texPosition;
			// Binding 1
			VkWriteDescriptorSet& writeNormal = writeDescSets[1];
			writeNormal.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeNormal.dstSet = mDescriptorInfos[LT_DEFERRED_LIGHTING].pSet;
			writeNormal.dstBinding = 1;
			writeNormal.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writeNormal.descriptorCount = 1;
			writeNormal.pImageInfo = &texNormal;
			// Binding 2
			VkWriteDescriptorSet& writeAlbedo = writeDescSets[2];
			writeAlbedo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeAlbedo.dstSet = mDescriptorInfos[LT_DEFERRED_LIGHTING].pSet;
			writeAlbedo.dstBinding = 2;
			writeAlbedo.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writeAlbedo.descriptorCount = 1;
			writeAlbedo.pImageInfo = &texAlbedo;
			// Binding 3
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

	void MainCameraPass_Vulkan::CreateSwapchainFrameBuffer()
	{
		VkImageView attachments[5] = {
			mFrameBuffer.mAttachments[0].pImageView,
			mFrameBuffer.mAttachments[1].pImageView,
			mFrameBuffer.mAttachments[2].pImageView,
			m_pVulkanRhi->m_pDepthImageView,
			m_pVulkanRhi->mSwapchainImageViews[0]
		};
		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.attachmentCount	= 5;
		createInfo.pAttachments		= attachments;
		createInfo.width			= m_pVulkanRhi->mSwapchainImageExtent.width;
		createInfo.height			= m_pVulkanRhi->mSwapchainImageExtent.height;
		createInfo.renderPass		= mFrameBuffer.pRenderPass;
		createInfo.layers			= 1;
		mSwapchainFramebuffers.resize(m_pVulkanRhi->mSwapchainImageViews.size());
		for (size_t i = 0; i < mSwapchainFramebuffers.size(); i++)
		{
			attachments[4] = m_pVulkanRhi->mSwapchainImageViews[i];
			if (vkCreateFramebuffer(m_pVulkanRhi->m_pDevice, &createInfo, nullptr, &mSwapchainFramebuffers[i]) != VK_SUCCESS)
			{
				LOG_ERROR("Swapchain framebuffer create failed!");
				throw std::runtime_error("Swapchain framebuffer create failed!");
			}
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
		VulkanUtils::TransitionImageLayout(m_pVulkanRhi, m_pTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, VK_IMAGE_ASPECT_COLOR_BIT);
		VulkanUtils::CopyBufferToImage(m_pVulkanRhi, pTempBuffer, m_pTextureImage, width, height);
		//为了能够在着色器中采样纹理图像数据，我们还要进行1次图像布局变换
		VulkanUtils::TransitionImageLayout(m_pVulkanRhi, m_pTextureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, VK_IMAGE_ASPECT_COLOR_BIT);

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

	void MainCameraPass_Vulkan::CreateVertexBuffer()
	{
		VkDeviceSize size = sizeof(g_Vertices[0]) * g_Vertices.size();
		VkBuffer pTempBuffer;
		VkDeviceMemory pTempMemory;
		VulkanUtils::CreateBuffer(
			m_pVulkanRhi->m_pDevice, m_pVulkanRhi->m_pPhysicalDevice,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, pTempBuffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, pTempMemory);

		void* pData;
		vkMapMemory(m_pVulkanRhi->m_pDevice, pTempMemory, 0, size, 0, &pData);
		memcpy(pData, g_Vertices.data(), size);
		vkUnmapMemory(m_pVulkanRhi->m_pDevice, pTempMemory);

		VulkanUtils::CreateBuffer(
			m_pVulkanRhi->m_pDevice, m_pVulkanRhi->m_pPhysicalDevice, 
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size, m_pVertexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_pVertexBufferMemory);

		VulkanUtils::CopyBuffer(m_pVulkanRhi, pTempBuffer, m_pVertexBuffer, size);

		vkDestroyBuffer(m_pVulkanRhi->m_pDevice, pTempBuffer, nullptr);
		vkFreeMemory(m_pVulkanRhi->m_pDevice, pTempMemory, nullptr);
	}

	void MainCameraPass_Vulkan::CreateIndexBuffer()
	{
		VkDeviceSize size = sizeof(g_Indices[0]) * g_Indices.size();
		VkBuffer pTempBuffer;
		VkDeviceMemory pTempMemory;
		VulkanUtils::CreateBuffer(
			m_pVulkanRhi->m_pDevice, m_pVulkanRhi->m_pPhysicalDevice, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, pTempBuffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, pTempMemory);

		void* pData;
		vkMapMemory(m_pVulkanRhi->m_pDevice, pTempMemory, 0, size, 0, &pData);
		memcpy(pData, g_Indices.data(), size);
		vkUnmapMemory(m_pVulkanRhi->m_pDevice, pTempMemory);

		VulkanUtils::CreateBuffer(
			m_pVulkanRhi->m_pDevice, m_pVulkanRhi->m_pPhysicalDevice, 
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size, m_pIndexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_pIndexBufferMemory);

		VulkanUtils::CopyBuffer(m_pVulkanRhi, pTempBuffer, m_pIndexBuffer, size);

		vkDestroyBuffer(m_pVulkanRhi->m_pDevice, pTempBuffer, nullptr);
		vkFreeMemory(m_pVulkanRhi->m_pDevice, pTempMemory, nullptr);
	}

	void MainCameraPass_Vulkan::UpdateUniformBuffer()
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currTime - startTime).count();

		UniformBufferObj ubo = {};
		ubo.model = glm::rotate(glm::mat4(1.f), glm::radians(10.f) * time, { 1.f, 1.f, 0.f });
		ubo.view = glm::lookAt(glm::vec3(0.f, 0.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
		ubo.proj = glm::perspective(glm::radians(45.f), (float)mFrameBuffer.width / mFrameBuffer.height, 0.1f, 1000.f);
		//ubo.proj[1][1] *= -1;

		void* pData;
		vkMapMemory(m_pVulkanRhi->m_pDevice, mUniformBuffer.pMem, 0, sizeof(ubo), 0, &pData);
		memcpy(pData, &ubo, sizeof(ubo));
		vkUnmapMemory(m_pVulkanRhi->m_pDevice, mUniformBuffer.pMem);
	}

}