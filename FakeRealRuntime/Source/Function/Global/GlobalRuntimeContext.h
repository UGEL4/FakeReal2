#pragma once

#include <memory>

namespace FakeReal
{
	class LogSystem;
	class WindowSystem;
	class RenderSystem;

	class GlobalRuntimeContext
	{
	public:
		GlobalRuntimeContext();
		~GlobalRuntimeContext();

		void InitializeSystems();
		void ShutdownSystems();

	public:
		std::shared_ptr<LogSystem> m_pLogSystem;
		std::shared_ptr<WindowSystem> m_pWindowSystem;
		std::shared_ptr<RenderSystem> m_pRenderSystem;
	};

	extern GlobalRuntimeContext g_global_runtime_context;
}