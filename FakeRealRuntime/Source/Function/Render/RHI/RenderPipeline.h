#pragma once
#include "Core/Base/BaseDefine.h"
namespace FakeReal
{
	class RHI;
	class RenderPass;
	class RenderPipeline
	{
	public:
		RenderPipeline();
		virtual ~RenderPipeline() = 0;

		virtual void Initialize() = 0;
		virtual void Clear() = 0;
		virtual void PreparePassData();
		virtual void DeferredRender() = 0;

		void SetRHI(SharedPtr<RHI> rhi);
	protected:
		SharedPtr<RHI> m_pRhi;
		SharedPtr<RenderPass> m_pMainCameraPass;
	};
}