#include "GlobalRuntimeContext.h"
#include "Core/Log/LogSystem.h"

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
	}

	void GlobalRuntimeContext::ShutdownSystems()
	{
		m_pLogSystem.reset();
	}

}