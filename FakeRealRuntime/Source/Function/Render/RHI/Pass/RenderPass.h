#pragma once

#include "Core/Base/BaseDefine.h"

namespace FakeReal
{
	class RHI;

	struct RenderPassInitInfo
	{};

	struct RenderPassCommonInfo
	{
		SharedPtr<RHI> rhi;
	};

	class RenderPass
	{
	public:
		RenderPass() = default;
		virtual ~RenderPass() = 0;
		virtual void Initialize(RenderPassCommonInfo* pInfo) = 0;
		void SetCommonInfo(RenderPassCommonInfo info);

	public:
	protected:
		SharedPtr<RHI> m_pRHI;
	};
}