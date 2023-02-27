#pragma once
#include <string>
#include <vector>
#include "Framework/Object/GameObjectIdAllocator.h"

namespace FakeReal
{
	struct GameObjectMeshDesc
	{
		std::string mMeshFile;
	};

	struct GameObjectMaterialDesc
	{
		std::string mBaseColorTextureFile;
		bool mWithTexture { false };
	};

	struct GameObjectPartDesc
	{
		GameObjectMeshDesc mMeshDesc;
		GameObjectMaterialDesc mMaterialDesc;
	};

	struct GameObjectPartId
	{
		GObjId mGObjId { invalid_obj_id };
		size_t mPartId { invalid_obj_id };

		bool operator ==(const GameObjectPartId& other) const { return mGObjId == other.mGObjId && mPartId == other.mPartId; }
		size_t GetHashValue() const { return mGObjId ^ (mPartId << 1); }
		bool IsValid() const { return mGObjId != invalid_obj_id && mPartId != invalid_obj_id; }
	};

	class GameObjectDesc
	{
	public:
		GameObjectDesc(GObjId id) : mId(id) {}
		GameObjectDesc(GObjId id, const std::vector<GameObjectPartDesc>& gameObjectParts) : mId(id), mGameObjectParts(gameObjectParts) {}
		~GameObjectDesc() {}

		GObjId GetId() const { return mId; }
		const std::vector<GameObjectPartDesc>& GetGameObjectParts() const { return mGameObjectParts; }
	private:
		GObjId mId;
		std::vector<GameObjectPartDesc> mGameObjectParts;
	};
}

template<>
struct std::hash<FakeReal::GameObjectPartId>
{
	size_t operator()(const FakeReal::GameObjectPartId& other) const noexcept { return other.GetHashValue(); }
};