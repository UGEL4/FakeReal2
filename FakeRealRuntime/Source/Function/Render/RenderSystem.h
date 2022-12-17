#pragma once

#include <memory>
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
		std::shared_ptr<RHI> m_pRhi;
	};
}