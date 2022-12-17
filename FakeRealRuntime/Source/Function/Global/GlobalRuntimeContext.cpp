#include "FRPch.h"
#include "GlobalRuntimeContext.h"
#include "Core/Log/LogSystem.h"
#include "Function/Render/WindowSystem.h"
#include "Function/Render/RenderSystem.h"
#include "Function/Render/RHI.h"

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

		RHIInitInfo rhi_info{ m_pWindowSystem };
		m_pRenderSystem = std::make_shared<RenderSystem>();
		m_pRenderSystem->Initialize(rhi_info);
	}

	void GlobalRuntimeContext::ShutdownSystems()
	{
		m_pRenderSystem->Shutdown();
		m_pRenderSystem.reset();

		m_pWindowSystem->Shutdown();
		m_pWindowSystem.reset();

		m_pLogSystem.reset();
	}

}