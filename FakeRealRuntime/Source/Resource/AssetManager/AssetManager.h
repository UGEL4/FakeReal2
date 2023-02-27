#pragma once

#include <fstream>
#include <filesystem>
#include "Core/Mate/Serializer/Serializer.h"
#include "Core/Base/Macro.h"
#include "_generated/AllSerialize.h"
namespace FakeReal
{
	class AssetManager
	{
	public:
		template <typename AssetType>
		bool LoadAsset(const std::string& assetUrl, AssetType& outAsset) const
		{
			std::filesystem::path assetFilePath = GetFullPath(assetUrl);
			std::ifstream is(assetFilePath);
			if (!is.is_open())
			{
				LOG_ERROR("Open file {} failed!", assetFilePath.generic_string());
				return false;
			}
			std::stringstream ss;
			ss << is.rdbuf();
			std::string json_str(ss.str());

			is.close();

			JsonReader reader(json_str.c_str());
			bool result = Serializer::Read(reader, outAsset);
			if (!result)
			{
				LOG_ERROR("Parse json {} failed!", assetUrl);
				return false;
			}

			return true;
		}

		template <typename AssetType>
		bool SaveAsset(const std::string& assetUrl, const AssetType& asset) const
		{
			std::filesystem::path assetFilePath = GetFullPath(assetUrl);
			std::ofstream os(assetFilePath);
			if (!os.is_open())
			{
				LOG_ERROR("Open file {} failed!", assetFilePath.generic_string());
				return false;
			}

			JsonWriter writer;
			Serializer::Write(writer, asset);

			os << writer.GetString();
			os.close();
			LOG_ERROR("Save json {} successed!", assetUrl);
			return true;
		}

		std::filesystem::path GetFullPath(const std::string& relativePath) const;
	};
}