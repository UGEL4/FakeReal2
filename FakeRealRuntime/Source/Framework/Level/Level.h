#pragma once
#include <unordered_map>
#include <string>
#include "Core/Base/BaseDefine.h"
#include "Framework/Object/GameObjectIdAllocator.h"
namespace FakeReal
{
	class GameObject;
	class ObjectInstanceResource;
	class Level
	{
	public:
		Level();
		virtual ~Level();

		void Update(double deltaTime);

		bool Load(const std::string& url);
		void Unload();

		bool Save();

		GObjId CreateObject(const ObjectInstanceResource& res);

		WeakPtr<GameObject> GetGameObjectById(GObjId id) const;
		void DeleteGameObjectById(GObjId id);
	private:
		void Clear();
	protected:
		bool mIsLoaded { false };
		std::string mLevelResourceUrl;
		std::unordered_map<GObjId, SharedPtr<GameObject>> mGObjectMap;
	};
}