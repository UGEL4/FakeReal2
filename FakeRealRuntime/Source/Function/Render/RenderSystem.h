#pragma once

#include "Core/Base/BaseDefine.h"
namespace FakeReal
{
	class RHI;
	class RenderPipeline;
	class RenderResource;
	class RenderSystem
	{
	public:
		RenderSystem();
		~RenderSystem();

		void Initialize(const struct RHIInitInfo& info);
		void Shutdown();
		void Tick();
		void ProcessSwapData();

	private:
		SharedPtr<RHI> m_pRhi;
		SharedPtr<RenderPipeline> m_pPipeline;
		SharedPtr<RenderResource> m_pRenderResource;
	};
}