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

		friend class VulkanRenderResource;

		void Initialize(const struct RHIInitInfo& info);
		void Shutdown();
		void Tick();
		void ProcessSwapData();

		SharedPtr<RHI> GetRHI() { return m_pRhi; }
	private:
		SharedPtr<RHI> m_pRhi;
		SharedPtr<RenderPipeline> m_pPipeline;
		SharedPtr<RenderResource> m_pRenderResource;
	};
}