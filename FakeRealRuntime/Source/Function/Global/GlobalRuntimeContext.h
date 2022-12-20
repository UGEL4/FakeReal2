#pragma once

#include "Core/Base/BaseDefine.h"

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
		SharedPtr<LogSystem> m_pLogSystem;
		SharedPtr<WindowSystem> m_pWindowSystem;
		SharedPtr<RenderSystem> m_pRenderSystem;
	};

	extern GlobalRuntimeContext g_global_runtime_context;
}