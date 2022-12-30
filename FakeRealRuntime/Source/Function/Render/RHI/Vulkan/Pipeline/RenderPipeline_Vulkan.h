#pragma once
#include "Function/Render/RHI/RenderPipeline.h"

namespace FakeReal
{
	class RenderPipeline_Vulkan : public RenderPipeline
	{
	public:
		RenderPipeline_Vulkan();
		~RenderPipeline_Vulkan();

		virtual void Initialize() override final;
		virtual void Clear() override final;
		virtual void DeferredRender(SharedPtr<RHI> rhi, SharedPtr<RenderResource> renderResource) override final;

		void PassUpdateAfterRecreateSwapchain();
	};
}