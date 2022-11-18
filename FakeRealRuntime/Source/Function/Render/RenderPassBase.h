#pragma once

#include <memory>
namespace FakeReal
{
	class RHI;

	struct RenderPassInitInfo
	{};

	struct RenderPassCommonInfo
	{
		std::shared_ptr<RHI> rhi;
	};

	class RenderPassBase
	{
	public:
		virtual void Initialize(RenderPassCommonInfo* pInfo) = 0;
		void SetCommonInfo(RenderPassCommonInfo info);
	protected:
		std::shared_ptr<RHI> m_pRHI;
	};
}