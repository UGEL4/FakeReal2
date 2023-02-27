#pragma once

#include "Framework/Component/Component.h"
#include "Resource/ResourceType/Component/Mesh.h"
#include "Function/Render/RenderObject.h"
#include "Core/Mate/Reflection.h"

namespace FakeReal
{
	class CLASS(MeshComponent, WhiteList) : BASE_CLASS(public Component)
	{
	public:
		MeshComponent();
		~MeshComponent();

		virtual void Update(double deltaTime) override final;
		virtual void PostLoadResource(WeakPtr<GameObject> parent) override;

	public:
		PROPERTY()
		MeshComponentResource mMeshRes;
	private:
		std::vector<GameObjectPartDesc> mRawMeshs;
	};
}
