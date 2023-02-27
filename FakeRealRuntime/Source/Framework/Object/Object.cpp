#include "FRPch.h"
#include "Framework/Object/Object.h"
#include "Framework/Component/Mesh/MeshComponent.h"
#include "Resource/ResourceType/Common/ObjectRes.h"

namespace FakeReal
{

	GameObject::~GameObject()
	{
		for (auto& comp : mComponents)
		{
			FR_REFLECTION_DELETE(comp);
		}
		mComponents.clear();
	}

	void GameObject::Update(double deltaTime)
	{
		for (auto& component : mComponents)
		{
			if (ShouldComponentTick(component->GetTypeName()))
			{
				component->Update(deltaTime);
			}
		}
	}

	bool GameObject::Load(const ObjectInstanceResource& instanceRes)
	{
		//test
		ObjectInstanceResource no_const_instanceRes = instanceRes;
		no_const_instanceRes.mName = "defauit_object";
		no_const_instanceRes.mInstanceComponents.clear();
		ReflectionPtr<MeshComponent> tmpComponent = FR_REFLECTION_NEW(MeshComponent);
		tmpComponent->SetTypeName("default_cube_mesh");
		no_const_instanceRes.mInstanceComponents.emplace_back(tmpComponent);
		//test end

		mComponents.clear();
		mName = no_const_instanceRes.mName;
		//object instanced component
		mComponents = no_const_instanceRes.mInstanceComponents;
		for (auto& component : mComponents)
		{
			if (component)
			{
				component->PostLoadResource(weak_from_this());
			}
		}
		//object definition component
		ObjectDefinitionResource definitionResource;
		for (auto component : definitionResource.mDefinitionComponents)
		{
			const std::string& name = component->GetTypeName();
			if (HasComponent(name))
			{
				continue;
			}
			component->PostLoadResource(weak_from_this());
			mComponents.emplace_back(component);
		}
		return true;
	}

	bool GameObject::Save(ObjectInstanceResource& instanceRes)
	{
		instanceRes.mName				= mName;
		instanceRes.mDefinitionUrl		= mDefinitionUrl;
		instanceRes.mInstanceComponents = mComponents;
		
		return true;
	}

	bool GameObject::HasComponent(const std::string& name) const
	{
		for (const auto& component : mComponents)
		{
			if (name == component->GetTypeName())
			{
				return true;
			}
		}
		return false;
	}

	bool GameObject::ShouldComponentTick(const std::string& name) const
	{
		return true;
	}

}