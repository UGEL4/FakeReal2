#pragma once
#include "RenderPass_Vulkan.h"
#include "Core/Math/Matrix.h"

namespace FakeReal
{
	class RenderResource;

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
		virtual void PreparePassData(SharedPtr<RenderResource> pRenderResource) override final;
		virtual void PassUpdateAfterRecreateSwapchain() override final;

	public:
		struct UniformBuffer
		{
			VkBuffer pBuffer;
			VkDeviceMemory pMem;
		};
		struct UniformBufferObj
		{
			Matrix4x4 model;
			Matrix4x4 view;
			Matrix4x4 proj;
		};
		void Draw(uint32_t curSwapchainImageIndex);

	private:
		void DrawMeshGBuffer();
		void SetupAttachements();
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreateRenderPipeline();
		void CreateDescriptorSets();
		void SetupDescriptorSets();
		void CreateSwapchainFrameBuffer();

		void UpdateUniformBuffer();
	private:
		UniformBuffer mUniformBuffer;
	};
}