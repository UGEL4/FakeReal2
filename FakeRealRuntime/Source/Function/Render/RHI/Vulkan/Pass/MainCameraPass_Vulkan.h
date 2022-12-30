#pragma once
#include "RenderPass_Vulkan.h"
#include <glm/glm.hpp>

namespace FakeReal
{
	class MainCameraPass_Vulkan : public RenderPass_Vulkan
	{
	public:
		enum LayoutType : uint8_t
		{
			LT_PER_MESH,
			LT_DEFERRED_LIGHTING,
			LT_MESH_PER_MATERIAL,
			LT_COUNT,
		};

		enum RenderPipelineType : uint8_t
		{
			RPT_MESH_GBUFFER,
			RPT_DEFERRED_LIGHTING,
			RPT_COUNT,
		};
	public:
		MainCameraPass_Vulkan();
		~MainCameraPass_Vulkan();

		virtual void Initialize(RenderPassCommonInfo* pInfo) override final;
		virtual void Clear() override final;
		virtual void PreparePassData() override final;
		virtual void PassUpdateAfterRecreateSwapchain() override final;

	public:
		struct UniformBuffer
		{
			VkBuffer pBuffer;
			VkDeviceMemory pMem;
		};
		struct UniformBufferObj
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		};
		void Draw(SharedPtr<class RenderResource> renderResource);

	private:
		void SetupAttachements();
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreateRenderPipeline();
		void CreateDescriptorSets();
		void SetupDescriptorSets();
		void CreateSwapchainFrameBuffer();

		void CreateTextureImage();
		void CreateTextureImageView();
		void CreateTextureSampler();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void UpdateUniformBuffer();
	private:
		UniformBuffer mUniformBuffer;
		VkImage m_pTextureImage;
		VkDeviceMemory m_pTextureImageMemory;
		VkImageView m_pTextureImageView;
		VkSampler m_pTextureSampler;
		VkBuffer m_pVertexBuffer;
		VkDeviceMemory m_pVertexBufferMemory;
		VkBuffer m_pIndexBuffer;
		VkDeviceMemory m_pIndexBufferMemory;
	};
}