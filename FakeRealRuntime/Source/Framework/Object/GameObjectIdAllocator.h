#pragma once

#include <atomic>
#include <limits>

namespace FakeReal
{
	using GObjId = size_t;
	constexpr GObjId invalid_obj_id = std::numeric_limits<GObjId>::max();

	class GameObjectIdAllocator
	{
	public:
		static GObjId Alloc();
	private:
		GameObjectIdAllocator() {}
	private:
		static std::atomic<GObjId> s_NextId;
	};
}