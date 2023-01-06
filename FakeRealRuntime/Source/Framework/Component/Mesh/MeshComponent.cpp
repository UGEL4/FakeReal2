#include "FRPch.h"
#include "Framework/Component/Mesh/MeshComponent.h"

namespace FakeReal
{
	MeshComponent::MeshComponent()
	{
		mRawMeshs.clear();
	}

	MeshComponent::~MeshComponent()
	{
		mRawMeshs.clear();
	}

	void MeshComponent::Update(double deltaTime)
	{
		if (!m_pParent.lock())
		{
			return;
		}
	}

}