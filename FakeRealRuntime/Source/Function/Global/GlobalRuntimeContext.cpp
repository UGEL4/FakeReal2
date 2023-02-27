#include "FRPch.h"
#include "GlobalRuntimeContext.h"
#include "Core/Log/LogSystem.h"
#include "Function/Render/WindowSystem.h"
#include "Function/Render/RenderSystem.h"
#include "Function/Render/RHI.h"
#include "Framework/World/WorldManager.h"
#include "Resource/ConfigManager/ConfigManager.h"
#include "Resource/AssetManager/AssetManager.h"

namespace FakeReal
{
	GlobalRuntimeContext g_global_runtime_context;

	GlobalRuntimeContext::GlobalRuntimeContext()
	{

	}

	GlobalRuntimeContext::~GlobalRuntimeContext()
	{

	}

	void GlobalRuntimeContext::InitializeSystems(const std::string& configFilePath)
	{
		m_pLogSystem = MakeShared<LogSystem>();

		m_pConfigManager = MakeShared<ConfigManager>();
		m_pConfigManager->Initialize(configFilePath);

		m_pAssetManager = MakeShared<AssetManager>();

		m_pWorldManager = MakeShared<WorldManager>();
		m_pWorldManager->Initialize();

		m_pWindowSystem = MakeShared<WindowSystem>();
		m_pWindowSystem->Initialize({ 1280, 720, false, "FR Engine" });

		RHIInitInfo rhi_info{ m_pWindowSystem };
		m_pRenderSystem = MakeShared<RenderSystem>();
		m_pRenderSystem->Initialize(rhi_info);
	}

	void GlobalRuntimeContext::ShutdownSystems()
	{
		m_pRenderSystem->Shutdown();
		m_pRenderSystem.reset();

		m_pWindowSystem->Shutdown();
		m_pWindowSystem.reset();

		m_pWorldManager->Clear();
		m_pWorldManager.reset();

		m_pLogSystem.reset();
	}

}