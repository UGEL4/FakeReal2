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
		virtual void DeferredRender() = 0;

	private:
		SharedPtr<RHI> m_pRhi;
		SharedPtr<RenderPass> m_pMainCameraPass;
	};
}