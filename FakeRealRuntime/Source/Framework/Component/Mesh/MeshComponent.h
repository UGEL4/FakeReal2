#pragma once

#include "Framework/Component/Component.h"
#include "Resource/Component/Mesh.h"
#include "Function/Render/RenderObject.h"

namespace FakeReal
{
	class MeshComponent : public Component
	{
	public:
		MeshComponent();
		~MeshComponent();

		virtual void Update(double deltaTime) override final;

	private:
		MeshComponentResource mMeshRes;
		std::vector<GameObjectPartDesc> mRawMeshs;
	};
}
