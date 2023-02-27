#include "FRPch.h"
#include "LevelRes_gen.h"
#include "ObjectRes_gen.h"
#include "Mesh_gen.h"
#include "MeshData_gen.h"
#include "Component_gen.h"
#include "MeshComponent_gen.h"

namespace FakeReal
{
	template<>
	void Serializer::Write(JsonWriter& writer, const LevelResource& instance);
	template<>
	void Serializer::Write(JsonWriter& writer, const LevelResource& instance)
	{
	    writer.StartObject();

		writer.SetKey("mName");
	    Serializer::Write(writer, instance.mName);
		writer.SetKey("mObjects");
		writer.StartArray();
		for (size_t i = 0; i < instance.mObjects.size(); i++)
		{
			Serializer::Write(writer, instance.mObjects[i]);
		}
		writer.EndArray();

	    writer.EndObject();
	}

	template<>
	bool Serializer::Read(JsonReader& reader, LevelResource& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, LevelResource& instance)
	{
		bool result = true;
		if (!reader.StartObject())
		{
			return false;
		}

		reader.Key("mName");
		Serializer::Read(reader, instance.mName);
		reader.Key("mObjects");
		size_t count = 0;
		if (!reader.StartArray(&count))
		{
			return false;
		}
		instance.mObjects.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			Serializer::Read(reader, instance.mObjects[i]);
		}
		if (!reader.EndArray())
		{
			return false;
		}

		result &= reader.EndObject();
		return result;
	}
	template<>
	void Serializer::Write(JsonWriter& writer, const ObjectInstanceResource& instance);
	template<>
	void Serializer::Write(JsonWriter& writer, const ObjectInstanceResource& instance)
	{
	    writer.StartObject();

		writer.SetKey("mName");
	    Serializer::Write(writer, instance.mName);
		writer.SetKey("mDefinitionUrl");
	    Serializer::Write(writer, instance.mDefinitionUrl);
		writer.SetKey("mInstanceComponents");
		writer.StartArray();
		for (size_t i = 0; i < instance.mInstanceComponents.size(); i++)
		{
			Serializer::Write(writer, instance.mInstanceComponents[i]);
		}
		writer.EndArray();

	    writer.EndObject();
	}

	template<>
	bool Serializer::Read(JsonReader& reader, ObjectInstanceResource& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, ObjectInstanceResource& instance)
	{
		bool result = true;
		if (!reader.StartObject())
		{
			return false;
		}

		reader.Key("mName");
		Serializer::Read(reader, instance.mName);
		reader.Key("mDefinitionUrl");
		Serializer::Read(reader, instance.mDefinitionUrl);
		reader.Key("mInstanceComponents");
		size_t count = 0;
		if (!reader.StartArray(&count))
		{
			return false;
		}
		instance.mInstanceComponents.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			Serializer::Read(reader, instance.mInstanceComponents[i]);
		}
		if (!reader.EndArray())
		{
			return false;
		}

		result &= reader.EndObject();
		return result;
	}
	template<>
	void Serializer::Write(JsonWriter& writer, const ObjectDefinitionResource& instance);
	template<>
	void Serializer::Write(JsonWriter& writer, const ObjectDefinitionResource& instance)
	{
	    writer.StartObject();

		writer.SetKey("mName");
	    Serializer::Write(writer, instance.mName);
		writer.SetKey("mDefinitionComponents");
		writer.StartArray();
		for (size_t i = 0; i < instance.mDefinitionComponents.size(); i++)
		{
			Serializer::Write(writer, instance.mDefinitionComponents[i]);
		}
		writer.EndArray();

	    writer.EndObject();
	}

	template<>
	bool Serializer::Read(JsonReader& reader, ObjectDefinitionResource& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, ObjectDefinitionResource& instance)
	{
		bool result = true;
		if (!reader.StartObject())
		{
			return false;
		}

		reader.Key("mName");
		Serializer::Read(reader, instance.mName);
		reader.Key("mDefinitionComponents");
		size_t count = 0;
		if (!reader.StartArray(&count))
		{
			return false;
		}
		instance.mDefinitionComponents.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			Serializer::Read(reader, instance.mDefinitionComponents[i]);
		}
		if (!reader.EndArray())
		{
			return false;
		}

		result &= reader.EndObject();
		return result;
	}
	template<>
	void Serializer::Write(JsonWriter& writer, const SubMeshResource& instance);
	template<>
	void Serializer::Write(JsonWriter& writer, const SubMeshResource& instance)
	{
	    writer.StartObject();

		writer.SetKey("mObjFile");
	    Serializer::Write(writer, instance.mObjFile);
		writer.SetKey("mMaterial");
	    Serializer::Write(writer, instance.mMaterial);

	    writer.EndObject();
	}

	template<>
	bool Serializer::Read(JsonReader& reader, SubMeshResource& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, SubMeshResource& instance)
	{
		bool result = true;
		if (!reader.StartObject())
		{
			return false;
		}

		reader.Key("mObjFile");
		Serializer::Read(reader, instance.mObjFile);
		reader.Key("mMaterial");
		Serializer::Read(reader, instance.mMaterial);

		result &= reader.EndObject();
		return result;
	}
	template<>
	void Serializer::Write(JsonWriter& writer, const MeshComponentResource& instance);
	template<>
	void Serializer::Write(JsonWriter& writer, const MeshComponentResource& instance)
	{
	    writer.StartObject();

		writer.SetKey("mSubMeshes");
		writer.StartArray();
		for (size_t i = 0; i < instance.mSubMeshes.size(); i++)
		{
			Serializer::Write(writer, instance.mSubMeshes[i]);
		}
		writer.EndArray();

	    writer.EndObject();
	}

	template<>
	bool Serializer::Read(JsonReader& reader, MeshComponentResource& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, MeshComponentResource& instance)
	{
		bool result = true;
		if (!reader.StartObject())
		{
			return false;
		}

		reader.Key("mSubMeshes");
		size_t count = 0;
		if (!reader.StartArray(&count))
		{
			return false;
		}
		instance.mSubMeshes.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			Serializer::Read(reader, instance.mSubMeshes[i]);
		}
		if (!reader.EndArray())
		{
			return false;
		}

		result &= reader.EndObject();
		return result;
	}
	template<>
	void Serializer::Write(JsonWriter& writer, const Vertex& instance);
	template<>
	void Serializer::Write(JsonWriter& writer, const Vertex& instance)
	{
	    writer.StartObject();

		writer.SetKey("x");
	    Serializer::Write(writer, instance.x);
		writer.SetKey("y");
	    Serializer::Write(writer, instance.y);
		writer.SetKey("z");
	    Serializer::Write(writer, instance.z);
		writer.SetKey("nx");
	    Serializer::Write(writer, instance.nx);
		writer.SetKey("ny");
	    Serializer::Write(writer, instance.ny);
		writer.SetKey("nz");
	    Serializer::Write(writer, instance.nz);
		writer.SetKey("tx");
	    Serializer::Write(writer, instance.tx);
		writer.SetKey("ty");
	    Serializer::Write(writer, instance.ty);

	    writer.EndObject();
	}

	template<>
	bool Serializer::Read(JsonReader& reader, Vertex& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, Vertex& instance)
	{
		bool result = true;
		if (!reader.StartObject())
		{
			return false;
		}

		reader.Key("x");
		Serializer::Read(reader, instance.x);
		reader.Key("y");
		Serializer::Read(reader, instance.y);
		reader.Key("z");
		Serializer::Read(reader, instance.z);
		reader.Key("nx");
		Serializer::Read(reader, instance.nx);
		reader.Key("ny");
		Serializer::Read(reader, instance.ny);
		reader.Key("nz");
		Serializer::Read(reader, instance.nz);
		reader.Key("tx");
		Serializer::Read(reader, instance.tx);
		reader.Key("ty");
		Serializer::Read(reader, instance.ty);

		result &= reader.EndObject();
		return result;
	}
	template<>
	void Serializer::Write(JsonWriter& writer, const MeshData& instance);
	template<>
	void Serializer::Write(JsonWriter& writer, const MeshData& instance)
	{
	    writer.StartObject();

		writer.SetKey("mVertices");
		writer.StartArray();
		for (size_t i = 0; i < instance.mVertices.size(); i++)
		{
			Serializer::Write(writer, instance.mVertices[i]);
		}
		writer.EndArray();
		writer.SetKey("mIndices");
		writer.StartArray();
		for (size_t i = 0; i < instance.mIndices.size(); i++)
		{
			Serializer::Write(writer, instance.mIndices[i]);
		}
		writer.EndArray();

	    writer.EndObject();
	}

	template<>
	bool Serializer::Read(JsonReader& reader, MeshData& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, MeshData& instance)
	{
		bool result = true;
		if (!reader.StartObject())
		{
			return false;
		}

		reader.Key("mVertices");
		size_t count = 0;
		if (!reader.StartArray(&count))
		{
			return false;
		}
		instance.mVertices.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			Serializer::Read(reader, instance.mVertices[i]);
		}
		if (!reader.EndArray())
		{
			return false;
		}
		reader.Key("mIndices");
		if (!reader.StartArray(&count))
		{
			return false;
		}
		instance.mIndices.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			Serializer::Read(reader, instance.mIndices[i]);
		}
		if (!reader.EndArray())
		{
			return false;
		}

		result &= reader.EndObject();
		return result;
	}
	template<>
	void Serializer::Write(JsonWriter& writer, const Component& instance);
	template<>
	void Serializer::Write(JsonWriter& writer, const Component& instance)
	{
	    writer.StartObject();

		writer.SetKey("mTypeName");
	    Serializer::Write(writer, instance.mTypeName);

	    writer.EndObject();
	}

	template<>
	bool Serializer::Read(JsonReader& reader, Component& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, Component& instance)
	{
		bool result = true;
		if (!reader.StartObject())
		{
			return false;
		}

		reader.Key("mTypeName");
		Serializer::Read(reader, instance.mTypeName);

		result &= reader.EndObject();
		return result;
	}
	template<>
	void Serializer::Write(JsonWriter& writer, const MeshComponent& instance);
	template<>
	void Serializer::Write(JsonWriter& writer, const MeshComponent& instance)
	{
	    writer.StartObject();

		//base class
		writer.SetKey("Component");
		Serializer::Write(writer, *((Component*)&(instance)));

		writer.SetKey("mMeshRes");
	    Serializer::Write(writer, instance.mMeshRes);

	    writer.EndObject();
	}

	template<>
	bool Serializer::Read(JsonReader& reader, MeshComponent& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, MeshComponent& instance)
	{
		bool result = true;
		if (!reader.StartObject())
		{
			return false;
		}

		//base class
		reader.Key("Component");
		Serializer::Read(reader, *((Component*)&(instance)));

		reader.Key("mMeshRes");
		Serializer::Read(reader, instance.mMeshRes);

		result &= reader.EndObject();
		return result;
	}
}
