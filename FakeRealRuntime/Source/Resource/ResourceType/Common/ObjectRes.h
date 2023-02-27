#pragma once
#include <string>
#include <vector>
//#include "Core/Base/BaseDefine.h"
#include "Core/Mate/Reflection.h"
namespace FakeReal
{
	class Component;
	class CLASS(ObjectInstanceResource)
	{
	public:
		std::string mName;
		std::string mDefinitionUrl;
		std::vector<ReflectionPtr<Component>> mInstanceComponents;
	};

	class CLASS(ObjectDefinitionResource)
	{
	public:
		std::string mName;
		std::vector<ReflectionPtr<Component>> mDefinitionComponents;
	};
}