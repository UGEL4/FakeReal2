#include "FRPch.h"
#include "Framework/World/WorldManager.h"
#include "Framework/Level/Level.h"
#include "Core/Base/Macro.h"

namespace FakeReal
{
	WorldManager::WorldManager()
	{

	}

	WorldManager::~WorldManager()
	{
		Clear();
	}

	void WorldManager::Initialize()
	{
		mIsWorldLoaded = false;
		mCurWorldUrl = "default";
	}

	void WorldManager::Update(double deltaTime)
	{
		if (!mIsWorldLoaded)
		{
			//load
		}
		SharedPtr<Level> activeLevel = mActiveLevel.lock();
		if (activeLevel)
		{
			activeLevel->Update(deltaTime);
		}
	}

	void WorldManager::Clear()
	{
		//do something
		for (auto& itr : mLevelMap)
		{
			itr.second->Unload();
		}
		mLevelMap.clear();

		mCurWorldResource.reset();
		mIsWorldLoaded = false;
	}

	bool WorldManager::LoadWorld(const std::string& url)
	{
		LOG_INFO("Load world: {}", url);
		WorldResource res;

		mCurWorldResource = MakeShared<WorldResource>();

		if (!LoadLevel(res.mDefaultLevelUrl))
		{
			return false;
		}

		auto itr = mLevelMap.find(res.mDefaultLevelUrl);
		ASSERT(itr != mLevelMap.end());
		mActiveLevel = itr->second;

		mIsWorldLoaded = true;

		LOG_INFO("Load world successed: {}", url);
		return true;
	}

	bool WorldManager::LoadLevel(const std::string& url)
	{
		SharedPtr<Level> level = MakeShared<Level>();
		mActiveLevel = level;
		//load level
		if (!level->Load(url))
		{
			return false;
		}

		mLevelMap.emplace(url, level);
		return true;
	}

}