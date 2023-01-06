#pragma once
#include "Core/Math/Matrix.h"

namespace FakeReal
{
	class RenderEntity
	{
	public:
		size_t mInstanceId{ 0 };
		Matrix4x4 mModelMatrix{ Matrix4x4(0.f) };

		//mesh
		size_t mMeshAssetId{ 0 };

		//material
		size_t mMaterialAssetId{ 0 };
	};
}