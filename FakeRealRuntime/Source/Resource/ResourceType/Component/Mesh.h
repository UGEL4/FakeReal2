#pragma once

#include <string>
#include <vector>
#include "Core/Math/Transform.h"
#include "Core/Mate/Reflection.h"

namespace FakeReal
{
	class CLASS(SubMeshResource, WhiteList)
	{
	public:
		PROPERTY()
		std::string mObjFile;
		PROPERTY()
		std::string mMaterial;
		Transform	mTransform;
	};

	class CLASS(MeshComponentResource)
	{
	public:
		std::vector<SubMeshResource> mSubMeshes;
	};
}