#include "FRPch.h"
#include "Resource/AssetManager/AssetManager.h"
#include "Resource/ConfigManager/ConfigManager.h"
#include "Function/Global/GlobalRuntimeContext.h"

namespace FakeReal
{
	std::filesystem::path AssetManager::GetFullPath(const std::string& relativePath) const
	{
		return std::filesystem::absolute(g_global_runtime_context.m_pConfigManager->GetRootFolder() / relativePath);
	}

}