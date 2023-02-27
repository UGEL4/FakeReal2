#include "FRPch.h"
#include "Core/Mate/Serializer/Serializer.h"

namespace FakeReal
{
	template<>
	void Serializer::Write(JsonWriter& writer, const int32_t& instance)
	{
		writer.Int(instance);
	}

	template<>
	bool Serializer::Read(JsonReader& reader, int32_t& instance)
	{
		reader.Value(instance);
		return true;
	}

	template<>
	void Serializer::Write(JsonWriter& writer, const int64_t& instance)
	{
		writer.Int64(instance);
	}

	template<>
	bool Serializer::Read(JsonReader& reader, int64_t& instance)
	{
		reader.Value(instance);
		return true;
	}

	template<>
	void Serializer::Write(JsonWriter& writer, const uint32_t& instance)
	{
		writer.UInt(instance);
	}

	template<>
	bool Serializer::Read(JsonReader& reader, uint32_t& instance)
	{
		reader.Value(instance);
		return true;
	}

	template<>
	void Serializer::Write(JsonWriter& writer, const uint64_t& instance)
	{
		writer.UInt64(instance);
	}

	template<>
	bool Serializer::Read(JsonReader& reader, uint64_t& instance)
	{
		reader.Value(instance);
		return true;
	}

	template<>
	void Serializer::Write(JsonWriter& writer, const float& instance)
	{
		writer.Double(instance);
	}

	template<>
	bool Serializer::Read(JsonReader& reader, float& instance)
	{
		double tmp = 0.0;
		reader.Value(tmp);
		instance = static_cast<float>(tmp);
		return true;
	}

	template<>
	void Serializer::Write(JsonWriter& writer, const double& instance)
	{
		writer.Double(instance);
	}

	template<>
	bool Serializer::Read(JsonReader& reader, double& instance)
	{
		reader.Value(instance);
		return true;
	}

	template<>
	void Serializer::Write(JsonWriter& writer, const bool& instance)
	{
		writer.Bool(instance);
	}

	template<>
	bool Serializer::Read(JsonReader& reader, bool& instance)
	{
		reader.Value(instance);
		return true;
	}

	template<>
	void Serializer::Write(JsonWriter& writer, const std::string& instance)
	{
		writer.String(instance);
	}

	template<>
	bool Serializer::Read(JsonReader& reader, std::string& instance)
	{
		reader.Value(instance);
		return true;
	}
}