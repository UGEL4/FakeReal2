#pragma once
#include <filesystem>
#include <string>

namespace FakeReal
{
	class ConfigManager
	{
	public:
		void Initialize(const std::filesystem::path& configFilePath);

		const std::filesystem::path& GetRootFolder() { return mRootFolder; }
		const std::filesystem::path& GetAssetFolder() { return mAssetFolder; }
	private:
		std::filesystem::path mRootFolder;
		std::filesystem::path mAssetFolder;
	};
}