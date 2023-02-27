#pragma once
#include "Core/Base/BaseDefine.h"
namespace FakeReal
{
	class RHI;
	class RenderPass;
	class RenderResource;
	class RenderPipeline
	{
	public:
		RenderPipeline();
		virtual ~RenderPipeline() = 0;

		virtual void Initialize() = 0;
		virtual void Clear() = 0;
		virtual void PreparePassData(SharedPtr<RenderResource> pRenderResource);
		virtual void DeferredRender(SharedPtr<RHI> rhi, SharedPtr<RenderResource> renderResource) = 0;

		void SetRHI(SharedPtr<RHI> rhi);

		SharedPtr<RenderPass> GetMainCameraPass() { return m_pMainCameraPass; }
	protected:
		SharedPtr<RHI> m_pRhi;
		SharedPtr<RenderPass> m_pMainCameraPass;
	};
}