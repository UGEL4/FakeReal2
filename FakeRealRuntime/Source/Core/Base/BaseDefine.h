#pragma once
#include <memory>

template <typename T>
using SharedPtr = std::shared_ptr<T>;
template <typename T, typename ... Args>
constexpr SharedPtr<T> MakeShared(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
using UniquePtr = std::unique_ptr<T>;
template <typename T, typename ... Args>
constexpr SharedPtr<T> MakeUnique(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T, typename U>
SharedPtr<T> StaticPointCast(const SharedPtr<U>& sp) noexcept
{
	return std::static_pointer_cast<T>(sp);
}