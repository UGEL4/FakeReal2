#include "FRPch.h"
#include "Framework/Object/Object.h"
#include "Resource/Common/Object.h"

namespace FakeReal
{

	GameObject::~GameObject()
	{
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
		mComponents.clear();
		mName = instanceRes.mName;
		//object instanced component
		mComponents = instanceRes.mInstanceComponents;
		for (auto& component : mComponents)
		{
			if (component)
			{
				component->PostLoadResource(weak_from_this());
			}
		}
		//object definition component

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