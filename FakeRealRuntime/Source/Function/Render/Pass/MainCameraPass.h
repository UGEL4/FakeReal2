#pragma once

#include "Function/Render/RenderPass.h"

namespace FakeReal
{
	class MainCameraPass : public RenderPass
	{
	public:
		MainCameraPass();
		~MainCameraPass();

		virtual void Initialize(RenderPassCommonInfo* pInfo) override final;

	public:
		void Draw();

	private:
		void SetupAttachements();
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreateRenderPipeline();
		void CreateSwapchainFrameBuffer();

	private:
		std::vector<VkFramebuffer> mSwapchainFrameBuffers;
	};
}