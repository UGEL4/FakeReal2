#pragma once
#include "Resource/ResourceType/Common/ObjectRes.h"
#include "Core/Mate/Reflection.h"
namespace FakeReal
{
	class CLASS(LevelResource)
	{
	public:
		std::string mName;
		std::vector<ObjectInstanceResource> mObjects;
	};
}