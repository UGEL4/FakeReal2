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
		virtual void DeferredRender() override final;
	};
}