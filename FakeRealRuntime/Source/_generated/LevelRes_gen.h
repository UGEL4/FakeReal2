#pragma once
#include "Core/Mate/Serializer/Serializer.h"
#include "Resource/ResourceType/Common/LevelRes.h"

namespace FakeReal
{
	template<>
	void Serializer::Write(JsonWriter& writer, const LevelResource& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, LevelResource& instance);
	
}
