#include "FRPch.h"
#include "RenderPassBase.h"

namespace FakeReal
{
	void RenderPassBase::SetCommonInfo(RenderPassCommonInfo info)
	{
		m_pRHI = info.rhi;
	}

}