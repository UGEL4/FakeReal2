#pragma once

#include <memory>

namespace FakeReal
{
	class LogSystem;

	class GlobalRuntimeContext
	{
	public:
		GlobalRuntimeContext();
		~GlobalRuntimeContext();

		void InitializeSystems();
		void ShutdownSystems();

	public:
		std::shared_ptr<LogSystem> m_pLogSystem;
	};

	extern GlobalRuntimeContext g_global_runtime_context;
}