#pragma once
#include "Resource/Common/Object.h"
namespace FakeReal
{
	class LevelResource
	{
	public:
		std::string mName;
		std::vector<ObjectInstanceResource> mObjects;
	};
}