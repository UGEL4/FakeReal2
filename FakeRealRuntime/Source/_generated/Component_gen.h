#pragma once
#include "Core/Mate/Serializer/Serializer.h"
#include "Framework/Component/Component.h"

namespace FakeReal
{
	template<>
	void Serializer::Write(JsonWriter& writer, const Component& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, Component& instance);
	
}
