#pragma once
#include "Core/Mate/Serializer/Serializer.h"
#include "Resource/ResourceType/Component/Mesh.h"

namespace FakeReal
{
	template<>
	void Serializer::Write(JsonWriter& writer, const SubMeshResource& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, SubMeshResource& instance);
	
	template<>
	void Serializer::Write(JsonWriter& writer, const MeshComponentResource& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, MeshComponentResource& instance);
	
}
