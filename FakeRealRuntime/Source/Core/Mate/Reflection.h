#pragma once

#define BASE_CLASS(className)	[[RefrectionClass]] className
#define CLASS(className, ...)	[[RefrectionClass, __VA_ARGS__]] className
#define PROPERTY(...)			[[Property, __VA_ARGS__]]

#define FR_REFLECTION_NEW(name, ...) FakeReal::ReflectionPtr(#name, new name(__VA_ARGS__));
#define FR_REFLECTION_DELETE(value) \
    if (value) \
    { \
        delete value.operator->(); \
        value.GetPtrRef() = nullptr; \
    }

#include <string>

namespace FakeReal
{
	template <class T>
	class ReflectionPtr
	{
		template <class U>
		friend class ReflectionPtr;
	public:
		ReflectionPtr() : mTypeName(""), m_pInstance(nullptr) {}
		ReflectionPtr(const std::string& typeName, T* pInstance) : mTypeName(typeName), m_pInstance(pInstance) {}
		~ReflectionPtr() {}
		ReflectionPtr(const ReflectionPtr& other) : mTypeName(other.mTypeName), m_pInstance(other.m_pInstance) {}
		ReflectionPtr<T>& operator =(const ReflectionPtr<T>& other)
		{
			if (this == &other)
			{
				return *this;
			}
			mTypeName = other.mTypeName;
			m_pInstance = other.m_pInstance;
			return *this;
		}

		ReflectionPtr<T>& operator =(ReflectionPtr<T>&& other)
		{
			if (this == &other)
			{
				return *this;
			}
			mTypeName = other.mTypeName;
			m_pInstance = other.m_pInstance;
			return *this;
		}

		template <typename U>
		ReflectionPtr<T>& operator =(const ReflectionPtr<U>& other)
		{
			if (this == static_cast<void*>(&other))
			{
				return *this;
			}
			mTypeName	= other.mTypeName;
			m_pInstance = static_cast<T*>(other.m_pInstance);
			return *this;
		}

		template <typename U>
		ReflectionPtr<T>& operator =(ReflectionPtr<U>&& other)
		{
			if (this == static_cast<void*>(&other))
			{
				return *this;
			}
			mTypeName = other.mTypeName;
			m_pInstance = static_cast<T*>(other.m_pInstance);
			return *this;
		}

		template <typename U>
		explicit operator U*()
		{
			return static_cast<U*>(m_pInstance);
		}

		template <typename U>
		explicit operator const U*() const
		{
			return static_cast<U*>(m_pInstance);
		}

		template <typename U>
		operator ReflectionPtr<U>()
		{
			return ReflectionPtr<U>(mTypeName, static_cast<U*>(m_pInstance));
		}

		template <typename U>
		operator const ReflectionPtr<U>() const
		{
			return ReflectionPtr<U>(mTypeName, static_cast<U*>(m_pInstance));
		}

		std::string GetTypeName() const { return mTypeName; }
		void SetTypeName(const std::string& typeName) { mTypeName = typeName; }

		bool operator ==(const T* ptr) const
		{
			return m_pInstance == ptr;
		}

		bool operator !=(const T* ptr) const
		{
			return m_pInstance != ptr;
		}

		bool operator ==(const ReflectionPtr<T>& other) const
		{
			return m_pInstance == other.m_pInstance;
		}

		bool operator !=(const ReflectionPtr<T>& other) const
		{
			return m_pInstance != other.m_pInstance;
		}

		T* operator ->()
		{
			return m_pInstance;
		}

		T* operator ->() const
		{
			return m_pInstance;
		}

		T* Get()
		{
			return m_pInstance;
		}

		T* Get() const
		{
			return m_pInstance;
		}

		T*& GetPtrRef()
		{
			return m_pInstance;
		}

		T& operator *()
		{
			return *m_pInstance;
		}

		const T& operator *() const
		{
			return *(static_cast<const T*>(m_pInstance));
		}

		operator bool() const
		{
			return m_pInstance != nullptr;
		}

	private:
		std::string mTypeName{ "" };
		T* m_pInstance{ nullptr };
	};
}