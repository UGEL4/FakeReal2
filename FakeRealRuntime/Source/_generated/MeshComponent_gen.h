#pragma once
#include "Core/Mate/Serializer/Serializer.h"
#include "Framework/Component/Mesh/MeshComponent.h"

namespace FakeReal
{
	template<>
	void Serializer::Write(JsonWriter& writer, const MeshComponent& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, MeshComponent& instance);
	
}
