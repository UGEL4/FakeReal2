#pragma once

#include "Core/Base/BaseDefine.h"
namespace FakeReal
{
	class RHI;
	class RenderSystem
	{
	public:
		RenderSystem();
		~RenderSystem();

		void Initialize(const struct RHIInitInfo& info);
		void Shutdown();
		void Tick();

	private:
		SharedPtr<RHI> m_pRhi;
	};
}