#pragma once

#include <memory>

namespace FakeReal
{
	class LogSystem;
	class WindowSystem;

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
	};

	extern GlobalRuntimeContext g_global_runtime_context;
}