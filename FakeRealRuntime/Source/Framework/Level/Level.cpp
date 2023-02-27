#include "FRPch.h"
#include "Core/Base/Macro.h"
#include "Framework/Level/Level.h"
#include "Framework/Object/Object.h"
#include "Resource/ResourceType/Common/LevelRes.h"
#include "Resource/AssetManager/AssetManager.h"
#include "Function/Global/GlobalRuntimeContext.h"

namespace FakeReal
{
	Level::Level()
	{
		mGObjectMap.clear();
	}

	Level::~Level()
	{
		mGObjectMap.clear();
	}

	void Level::Update(double deltaTime)
	{
		if (!mIsLoaded)
		{
			return;
		}

		for (auto& itr : mGObjectMap)
		{
			ASSERT(itr.second);
			if (itr.second)
			{
				itr.second->Update(deltaTime);
			}
		}
	}

	bool Level::Load(const std::string& url)
	{
		LOG_INFO("Load level: {}", url);
		mLevelResourceUrl = url;

		LevelResource res;
		//test
		res.mObjects.clear();
		ObjectInstanceResource instanceRes{};
		res.mObjects.emplace_back(instanceRes);

		for (const ObjectInstanceResource& go_res : res.mObjects)
		{
			CreateObject(go_res);
		}

		mIsLoaded = true;
		LOG_INFO("Load level successed: {}", url);
		return true;
	}

	void Level::Unload()
	{
		Clear();
		LOG_INFO("Unload level: {}", mLevelResourceUrl);
	}

	bool Level::Save()
	{
		LOG_INFO("Save level: {}", mLevelResourceUrl);
		LevelResource level_res;
		size_t count = mGObjectMap.size();
		level_res.mObjects.resize(count);
		size_t index = 0;
		for (auto& pair : mGObjectMap)
		{
			if (pair.second)
			{
				pair.second->Save(level_res.mObjects[index++]);
			}
		}
		bool saveResult = g_global_runtime_context.m_pAssetManager->SaveAsset(mLevelResourceUrl, level_res);
		if (saveResult)
		{
			LOG_INFO("Save level Success.");
		}
		else
		{
			LOG_INFO("Save level Failed. {}", mLevelResourceUrl);
		}
		return saveResult;
	}

	GObjId Level::CreateObject(const ObjectInstanceResource& res)
	{
		GObjId id = GameObjectIdAllocator::Alloc();
		ASSERT(id != invalid_obj_id);

		SharedPtr<GameObject> object = MakeShared<GameObject>(id);
		if (object->Load(res))
		{
			mGObjectMap.emplace(id, object);
		}
		else
		{
			LOG_ERROR("Load object " + res.mName + " failed!");
			return invalid_obj_id;
		}

		return id;
	}

	WeakPtr<FakeReal::GameObject> Level::GetGameObjectById(GObjId id) const
	{
		auto itr = mGObjectMap.find(id);
		if (itr != mGObjectMap.end())
		{
			return itr->second;
		}
		return WeakPtr<GameObject>();
	}

	void Level::DeleteGameObjectById(GObjId id)
	{
		mGObjectMap.erase(id);
	}

	void Level::Clear()
	{
		mGObjectMap.clear();
		mIsLoaded = false;
	}

}