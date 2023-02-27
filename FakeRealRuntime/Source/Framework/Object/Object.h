#pragma once
#include <memory>
#include <vector>
#include <string>
#include "Framework/Object/GameObjectIdAllocator.h"
#include "Framework/Component/Component.h"
#include "Core/Base/BaseDefine.h"
#include "Core/Mate/Reflection.h"

namespace FakeReal
{
	class ObjectInstanceResource;
	class GameObject : public std::enable_shared_from_this<GameObject>
	{
	public:
		GameObject(GObjId objId) : mId(objId) {}
		virtual ~GameObject();

		virtual void Update(double deltaTime);

		GObjId GetGameObjectId() const { return mId; }

		bool Load(const ObjectInstanceResource& instanceRes);
		bool Save(ObjectInstanceResource& instanceRes);

		bool HasComponent(const std::string& name) const;
		bool ShouldComponentTick(const std::string& name) const;

		template <typename T>
		T* TryGetComponent(const std::string& name)
		{
			for (auto& component : mComponents)
			{
				if (name == component->GetTypeName())
				{
					return static_cast<T*>(component.Get());
				}
			}
			return nullptr;
		}

		template <typename T>
		const T* TryGetComponentConst(const std::string& name) const
		{
			for (const auto& component : mComponents)
			{
				if (name == component->GetTypeName())
				{
					return static_cast<const T*>(component.Get());
				}
			}
			return nullptr;
		}
	protected:
		GObjId mId { invalid_obj_id };
		std::string mName {"Object"};
		std::string mDefinitionUrl;
		std::vector<ReflectionPtr<Component>> mComponents;
	};
}