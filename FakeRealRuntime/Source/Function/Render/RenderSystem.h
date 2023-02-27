#pragma once

#include "Core/Base/BaseDefine.h"
#include "Function/Render/RenderSwapContext.h"

namespace FakeReal
{
	class RHI;
	class RenderPipeline;
	class RenderResource;
	class Scene;
	class RenderSystem
	{
	public:
		RenderSystem();
		~RenderSystem();

		friend class VulkanRenderResource;

		void Initialize(const struct RHIInitInfo& info);
		void Shutdown();
		void Tick();
		void SwapRenderData();
		void ProcessSwapData();

		SharedPtr<RHI> GetRHI() { return m_pRhi; }
		RenderSwapContext& GetRenderSwapContext() { return mRenderSwapContext; }
	private:
		SharedPtr<RHI> m_pRhi;
		SharedPtr<RenderPipeline> m_pPipeline;
		SharedPtr<RenderResource> m_pRenderResource;
		SharedPtr<Scene> m_pRenderScene;

		RenderSwapContext mRenderSwapContext;
	};
}