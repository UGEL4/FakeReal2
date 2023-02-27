#include "FRPch.h"
#include "Resource/ConfigManager/ConfigManager.h"
#include <fstream>

namespace FakeReal
{

	void ConfigManager::Initialize(const std::filesystem::path& configFilePath)
	{
		// read configs
		std::ifstream configFile(configFilePath);
		std::string   configLine;
		while (std::getline(configFile, configLine))
		{
			size_t seperate_pos = configLine.find_first_of('=');
			if (seperate_pos > 0 && seperate_pos < (configLine.length() - 1))
			{
				std::string name = configLine.substr(0, seperate_pos);
				std::string value = configLine.substr(seperate_pos + 1, configLine.length() - seperate_pos - 1);
				if (name == "RootFolder")
				{
					mRootFolder = configFilePath.parent_path() / value;
				}
				else if (name == "AssetFolder")
				{
					mAssetFolder = mRootFolder / value;
				}
				/*else if (name == "SchemaFolder")
				{
					m_schema_folder = m_root_folder / value;
				}
				else if (name == "DefaultWorld")
				{
					m_default_world_url = value;
				}
				else if (name == "BigIconFile")
				{
					m_editor_big_icon_path = m_root_folder / value;
				}
				else if (name == "SmallIconFile")
				{
					m_editor_small_icon_path = m_root_folder / value;
				}
				else if (name == "FontFile")
				{
					m_editor_font_path = m_root_folder / value;
				}
				else if (name == "GlobalRenderingRes")
				{
					m_global_rendering_res_url = value;
				}
				else if (name == "GlobalParticleRes")
				{
					m_global_particle_res_url = value;
				}*/
			}
		}
	}

}