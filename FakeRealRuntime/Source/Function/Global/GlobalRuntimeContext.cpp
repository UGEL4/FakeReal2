#include "FRPch.h"
#include "GlobalRuntimeContext.h"
#include "Core/Log/LogSystem.h"
#include "Function/Render/WindowSystem.h"

namespace FakeReal
{
	GlobalRuntimeContext g_global_runtime_context;

	GlobalRuntimeContext::GlobalRuntimeContext()
	{

	}

	GlobalRuntimeContext::~GlobalRuntimeContext()
	{

	}

	void GlobalRuntimeContext::InitializeSystems()
	{
		m_pLogSystem = std::make_shared<LogSystem>();

		m_pWindowSystem = std::make_shared<WindowSystem>();
		m_pWindowSystem->Initialize({ 1280, 720, false, "FR Engine" });
	}

	void GlobalRuntimeContext::ShutdownSystems()
	{
		m_pWindowSystem->Shutdown();
		m_pWindowSystem.reset();

		m_pLogSystem.reset();
	}

}