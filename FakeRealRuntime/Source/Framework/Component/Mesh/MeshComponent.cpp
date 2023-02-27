#include "FRPch.h"
#include "Framework/Component/Mesh/MeshComponent.h"
#include "Framework/Object/Object.h"
#include "Function/Render/RenderSystem.h"
#include "Function/Global/GlobalRuntimeContext.h"

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

		std::vector<GameObjectPartDesc> dirtyMeshParts = mRawMeshs;

		RenderSwapContext& context		= g_global_runtime_context.m_pRenderSystem->GetRenderSwapContext();
		RenderSwapData& logicSwaoData	= context.GetLogicSwapData();
		logicSwaoData.AddDirtyGameObject(GameObjectDesc{ m_pParent.lock()->GetGameObjectId(), dirtyMeshParts });
	}

	void MeshComponent::PostLoadResource(WeakPtr<GameObject> parent)
	{
		Component::PostLoadResource(parent);

		//test
		mRawMeshs.resize(1);
		GameObjectPartDesc& desc = mRawMeshs[0];
		desc.mMeshDesc = { mTypeName };
	}

}