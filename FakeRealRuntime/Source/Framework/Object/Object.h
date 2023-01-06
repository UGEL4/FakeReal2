#pragma once
#include <memory>
#include <vector>
#include <string>
#include "Framework/Object/GameObjectIdAllocator.h"
#include "Framework/Component/Component.h"
#include "Core/Base/BaseDefine.h"

namespace FakeReal
{
	class ObjectInstanceResource;
	class GameObject : public std::enable_shared_from_this<GameObject>
	{
	public:
		GameObject(GObjId objId) : mId(objId) {}
		virtual ~GameObject();

		virtual void Update(double deltaTime);

		GObjId GetGameObjId() const { return mId; }

		bool Load(const ObjectInstanceResource& instanceRes);

		bool HasComponent(const std::string& name) const;
		bool ShouldComponentTick(const std::string& name) const;

		template <typename T>
		T* TryGetComponent(const std::string& name)
		{
			for (auto& component : mComponents)
			{
				if (name == component->GetTypeName())
				{
					return static_cast<T*>(component.get());
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
					return static_cast<const T*>(component.get());
				}
			}
			return nullptr;
		}
	protected:
		GObjId mId { invalid_obj_id };
		std::string mName;
		std::string mDefinitionUrl;
		std::vector<SharedPtr<Component>> mComponents;
	};
}