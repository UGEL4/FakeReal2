#pragma once
#include "Core/Mate/Serializer/Serializer.h"
#include "Resource/ResourceType/Common/ObjectRes.h"

namespace FakeReal
{
	template<>
	void Serializer::Write(JsonWriter& writer, const ObjectInstanceResource& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, ObjectInstanceResource& instance);
	
	template<>
	void Serializer::Write(JsonWriter& writer, const ObjectDefinitionResource& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, ObjectDefinitionResource& instance);
	
}
