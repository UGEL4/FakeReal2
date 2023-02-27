#pragma once
#include <cstdint>
namespace FakeReal
{
	//
	static inline uint32_t RundUp(uint32_t value, uint32_t alignment)
	{
		uint32_t tmp = value + alignment - 1;
		return (tmp - tmp % alignment);
	}
}