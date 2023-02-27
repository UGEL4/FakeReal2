#pragma once

#include <string>
#include "Core/Base/BaseDefine.h"
#include "Core/Mate/Reflection.h"
namespace FakeReal
{
	class GameObject;
	class CLASS(Component, WhiteList)
	{
	public:
		Component();
		virtual ~Component();

		virtual void Update(double deltaTime);
		virtual void PostLoadResource(WeakPtr<GameObject> parent) { m_pParent = parent; }

		void SetTypeName(const std::string& name) { mTypeName = name; }
		const std::string& GetTypeName() const { return mTypeName; }

		bool IsDirty() const { return mIsDirty; }
		void SetDirtyFlag(bool dirty) { mIsDirty = dirty; }

		bool Save();

		PROPERTY()
		std::string mTypeName;
	protected:
		WeakPtr<GameObject> m_pParent;
		bool mIsDirty{ false };
	};
}