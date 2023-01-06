#pragma once
#include <string>
#include <vector>
namespace FakeReal
{
	class WorldResource
	{
	public:
		std::string mName;
		std::string mDefaultLevelUrl;
		std::vector<std::string> mLevelUrls;
	};
}