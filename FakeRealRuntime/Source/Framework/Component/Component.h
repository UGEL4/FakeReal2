#pragma once

#include <string>
#include "Core/Base/BaseDefine.h"
namespace FakeReal
{
	class GameObject;
	class Component
	{
	public:
		Component();
		virtual ~Component();

		virtual void Update(double deltaTime) = 0;
		virtual void PostLoadResource(WeakPtr<GameObject> parent) { m_pParent = parent; }

		void SetTypeName(const std::string& name) { mTypeName = name; }
		const std::string& GetTypeName() const { return mTypeName; }

		bool IsDirty() const { return mIsDirty; }
		void SetDirtyFlag(bool dirty) { mIsDirty = dirty; }
	protected:
		WeakPtr<GameObject> m_pParent;
		std::string mTypeName;
		bool mIsDirty{ false };
	};
}