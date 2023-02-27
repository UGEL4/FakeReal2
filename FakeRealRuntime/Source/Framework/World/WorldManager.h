#pragma once
#include <unordered_map>
#include <string>
#include "Core/Base/BaseDefine.h"
#include "Resource/ResourceType/Common/WorldRes.h"
namespace FakeReal
{
	class Level;
	class WorldManager
	{
	public:
		WorldManager();
		~WorldManager();

		void Initialize();
		void Update(double deltaTime);

		void Clear();
		bool LoadWorld(const std::string& url);
		bool LoadLevel(const std::string& url);
	protected:
	private:
		bool mIsWorldLoaded { false };
		std::string mCurWorldUrl;
		SharedPtr<WorldResource> mCurWorldResource;
		std::unordered_map<std::string, SharedPtr<Level>> mLevelMap;
		WeakPtr<Level> mActiveLevel;
	};
}