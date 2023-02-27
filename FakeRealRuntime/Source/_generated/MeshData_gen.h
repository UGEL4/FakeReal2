#pragma once
#include "Core/Mate/Serializer/Serializer.h"
#include "Resource/ResourceType/Data/MeshData.h"

namespace FakeReal
{
	template<>
	void Serializer::Write(JsonWriter& writer, const Vertex& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, Vertex& instance);
	
	template<>
	void Serializer::Write(JsonWriter& writer, const MeshData& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, MeshData& instance);
	
}
