#pragma once
#include <string>
#include <vector>
#include "Core/Base/BaseDefine.h"
namespace FakeReal
{
	class Component;
	class ObjectInstanceResource
	{
	public:
		std::string mName;
		std::vector<SharedPtr<Component>> mInstanceComponents;
	};

	class ObjectDefinitionResource
	{
	public:
		std::string mName;
		std::vector<SharedPtr<Component>> mDefinitionComponents;
	};
}