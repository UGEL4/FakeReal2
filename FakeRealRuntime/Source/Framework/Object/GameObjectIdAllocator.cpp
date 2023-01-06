#include "FRPch.h"
#include "Framework/Object/GameObjectIdAllocator.h"
#include "Core/Base/Macro.h"

namespace FakeReal
{
	std::atomic<GObjId> GameObjectIdAllocator::s_NextId { 0 };

	GObjId GameObjectIdAllocator::Alloc()
	{
		std::atomic<GObjId> id = s_NextId.load();
		s_NextId++;
		if (s_NextId >= invalid_obj_id)
		{
			LOG_FATAL("Game object id overflow!");
		}
		return id;
	}

}