#pragma once
#include <string>

namespace FakeReal
{
	struct GameObjectMeshDesc
	{
		std::string mMeshFile;
	};

	struct GameObjectPartDesc
	{
		GameObjectMeshDesc mMeshDesc;
	};
}