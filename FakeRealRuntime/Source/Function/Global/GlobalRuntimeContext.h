#pragma once

#include "Core/Base/BaseDefine.h"
#include <string>

namespace FakeReal
{
	class LogSystem;
	class WindowSystem;
	class RenderSystem;
	class WorldManager;
	class ConfigManager;
	class AssetManager;

	class GlobalRuntimeContext
	{
	public:
		GlobalRuntimeContext();
		~GlobalRuntimeContext();

		void InitializeSystems(const std::string& configFilePath);
		void ShutdownSystems();

	public:
		SharedPtr<LogSystem> m_pLogSystem;
		SharedPtr<WindowSystem> m_pWindowSystem;
		SharedPtr<RenderSystem> m_pRenderSystem;
		SharedPtr<WorldManager> m_pWorldManager;
		SharedPtr<ConfigManager> m_pConfigManager;
		SharedPtr<AssetManager> m_pAssetManager;
	};

	extern GlobalRuntimeContext g_global_runtime_context;
}