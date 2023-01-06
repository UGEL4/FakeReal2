#pragma once

#include <string>
#include <vector>
#include "Core/Math/Transform.h"

namespace FakeReal
{
	class SubMeshResource
	{
	public:
		std::string mObjFile;
		std::string mMaterial;
		Transform	mTransform;
	};

	class MeshComponentResource
	{
	public:
		std::vector<SubMeshResource> mSubMeshes;
	};
}